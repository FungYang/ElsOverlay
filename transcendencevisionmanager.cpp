#include "transcendencevisionmanager.h"
#include "transcendencevisionconfig.h"
#include "transcendencecapturesetup.h"
#include "transcendenceprecisioncrop.h"
#include "globalkeyboard.h"
#include "overlayroot.h"
#include "overlay.h"
#include "screencapture.h"

#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QVector>
#include <QElapsedTimer>
#include <atomic>

TranscendenceVisionManager::TranscendenceVisionManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRootPtr,
    Overlay *overlay,
    QObject *parent
    )
    : QObject(parent),
    keyboard(keyboard),
    overlayRoot(overlayRootPtr),
    overlay(overlay)
{
    loadSettings();
    loadIcon();

    m_delayTimer.setSingleShot(true);
    m_delayTimer.setInterval(TranscendenceVisionConfig::DELAY_MS);

    connect(
        &m_delayTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::startScanning
        );

    connect(
        &m_scanTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::scanTick
        );

    if (keyboard)
    {
        // P apre il ritaglio preciso.
        connect(
            keyboard,
            &GlobalKeyboard::keyPressed,
            this,
            [this](int key)
            {
                if (precisionCrop)
                {
                    if (key == Qt::Key_Left)
                    {
                        precisionCrop->moveLeft();
                        return;
                    }

                    if (key == Qt::Key_Right)
                    {
                        precisionCrop->moveRight();
                        return;
                    }

                    if (key == Qt::Key_Up)
                    {
                        precisionCrop->moveUp();
                        return;
                    }

                    if (key == Qt::Key_Down)
                    {
                        precisionCrop->moveDown();
                        return;
                    }

                    if (key == Qt::Key_Escape)
                    {
                        closePrecisionCrop();
                        return;
                    }

                    // ENTER viene gestito dal widget stesso quando ha focus.
                    if (key == Qt::Key_Return || key == Qt::Key_Enter)
                    {
                        precisionCrop->setFocus();
                        QKeyEvent event(
                            QEvent::KeyPress,
                            Qt::Key_Return,
                            Qt::NoModifier
                            );
                        QCoreApplication::sendEvent(
                            precisionCrop,
                            &event
                            );
                        return;
                    }

                    // Mentre il menu è aperto P non deve fare altro.
                    if (key == 'P')
                        return;
                }

                if (key != 'P')
                    return;

                if (!captureSetup)
                    return;

                if (!captureSetup->isVisible())
                    return;

                openPrecisionCrop();
            }
            );

        // INVIO - CONFERMA CONFIGURAZIONE.
        connect(
            keyboard,
            &GlobalKeyboard::confirmPressed,
            this,
            [this]()
            {
                // Se il selettore preciso è aperto, ENTER serve a confermare
                // il ritaglio e NON a chiudere il setup principale.
                if (precisionCrop)
                {
                    precisionCrop->setFocus();

                    QKeyEvent event(
                        QEvent::KeyPress,
                        Qt::Key_Return,
                        Qt::NoModifier
                        );

                    QCoreApplication::sendEvent(
                        precisionCrop,
                        &event
                        );

                    return;
                }

                if (!captureSetup)
                    return;

                if (!captureSetup->isVisible())
                    return;

                const QRect newArea =
                    captureSetup->searchArea();

                if (newArea.isNull() || newArea.isEmpty())
                    return;

                m_searchArea = newArea;

                saveSettings();

                if (m_searchRegionId >= 0)
                {
                    ScreenCapture::unregisterRegion(
                        m_searchRegionId
                        );

                    m_searchRegionId = -1;
                }

                m_searchRegionId =
                    ScreenCapture::registerRegion(
                        m_searchArea
                        );

                loadIcon();

                qDebug()
                    << "TRANSCENDENCE:"
                    << "search area aggiornata:"
                    << m_searchArea
                    << "regionId ="
                    << m_searchRegionId;

                captureSetup->hide();

                if (overlayRoot)
                    overlayRoot->raiseAll();
            }
            );
    }
}

TranscendenceVisionManager::~TranscendenceVisionManager()
{
    closePrecisionCrop();
    stopAll();

    if (m_searchRegionId >= 0)
    {
        ScreenCapture::unregisterRegion(m_searchRegionId);
        m_searchRegionId = -1;
    }
}

