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
        m_templateIcon.width() ==
            TranscendenceVisionConfig::ICON_WIDTH &&
        m_templateIcon.height() ==
            TranscendenceVisionConfig::ICON_HEIGHT &&
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

            // Cattura 84x84. Il selettore preciso sceglierà
            // al suo interno esattamente 28x28 pixel.
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
                new TranscendencePrecisionCrop(source);

            connect(
                precisionCrop,
                &TranscendencePrecisionCrop::accepted,
                this,
                [this](const QImage &image)
                {
                    savePreciseIcon(image);
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
    const QImage &icon
    )
{
    if (icon.isNull())
        return;

    if (icon.width() != TranscendenceVisionConfig::ICON_WIDTH ||
        icon.height() != TranscendenceVisionConfig::ICON_HEIGHT)
    {
        qDebug()
        << "TRANSCENDENCE: ritaglio non 28x28:"
        << icon.size();

        return;
    }

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
            "ICONA 28x28 SALVATA"
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

    for (int y = 0;
         y <= area.height() - height;
         ++y)
    {
        for (int x = 0;
             x <= area.width() - width;
             ++x)
        {
            const double current =
                compareAt(
                    area,
                    x,
                    y
                    );

            if (current > score)
            {
                score = current;

                foundRect =
                    QRect(
                        x,
                        y,
                        width,
                        height
                        );

                if (score >=
                    TranscendenceVisionConfig::MATCH_THRESHOLD)
                {
                    return true;
                }
            }
        }
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

    const int total =
        width * height;

    if (total <= 0)
        return 0.0;

    constexpr int sampleCount = 16;

    const int maxDifferentSamples =
        static_cast<int>(
            (TranscendenceVisionConfig::FAST_TOLERANCE / 100.0) *
            sampleCount
            );

    const int sampleX[sampleCount] = {
        0,
        width / 4,
        width / 2,
        (width * 3) / 4,
        width - 1,
        width / 4,
        width / 2,
        (width * 3) / 4,
        0,
        width / 4,
        width / 2,
        (width * 3) / 4,
        width - 1,
        width / 4,
        width / 2,
        (width * 3) / 4
    };

    const int sampleY[sampleCount] = {
        0,
        0,
        0,
        0,
        height / 4,
        height / 4,
        height / 4,
        height / 4,
        height / 2,
        height / 2,
        height / 2,
        height / 2,
        (height * 3) / 4,
        (height * 3) / 4,
        (height * 3) / 4,
        (height * 3) / 4
    };

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

    for (int y = 0;
         y < height;
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

        for (int x = 0;
             x < width;
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