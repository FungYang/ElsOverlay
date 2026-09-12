#include "transcendencevisionworker.h"
#include "transcendencevisionconfig.h"

#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QVector>
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

void TranscendenceVisionWorker::processFrame(QImage area)
{
    if (area.isNull() || m_templateIcon.isNull())
        return;

    QRect foundRect;
    double score = 0.0;

    const bool found = findIcon(area, foundRect, score);

    emit scanResult(found, foundRect, score, area);
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
    const int width =
        m_templateIcon.width();

    const int height =
        m_templateIcon.height();

    const int innerWidth =
        width - 2;

    const int innerHeight =
        height - 2;

    const int total =
        innerWidth * innerHeight;

    if (total <= 0)
        return 0.0;

    constexpr int sampleCount = 16;

    const int maxDifferentSamples =
        static_cast<int>(
            (TranscendenceVisionConfig::FAST_TOLERANCE / 100.0) *
            sampleCount
            );

    int sampleX[sampleCount];
    int sampleY[sampleCount];

    for (int gy = 0; gy < 4; ++gy)
    {
        for (int gx = 0; gx < 4; ++gx)
        {
            const int idx = gy * 4 + gx;

            sampleX[idx] =
                1 + (gx * (innerWidth - 1)) / 3;

            sampleY[idx] =
                1 + (gy * (innerHeight - 1)) / 3;
        }
    }

    int differentSamples = 0;

    for (int i = 0;
         i < sampleCount;
         ++i)
    {
        const QRgb *sourceLine =
            reinterpret_cast<const QRgb *>(
                area.constScanLine(
                    offsetY + sampleY[i]
                    )
                );

        const QRgb *templateLine =
            reinterpret_cast<const QRgb *>(
                m_templateIcon.constScanLine(
                    sampleY[i]
                    )
                );

        const QRgb sourcePixel =
            sourceLine[offsetX + sampleX[i]];

        const QRgb templatePixel =
            templateLine[sampleX[i]];

        const int dr =
            qAbs(qRed(sourcePixel) - qRed(templatePixel));

        const int dg =
            qAbs(qGreen(sourcePixel) - qGreen(templatePixel));

        const int db =
            qAbs(qBlue(sourcePixel) - qBlue(templatePixel));

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
        1.0 - (TranscendenceVisionConfig::MATCH_THRESHOLD / 100.0);

    const int maxDifferentPixels =
        static_cast<int>(maxDifferentRatio * total);

    int differentPixels = 0;

    for (int y = 1;
         y <= height - 2;
         ++y)
    {
        const QRgb *sourceLine =
            reinterpret_cast<const QRgb *>(
                area.constScanLine(offsetY + y)
                );

        const QRgb *templateLine =
            reinterpret_cast<const QRgb *>(
                m_templateIcon.constScanLine(y)
                );

        for (int x = 1;
             x <= width - 2;
             ++x)
        {
            const QRgb sourcePixel =
                sourceLine[offsetX + x];

            const QRgb templatePixel =
                templateLine[x];

            const int dr =
                qAbs(qRed(sourcePixel) - qRed(templatePixel));

            const int dg =
                qAbs(qGreen(sourcePixel) - qGreen(templatePixel));

            const int db =
                qAbs(qBlue(sourcePixel) - qBlue(templatePixel));

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