void TranscendenceVisionManager::loadSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    m_searchArea = QRect(
        settings.value("Transcendence/AreaX", 700).toInt(),
        settings.value("Transcendence/AreaY", 600).toInt(),
        settings.value("Transcendence/AreaW", 300).toInt(),
        settings.value("Transcendence/AreaH", 150).toInt()
        );

    m_iconWidth =
        settings.value(
                    "Transcendence/IconWidth",
                    TranscendenceVisionConfig::ICON_WIDTH
                    ).toInt();

    m_iconHeight =
        settings.value(
                    "Transcendence/IconHeight",
                    TranscendenceVisionConfig::ICON_HEIGHT
                    ).toInt();

    if (m_iconWidth < 1)
        m_iconWidth = TranscendenceVisionConfig::ICON_WIDTH;

    if (m_iconHeight < 1)
        m_iconHeight = TranscendenceVisionConfig::ICON_HEIGHT;

    if (m_searchRegionId >= 0)
    {
        ScreenCapture::unregisterRegion(m_searchRegionId);
        m_searchRegionId = -1;
    }

    if (m_searchArea.isValid() && !m_searchArea.isEmpty())
        m_searchRegionId = ScreenCapture::registerRegion(m_searchArea);
}

void TranscendenceVisionManager::saveSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    settings.setValue("Transcendence/AreaX", m_searchArea.x());
    settings.setValue("Transcendence/AreaY", m_searchArea.y());
    settings.setValue("Transcendence/AreaW", m_searchArea.width());
    settings.setValue("Transcendence/AreaH", m_searchArea.height());

    settings.setValue(
        "Transcendence/IconWidth",
        m_iconWidth
        );

    settings.setValue(
        "Transcendence/IconHeight",
        m_iconHeight
        );

    settings.sync();
}

void TranscendenceVisionManager::loadIcon()
{
    const QString path =
        QCoreApplication::applicationDirPath() +
        "/images/transcendence_search.png";

    m_templateIcon.load(path);

    if (!m_templateIcon.isNull())
    {
        m_templateIcon =
            m_templateIcon.convertToFormat(
                QImage::Format_ARGB32
                );
    }
    else
    {
        qDebug()
        << "TRANSCENDENCE: impossibile caricare reference:"
        << path;
    }

    m_configured =
        !m_templateIcon.isNull() &&
        m_templateIcon.width() == m_iconWidth &&
        m_templateIcon.height() == m_iconHeight &&
        m_searchArea.isValid() &&
        !m_searchArea.isEmpty() &&
        m_searchRegionId >= 0;
}

void TranscendenceVisionManager::configure()
{
    if (!overlayRoot)
        return;

    if (!captureSetup)
    {
        captureSetup =
            new TranscendenceCaptureSetup(overlayRoot);

        overlayRoot->registerOverlay(captureSetup);
    }

    captureSetup->setSearchArea(m_searchArea);
    captureSetup->show();
    captureSetup->raise();
    captureSetup->activateWindow();
    captureSetup->setFocus();

    overlayRoot->raiseAll();
}

