#include "transcendencevisionworker.h"
#include "transcendencevisionconfig.h"

#include <QMutexLocker>
#include <QSemaphore>
#include <QThreadPool>

#include <atomic>
#include <memory>


namespace
{

struct ChunkResult
{
    double score = 0.0;
    QRect rect;
    bool found = false;
};

struct ScanContext
{
    std::atomic<bool> stopFlag{false};
    QSemaphore finished{0};
    QVector<ChunkResult> results;
};

} // namespace


TranscendenceVisionWorker::TranscendenceVisionWorker(QObject *parent)
    : QObject(parent)
{
    // Il matcher usa un pool dedicato.
    // Tre thread sono sufficienti per questo tipo di ricerca
    // e impediscono al matcher di saturare la CPU.
    m_matchPool.setMaxThreadCount(3);
}


TranscendenceVisionWorker::~TranscendenceVisionWorker()
{
    // Aspettiamo che eventuali task del matcher siano terminati
    // prima di distruggere il worker.
    m_matchPool.waitForDone();
}


void TranscendenceVisionWorker::setTemplate(
    QImage templateIcon,
    int iconWidth,
    int iconHeight
    )
{
    Q_UNUSED(iconWidth);
    Q_UNUSED(iconHeight);

    m_templateIcon = std::move(templateIcon);
}


void TranscendenceVisionWorker::submitFrame(
    quint64 frameId,
    QImage area
    )
{
    if (area.isNull())
        return;

    {
        QMutexLocker locker(&m_frameMutex);

        if (m_processing)
        {
            m_pendingFrame.frameId = frameId;
            m_pendingFrame.image = std::move(area);
            return;
        }

        m_processing = true;
    }

    QMetaObject::invokeMethod(
        this,
        "processFrame",
        Qt::QueuedConnection,
        Q_ARG(quint64, frameId),
        Q_ARG(QImage, area)
        );
}


