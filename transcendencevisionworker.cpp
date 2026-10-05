#include "transcendencevisionworker.h"

#include "transcendencevisionconfig.h"

#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QVector>
#include <QMutexLocker>
#include <atomic>

TranscendenceVisionWorker::TranscendenceVisionWorker(QObject *parent)
    : QObject(parent)
{
}

void TranscendenceVisionWorker::setTemplate(QImage templateIcon, int iconWidth, int iconHeight)
{
    Q_UNUSED(iconWidth);
    Q_UNUSED(iconHeight);
    m_templateIcon = templateIcon;
}

void TranscendenceVisionWorker::submitFrame(QImage area)
{
    if (area.isNull())
        return;

    bool startProcessing = false;

    {
        QMutexLocker locker(&m_frameMutex);

        // Se il worker sta già elaborando un frame,
        // conserviamo soltanto l'ultimo arrivato.
        if (m_processing)
        {
            m_pendingFrame = std::move(area);
            return;
        }

        m_processing = true;
        startProcessing = true;
    }

    if (startProcessing)
    {
        QMetaObject::invokeMethod(
            this,
            "processFrame",
            Qt::QueuedConnection,
            Q_ARG(QImage, area)
            );
    }
}


void TranscendenceVisionWorker::processFrame(QImage area)
{
    while (!area.isNull())
    {
        if (!m_templateIcon.isNull())
        {
            QRect foundRect;
            double score = 0.0;

            const bool found =
                findIcon(area, foundRect, score);

            emit scanResult(
                found,
                foundRect,
                score,
                area
                );
        }

        QImage nextFrame;

        {
            QMutexLocker locker(&m_frameMutex);

            if (!m_pendingFrame.isNull())
            {
                // Prendiamo solamente l'ultimo frame disponibile.
                nextFrame = std::move(m_pendingFrame);
                m_pendingFrame = QImage();
            }
            else
            {
                // Nessun altro frame: il worker torna disponibile.
                m_processing = false;
                return;
            }
        }

        area = std::move(nextFrame);
    }

    // Caso difensivo: area nulla.
    QMutexLocker locker(&m_frameMutex);
    m_processing = false;
}

bool TranscendenceVisionWorker::findIcon(
    const QImage &area,
    QRect &foundRect,
    double &score
    ) const
{
    score = 0.0;

    if (area.isNull() || m_templateIcon.isNull())
        return false;

    const int width =
        m_templateIcon.width();

    const int height =
        m_templateIcon.height();

    if (area.width() < width ||
        area.height() < height)
    {
        return false;
    }

    const int maxY =
        area.height() - height;

    const int maxX =
        area.width() - width;

    struct ChunkResult
    {
        double score = 0.0;
        QRect rect;
        bool found = false;
    };

    std::atomic<bool> stopFlag{false};

    const int threadCount =
        qMax(1, QThread::idealThreadCount()/2);

    const int totalRows =
        maxY + 1;

    constexpr int CHUNKS_PER_THREAD = 1;

    const int desiredChunks =
        qMax(1, threadCount * CHUNKS_PER_THREAD);

    const int rowsPerChunk =
        qMax(
            1,
            (totalRows + desiredChunks - 1) / desiredChunks
            );

    QVector<QFuture<ChunkResult>> futures;

    for (int yStart = 0;
         yStart <= maxY;
         yStart += rowsPerChunk)
    {
        const int yEnd =
            qMin(
                yStart + rowsPerChunk - 1,
                maxY
                );

        futures.append(
            QtConcurrent::run(
                [this, &area, &stopFlag, yStart, yEnd, maxX, width, height]()
                -> ChunkResult
                {
                    ChunkResult result;

                    for (int y = yStart;
                         y <= yEnd;
                         ++y)
                    {
                        if (stopFlag.load(std::memory_order_relaxed))
                            break;

                        for (int x = 0;
                             x <= maxX;
                             ++x)
                        {
                            const double current =
                                compareAt(
                                    area,
                                    x,
                                    y
                                    );

                            if (current > result.score)
                            {
                                result.score = current;

                                result.rect =
                                    QRect(
                                        x,
                                        y,
                                        width,
                                        height
                                        );

                                if (current >=
                                    TranscendenceVisionConfig::MATCH_THRESHOLD)
                                {
                                    result.found = true;

                                    stopFlag.store(
                                        true,
                                        std::memory_order_relaxed
                                        );

                                    return result;
                                }
                            }
                        }
                    }

                    return result;
                }
                )
            );
    }

    for (QFuture<ChunkResult> &future : futures)
        future.waitForFinished();

    for (QFuture<ChunkResult> &future : futures)
    {
        const ChunkResult result =
            future.result();

        if (result.score > score)
        {
            score = result.score;
            foundRect = result.rect;
        }

        if (result.found)
            return true;
    }

    return score >=
           TranscendenceVisionConfig::MATCH_THRESHOLD;
}