void TranscendenceVisionManager::openPrecisionCrop()
{
    if (!captureSetup)
        return;

    if (precisionCrop)
    {
        precisionCrop->raise();
        precisionCrop->activateWindow();
        precisionCrop->setFocus();
        return;
    }

    const QRect bigRect =
        captureSetup->iconRect();

    if (bigRect.width() !=
            TranscendenceVisionConfig::ICON_BOX_WIDTH ||
        bigRect.height() !=
            TranscendenceVisionConfig::ICON_BOX_HEIGHT)
    {
        captureSetup->showFeedback(
            "ERRORE: riquadro giallo non valido"
            );
        return;
    }

    // Nascondiamo i rettangoli prima dello screenshot.
    captureSetup->hide();

    QTimer::singleShot(
        120,
        this,
        [this, bigRect]()
        {
            if (!captureSetup)
                return;

            QScreen *screen =
                QGuiApplication::primaryScreen();

            if (!screen)
            {
                captureSetup->show();
                captureSetup->showFeedback(
                    "ERRORE: schermo non disponibile"
                    );
                return;
            }

            // Cattura il riquadro grande per l'anteprima interattiva.
            // La dimensione del crop iniziale viene presa dal config.
            const QImage source =
                ScreenCapture::captureRegionReliable(
                    screen,
                    bigRect
                    );

            if (source.isNull())
            {
                captureSetup->show();
                captureSetup->raise();
                captureSetup->activateWindow();
                captureSetup->setFocus();

                captureSetup->showFeedback(
                    "ERRORE: cattura fallita"
                    );

                return;
            }

            precisionCrop =
                new TranscendencePrecisionCrop(
                    source,
                    QSize(
                        m_iconWidth,
                        m_iconHeight
                        )
                    );

            connect(
                precisionCrop,
                &TranscendencePrecisionCrop::accepted,
                this,
                [this](const QImage &image, const QSize &size)
                {
                    savePreciseIcon(
                        image,
                        size
                        );
                }
                );

            connect(
                precisionCrop,
                &TranscendencePrecisionCrop::canceled,
                this,
                [this]()
                {
                    closePrecisionCrop();

                    if (captureSetup)
                    {
                        captureSetup->show();
                        captureSetup->raise();
                        captureSetup->activateWindow();
                        captureSetup->setFocus();
                    }
                }
                );

            connect(
                precisionCrop,
                &QObject::destroyed,
                this,
                [this]()
                {
                    precisionCrop = nullptr;
                }
                );

            precisionCrop->show();
            precisionCrop->raise();
            precisionCrop->activateWindow();
            precisionCrop->setFocus();
        }
        );
}

void TranscendenceVisionManager::savePreciseIcon(
    const QImage &icon,
    const QSize &size
    )
{
    if (icon.isNull())
        return;

    if (size.width() <= 0 ||
        size.height() <= 0)
    {
        return;
    }

    if (icon.size() != size)
    {
        qDebug()
        << "TRANSCENDENCE: dimensione crop incoerente:"
        << "image =" << icon.size()
        << "size =" << size;

        return;
    }

    m_iconWidth = size.width();
    m_iconHeight = size.height();

    saveSettings();

    QDir dir(
        QCoreApplication::applicationDirPath() +
        "/images"
        );

    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            if (captureSetup)
                captureSetup->showFeedback(
                    "ERRORE: impossibile creare images"
                    );

            return;
        }
    }

    const QString path =
        QCoreApplication::applicationDirPath() +
        "/images/transcendence_search.png";

    if (!icon.save(path))
    {
        if (captureSetup)
            captureSetup->showFeedback(
                "ERRORE: impossibile salvare icona"
                );

        return;
    }

    loadIcon();

    closePrecisionCrop();

    if (captureSetup)
    {
        captureSetup->show();
        captureSetup->raise();
        captureSetup->activateWindow();
        captureSetup->setFocus();
        captureSetup->showFeedback(
            QString("ICONA %1x%2 SALVATA")
                .arg(m_iconWidth)
                .arg(m_iconHeight)
            );
    }
}

void TranscendenceVisionManager::closePrecisionCrop()
{
    if (!precisionCrop)
        return;

    precisionCrop->close();
    precisionCrop->deleteLater();
    precisionCrop = nullptr;
}

void TranscendenceVisionManager::saveCurrentIcon()
{
    // Manteniamo il metodo per compatibilità con eventuali altre chiamate.
    // Il nuovo flusso passa da P -> openPrecisionCrop().
    openPrecisionCrop();
}

void TranscendenceVisionManager::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (!enabled)
    {
        stopAll();

        if (overlay)
            overlay->setEnabled(false);

        return;
    }

    if (overlay)
        overlay->setEnabled(true);
}

void TranscendenceVisionManager::stopAll()
{
    m_delayTimer.stop();
    stopScanning();
}

void TranscendenceVisionManager::onCooldownStarted()
{
    if (!m_enabled || !m_configured)
        return;

    stopScanning();
    m_delayTimer.stop();
    m_delayTimer.start();
}

void TranscendenceVisionManager::onCooldownReset()
{
    stopAll();
}