void TranscendenceVisionWorker::processFrame(
    quint64 frameId,
    QImage area
    )
{
    while (!area.isNull())
    {
        if (!m_templateIcon.isNull())
        {
            QRect foundRect;
            double score = 0.0;

            const bool found =
                findIcon(
                    area,
                    foundRect,
                    score
                    );

            emit scanResult(
                frameId,
                found,
                foundRect,
                score,
                area
                );
        }

        PendingFrame nextFrame;

        {
            QMutexLocker locker(&m_frameMutex);

            if (!m_pendingFrame.image.isNull())
            {
                nextFrame = std::move(m_pendingFrame);
                m_pendingFrame = PendingFrame();
            }
            else
            {
                m_processing = false;
                return;
            }
        }

        frameId = nextFrame.frameId;
        area = std::move(nextFrame.image);
    }

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
    foundRect = QRect();

    if (area.isNull() ||
        m_templateIcon.isNull())
    {
        return false;
    }

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

    /*
     * Usiamo tre chunk.
     *
     * Il QThreadPool è persistente e contiene già
     * tre thread disponibili.
     */
    constexpr int threadCount = 3;

    const int totalRows =
        maxY + 1;

    const int chunkCount =
        qMin(
            threadCount,
            totalRows
            );

    if (chunkCount <= 0)
        return false;

    const int rowsPerChunk =
        qMax(
            1,
            (totalRows + chunkCount - 1) /
                chunkCount
            );

    auto context =
        std::make_shared<ScanContext>();

    context->results.resize(chunkCount);

    for (int chunkIndex = 0;
         chunkIndex < chunkCount;
         ++chunkIndex)
    {
        const int yStart =
            chunkIndex * rowsPerChunk;

        if (yStart > maxY)
            break;

        const int yEnd =
            qMin(
                yStart + rowsPerChunk - 1,
                maxY
                );

        /*
         * QThreadPool mantiene i thread vivi.
         *
         * La lambda viene eseguita in uno dei tre thread
         * del pool e può accedere a compareAt() perché
         * viene creata all'interno del metodo della classe.
         */
        m_matchPool.start(
            [this,
             &area,
             context,
             chunkIndex,
             yStart,
             yEnd,
             maxX,
             width,
             height]()
            {
                ChunkResult result;

                for (int y = yStart;
                     y <= yEnd;
                     ++y)
                {
                    if (context->stopFlag.load(
                            std::memory_order_relaxed))
                    {
                        break;
                    }

                    for (int x = 0;
                         x <= maxX;
                         ++x)
                    {
                        if (context->stopFlag.load(
                                std::memory_order_relaxed))
                        {
                            break;
                        }

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

                            /*
                             * Abbiamo trovato un match
                             * sufficientemente buono.
                             *
                             * Gli altri thread vedranno
                             * stopFlag e termineranno
                             * appena possibile.
                             */
                            if (current >=
                                TranscendenceVisionConfig::MATCH_THRESHOLD)
                            {
                                result.found = true;

                                context->stopFlag.store(
                                    true,
                                    std::memory_order_relaxed
                                    );

                                break;
                            }
                        }
                    }
                }

                /*
                 * Ogni thread scrive esclusivamente
                 * nel proprio elemento del QVector.
                 */
                context->results[chunkIndex] =
                    result;

                context->finished.release();
            }
            );
    }

    /*
     * Aspettiamo che tutti i chunk abbiano terminato.
     *
     * Questo è importante perché 'area' è un riferimento
     * al parametro di findIcon() e non deve più essere
     * utilizzato dai thread quando usciamo dalla funzione.
     */
    for (int i = 0;
         i < chunkCount;
         ++i)
    {
        context->finished.acquire();
    }

    /*
     * Se un thread ha trovato un match, restituiamo
     * immediatamente il risultato.
     *
     * Altrimenti conserviamo il miglior punteggio
     * trovato tra tutti i chunk.
     */
    for (const ChunkResult &result :
         context->results)
    {
        if (result.score > score)
        {
            score = result.score;
            foundRect = result.rect;
        }

        if (result.found)
        {
            foundRect = result.rect;
            score = result.score;

            return true;
        }
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

    /*
     * Fast rejection:
     *
     * controlliamo 16 pixel distribuiti
     * sull'area prima di eseguire il confronto
     * completo.
     */
    int sampleX[sampleCount];
    int sampleY[sampleCount];

    for (int gy = 0; gy < 4; ++gy)
    {
        for (int gx = 0; gx < 4; ++gx)
        {
            const int index =
                gy * 4 + gx;

            sampleX[index] =
                1 +
                (gx * (innerWidth - 1)) / 3;

            sampleY[index] =
                1 +
                (gy * (innerHeight - 1)) / 3;
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
            sourceLine[
                offsetX + sampleX[i]
        ];

        const QRgb templatePixel =
            templateLine[
                sampleX[i]
        ];

        int dr =
            static_cast<int>(
                (sourcePixel >> 16) & 0xFF
                ) -
            static_cast<int>(
                (templatePixel >> 16) & 0xFF
                );

        int dg =
            static_cast<int>(
                (sourcePixel >> 8) & 0xFF
                ) -
            static_cast<int>(
                (templatePixel >> 8) & 0xFF
                );

        int db =
            static_cast<int>(
                sourcePixel & 0xFF
                ) -
            static_cast<int>(
                templatePixel & 0xFF
                );

        if (dr < 0)
            dr = -dr;

        if (dg < 0)
            dg = -dg;

        if (db < 0)
            db = -db;

        if (dr >
                TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            dg >
                TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            db >
                TranscendenceVisionConfig::PIXEL_TOLERANCE)
        {
            ++differentSamples;

            if (differentSamples >
                maxDifferentSamples)
            {
                return 0.0;
            }
        }
    }

    const double maxDifferentRatio =
        1.0 -
        (TranscendenceVisionConfig::MATCH_THRESHOLD / 100.0);

    const int maxDifferentPixels =
        static_cast<int>(
            maxDifferentRatio * total
            );

    int differentPixels = 0;

    for (int y = 1;
         y <= height - 2;
         ++y)
    {
        const QRgb *sourceLine =
            reinterpret_cast<const QRgb *>(
                area.constScanLine(
                    offsetY + y
                    )
                );

        const QRgb *templateLine =
            reinterpret_cast<const QRgb *>(
                m_templateIcon.constScanLine(y)
                );

        const QRgb *sourcePixel =
            sourceLine + offsetX + 1;

        const QRgb *templatePixel =
            templateLine + 1;

        for (int x = 1;
             x <= width - 2;
             ++x)
        {
            const QRgb source =
                *sourcePixel++;

            const QRgb templ =
                *templatePixel++;

            int dr =
                static_cast<int>(
                    (source >> 16) & 0xFF
                    ) -
                static_cast<int>(
                    (templ >> 16) & 0xFF
                    );

            int dg =
                static_cast<int>(
                    (source >> 8) & 0xFF
                    ) -
                static_cast<int>(
                    (templ >> 8) & 0xFF
                    );

            int db =
                static_cast<int>(
                    source & 0xFF
                    ) -
                static_cast<int>(
                    templ & 0xFF
                    );

            if (dr < 0)
                dr = -dr;

            if (dg < 0)
                dg = -dg;

            if (db < 0)
                db = -db;

            if (dr >
                    TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                dg >
                    TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                db >
                    TranscendenceVisionConfig::PIXEL_TOLERANCE)
            {
                ++differentPixels;

                /*
                 * Early exit:
                 * non possiamo più raggiungere
                 * MATCH_THRESHOLD.
                 */
                if (differentPixels >
                    maxDifferentPixels)
                {
                    const double ratio =
                        static_cast<double>(
                            differentPixels
                            ) /
                        static_cast<double>(
                            total
                            );

                    return
                        (1.0 - ratio) * 100.0;
                }
            }
        }
    }

    const double differentRatio =
        static_cast<double>(
            differentPixels
            ) /
        static_cast<double>(
            total
            );

    return
        (1.0 - differentRatio) * 100.0;
}