double TranscendenceVisionWorker::compareAt(
    const QImage &area,
    int offsetX,
    int offsetY
    ) const
{
    const int width = m_templateIcon.width();
    const int height = m_templateIcon.height();

    const int innerWidth = width - 2;
    const int innerHeight = height - 2;

    const int total = innerWidth * innerHeight;

    if (total <= 0)
        return 0.0;

    constexpr int sampleCount = 16;

    const int maxDifferentSamples =
        static_cast<int>(
            (TranscendenceVisionConfig::FAST_TOLERANCE / 100.0) *
            sampleCount
            );

    /*
     * Fast rejection:
     * controlliamo 16 pixel distribuiti sull'area prima
     * di eseguire il confronto completo.
     */
    int sampleX[sampleCount];
    int sampleY[sampleCount];

    for (int gy = 0; gy < 4; ++gy)
    {
        for (int gx = 0; gx < 4; ++gx)
        {
            const int index = gy * 4 + gx;

            sampleX[index] =
                1 + (gx * (innerWidth - 1)) / 3;

            sampleY[index] =
                1 + (gy * (innerHeight - 1)) / 3;
        }
    }

    int differentSamples = 0;

    for (int i = 0; i < sampleCount; ++i)
    {
        const QRgb *sourceLine =
            reinterpret_cast<const QRgb *>(
                area.constScanLine(offsetY + sampleY[i])
                );

        const QRgb *templateLine =
            reinterpret_cast<const QRgb *>(
                m_templateIcon.constScanLine(sampleY[i])
                );

        const QRgb sourcePixel =
            sourceLine[offsetX + sampleX[i]];

        const QRgb templatePixel =
            templateLine[sampleX[i]];

        int dr =
            static_cast<int>((sourcePixel >> 16) & 0xFF) -
            static_cast<int>((templatePixel >> 16) & 0xFF);

        int dg =
            static_cast<int>((sourcePixel >> 8) & 0xFF) -
            static_cast<int>((templatePixel >> 8) & 0xFF);

        int db =
            static_cast<int>(sourcePixel & 0xFF) -
            static_cast<int>(templatePixel & 0xFF);

        if (dr < 0)
            dr = -dr;

        if (dg < 0)
            dg = -dg;

        if (db < 0)
            db = -db;

        if (dr > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            dg > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            db > TranscendenceVisionConfig::PIXEL_TOLERANCE)
        {
            ++differentSamples;

            if (differentSamples > maxDifferentSamples)
                return 0.0;
        }
    }

    const double maxDifferentRatio =
        1.0 -
        (TranscendenceVisionConfig::MATCH_THRESHOLD / 100.0);

    const int maxDifferentPixels =
        static_cast<int>(maxDifferentRatio * total);

    int differentPixels = 0;

    for (int y = 1; y <= height - 2; ++y)
    {
        const QRgb *sourceLine =
            reinterpret_cast<const QRgb *>(
                area.constScanLine(offsetY + y)
                );

        const QRgb *templateLine =
            reinterpret_cast<const QRgb *>(
                m_templateIcon.constScanLine(y)
                );

        const QRgb *sourcePixel =
            sourceLine + offsetX + 1;

        const QRgb *templatePixel =
            templateLine + 1;

        for (int x = 1; x <= width - 2; ++x)
        {
            const QRgb source = *sourcePixel++;
            const QRgb templ = *templatePixel++;

            int dr =
                static_cast<int>((source >> 16) & 0xFF) -
                static_cast<int>((templ >> 16) & 0xFF);

            int dg =
                static_cast<int>((source >> 8) & 0xFF) -
                static_cast<int>((templ >> 8) & 0xFF);

            int db =
                static_cast<int>(source & 0xFF) -
                static_cast<int>(templ & 0xFF);

            if (dr < 0)
                dr = -dr;

            if (dg < 0)
                dg = -dg;

            if (db < 0)
                db = -db;

            if (dr > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                dg > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                db > TranscendenceVisionConfig::PIXEL_TOLERANCE)
            {
                ++differentPixels;

                if (differentPixels > maxDifferentPixels)
                {
                    const double ratio =
                        static_cast<double>(differentPixels) /
                        static_cast<double>(total);

                    return (1.0 - ratio) * 100.0;
                }
            }
        }
    }

    const double differentRatio =
        static_cast<double>(differentPixels) /
        static_cast<double>(total);

    return (1.0 - differentRatio) * 100.0;
}