void TranscendenceVisionManager::startScanning()
{
    if (!m_enabled ||
        !m_configured ||
        m_searchRegionId < 0)
    {
        return;
    }

    if (!ScreenCapture::isRegionRegistered(m_searchRegionId))
    {
        qDebug()
        << "TRANSCENDENCE: search region non piu' registrata";

        return;
    }

    m_scanTimer.start(
        TranscendenceVisionConfig::SCAN_INTERVAL_MS
        );
}

void TranscendenceVisionManager::stopScanning()
{
    m_scanTimer.stop();
}

void TranscendenceVisionManager::scanTick()
{
    if (!m_enabled ||
        !m_configured ||
        m_searchRegionId < 0)
    {
        stopScanning();
        return;
    }

    if (!ScreenCapture::isRegionRegistered(m_searchRegionId))
    {
        qDebug()
        << "TRANSCENDENCE: region ID non valido:"
        << m_searchRegionId;

        stopScanning();
        return;
    }

    if (!ScreenCapture::beginFrame())
    {
        qDebug()
        << "TRANSCENDENCE: beginFrame() fallito";

        return;
    }

    QImage area =
        ScreenCapture::captureRegion(
            m_searchRegionId
            );

    ScreenCapture::endFrame();

    if (area.isNull())
    {
        qDebug()
        << "TRANSCENDENCE: area catturata nulla";

        return;
    }

    QRect foundRect;
    double score = 0.0;

    const bool found =
        findIcon(
            area,
            foundRect,
            score
            );

#ifdef QT_DEBUG
    // ==== BLOCCO DIAGNOSTICO - SOLO BUILD DI DEBUG ====
    qDebug()
        << "TRANSCENDENCE: score ="
        << score
        << "soglia ="
        << TranscendenceVisionConfig::MATCH_THRESHOLD
        << "found ="
        << found;

    {
        static QElapsedTimer dumpTimer;

        const bool shouldDump =
            found ||
            !dumpTimer.isValid() ||
            dumpTimer.elapsed() >= 1000;

        if (shouldDump)
        {
            const QString debugDir =
                QCoreApplication::applicationDirPath() +
                "/images/debug";

            QDir().mkpath(debugDir);

            const QString areaSuffix =
                found ? "_found" : "";

            area.save(
                debugDir +
                "/last_captured_area" +
                areaSuffix +
                ".png"
                );

            if (!foundRect.isNull() &&
                foundRect.width() == m_templateIcon.width() &&
                foundRect.height() == m_templateIcon.height())
            {
                const QImage candidate =
                    area.copy(foundRect);

                candidate.save(
                    debugDir +
                    "/last_best_candidate" +
                    areaSuffix +
                    ".png"
                    );

                long long sumR = 0;
                long long sumG = 0;
                long long sumB = 0;

                int maxR = 0;
                int maxG = 0;
                int maxB = 0;

                long long sumBorderR = 0;
                long long sumBorderG = 0;
                long long sumBorderB = 0;
                int maxBorderR = 0;
                int maxBorderG = 0;
                int maxBorderB = 0;
                int borderCount = 0;

                long long sumInnerR = 0;
                long long sumInnerG = 0;
                long long sumInnerB = 0;
                int maxInnerR = 0;
                int maxInnerG = 0;
                int maxInnerB = 0;
                int innerCount = 0;

                const int w = m_templateIcon.width();
                const int h = m_templateIcon.height();

                for (int y = 0; y < h; ++y)
                {
                    const QRgb *cLine =
                        reinterpret_cast<const QRgb *>(
                            candidate.constScanLine(y)
                            );

                    const QRgb *tLine =
                        reinterpret_cast<const QRgb *>(
                            m_templateIcon.constScanLine(y)
                            );

                    const bool borderRow =
                        (y == 0 || y == h - 1);

                    for (int x = 0; x < w; ++x)
                    {
                        const int dr =
                            qAbs(qRed(cLine[x]) - qRed(tLine[x]));

                        const int dg =
                            qAbs(qGreen(cLine[x]) - qGreen(tLine[x]));

                        const int db =
                            qAbs(qBlue(cLine[x]) - qBlue(tLine[x]));

                        sumR += dr;
                        sumG += dg;
                        sumB += db;

                        maxR = qMax(maxR, dr);
                        maxG = qMax(maxG, dg);
                        maxB = qMax(maxB, db);

                        const bool isBorder =
                            borderRow ||
                            x == 0 ||
                            x == w - 1;

                        if (isBorder)
                        {
                            sumBorderR += dr;
                            sumBorderG += dg;
                            sumBorderB += db;

                            maxBorderR = qMax(maxBorderR, dr);
                            maxBorderG = qMax(maxBorderG, dg);
                            maxBorderB = qMax(maxBorderB, db);

                            ++borderCount;
                        }
                        else
                        {
                            sumInnerR += dr;
                            sumInnerG += dg;
                            sumInnerB += db;

                            maxInnerR = qMax(maxInnerR, dr);
                            maxInnerG = qMax(maxInnerG, dg);
                            maxInnerB = qMax(maxInnerB, db);

                            ++innerCount;
                        }
                    }
                }

                const int total = w * h;

                qDebug()
                    << "TRANSCENDENCE DIFF: media R/G/B ="
                    << (double(sumR) / total)
                    << (double(sumG) / total)
                    << (double(sumB) / total)
                    << " massima R/G/B ="
                    << maxR << maxG << maxB
                    << " (PIXEL_TOLERANCE ="
                    << TranscendenceVisionConfig::PIXEL_TOLERANCE
                    << ")";

                if (borderCount > 0)
                {
                    qDebug()
                    << "TRANSCENDENCE DIFF BORDO"
                    << "(" << borderCount << "px ):"
                    << "media R/G/B ="
                    << (double(sumBorderR) / borderCount)
                    << (double(sumBorderG) / borderCount)
                    << (double(sumBorderB) / borderCount)
                    << " massima R/G/B ="
                    << maxBorderR << maxBorderG << maxBorderB;
                }

                if (innerCount > 0)
                {
                    qDebug()
                    << "TRANSCENDENCE DIFF INTERNO"
                    << "(" << innerCount << "px ):"
                    << "media R/G/B ="
                    << (double(sumInnerR) / innerCount)
                    << (double(sumInnerG) / innerCount)
                    << (double(sumInnerB) / innerCount)
                    << " massima R/G/B ="
                    << maxInnerR << maxInnerG << maxInnerB;
                }
            }

            dumpTimer.restart();
        }
    }
    // ==== FINE BLOCCO DIAGNOSTICO ====
#endif

    if (!found)
        return;

    stopScanning();

    if (overlay)
        overlay->restartCooldown();

    m_delayTimer.stop();
    m_delayTimer.start();
}

bool TranscendenceVisionManager::findIcon(
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
        qMax(1, QThread::idealThreadCount());

    const int totalRows =
        maxY + 1;

    constexpr int CHUNKS_PER_THREAD = 4;

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

double TranscendenceVisionManager::compareAt(
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
            sourceLine[
                offsetX + sampleX[i]
        ];

        const QRgb templatePixel =
            templateLine[
                sampleX[i]
        ];

        const int dr =
            qAbs(
                qRed(sourcePixel) -
                qRed(templatePixel)
                );

        const int dg =
            qAbs(
                qGreen(sourcePixel) -
                qGreen(templatePixel)
                );

        const int db =
            qAbs(
                qBlue(sourcePixel) -
                qBlue(templatePixel)
                );

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

        for (int x = 1;
             x <= width - 2;
             ++x)
        {
            const QRgb sourcePixel =
                sourceLine[
                    offsetX + x
            ];

            const QRgb templatePixel =
                templateLine[x];

            const int dr =
                qAbs(
                    qRed(sourcePixel) -
                    qRed(templatePixel)
                    );

            const int dg =
                qAbs(
                    qGreen(sourcePixel) -
                    qGreen(templatePixel)
                    );

            const int db =
                qAbs(
                    qBlue(sourcePixel) -
                    qBlue(templatePixel)
                    );

            if (dr >
                    TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                dg >
                    TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                db >
                    TranscendenceVisionConfig::PIXEL_TOLERANCE)
            {
                ++differentPixels;

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
                        (1.0 - ratio) *
                        100.0;
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
        (1.0 - differentRatio) *
        100.0;
}