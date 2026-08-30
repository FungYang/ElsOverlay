#include "transcendencevisionmanager.h"
#include "transcendencevisionconfig.h"
#include "transcendencecapturesetup.h"
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

// =========================================================
// CONSTRUCTOR
// =========================================================

TranscendenceVisionManager::TranscendenceVisionManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRoot,
    Overlay *overlay,
    QObject *parent
    )
    : QObject(parent),
    keyboard(keyboard),
    overlayRoot(overlayRoot),
    overlay(overlay)
{
    // =====================================================
    // SETTINGS
    // =====================================================
    loadSettings();
    loadIcon();
    // qDebug() << "TRANSCENDENCE: constructor - after loadIcon";

    // =====================================================
    // DELAY TIMER
    // =====================================================
    m_delayTimer.setSingleShot(true);
    m_delayTimer.setInterval(TranscendenceVisionConfig::DELAY_MS);

    connect(
        &m_delayTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::startScanning
        );
    // qDebug() << "TRANSCENDENCE: constructor - before delay timer";

    // =====================================================
    // SCAN TIMER
    // =====================================================
    connect(
        &m_scanTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::scanTick
        );

    // =====================================================
    // TASTO P - SALVA ICONA DI RIFERIMENTO
    // =====================================================
    if (keyboard)
    {
        connect(
            keyboard,
            &GlobalKeyboard::keyPressed,
            this,
            [this](int key)
            {
                if (key != 'P')
                    return;

                if (!captureSetup)
                    return;

                if (!captureSetup->isVisible())
                    return;

                saveCurrentIcon();
            }
            );

        // =================================================
        // INVIO - CONFERMA CONFIGURAZIONE
        // =================================================
        connect(
            keyboard,
            &GlobalKeyboard::confirmPressed,
            this,
            [this]()
            {
                if (!captureSetup)
                    return;

                if (!captureSetup->isVisible())
                    return;

                // =========================================
                // SALVA NUOVA AREA
                // =========================================
                m_searchArea = captureSetup->searchArea();
                saveSettings();

                // =========================================
                // AGGIORNA REGION ID
                // =========================================
                // =========================================
                // AGGIORNA REGIONE DI RICERCA
                // =========================================

                if (m_searchArea.isValid() && !m_searchArea.isEmpty())
                {
                    if (m_searchRegionId >= 0 &&
                        ScreenCapture::isRegionRegistered(m_searchRegionId))
                    {
                        // Manteniamo lo stesso ID e cambiamo
                        // direttamente il rettangolo.
                        if (!ScreenCapture::updateRegion(
                                m_searchRegionId,
                                m_searchArea))
                        {
                            qDebug()
                            << "TRANSCENDENCE:"
                            << "updateRegion fallito, registro nuova regione";

                            m_searchRegionId =
                                ScreenCapture::registerRegion(
                                    m_searchArea);
                        }
                    }
                    else
                    {
                        m_searchRegionId =
                            ScreenCapture::registerRegion(
                                m_searchArea);
                    }
                }
                else
                {
                    if (m_searchRegionId >= 0)
                    {
                        ScreenCapture::unregisterRegion(
                            m_searchRegionId);

                        m_searchRegionId = -1;
                    }
                }

                // =========================================
                // RICARICA CONFIGURAZIONE
                // =========================================
                loadIcon();

                // =========================================
                // CHIUDI SETUP
                // =========================================
                captureSetup->hide();

                // =========================================
                // RIPORTA GLI OVERLAY IN PRIMO PIANO
                // =========================================
                if (this->overlayRoot)
                {
                    this->overlayRoot->raiseAll();
                }
            }
            );
        // qDebug() << "TRANSCENDENCE: constructor COMPLETE";
    }
}
// =========================================================
// DESTRUCTOR
// =========================================================

TranscendenceVisionManager::~TranscendenceVisionManager()
{
    stopAll();

    if(m_searchRegionId >= 0)
    {
        ScreenCapture::unregisterRegion(
            m_searchRegionId
            );

        m_searchRegionId = -1;
    }
}


// =========================================================
// SETTINGS
// =========================================================

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

    // =====================================================
    // REGISTRA LA REGIONE
    // =====================================================
    if (m_searchRegionId >= 0)
    {
        ScreenCapture::unregisterRegion(m_searchRegionId);
        m_searchRegionId = -1;
    }

    if (m_searchArea.isValid() && !m_searchArea.isEmpty())
    {
        m_searchRegionId = ScreenCapture::registerRegion(m_searchArea);
    }

    // qDebug() << "TRANSCENDENCE SETTINGS:"
    //          << "area =" << m_searchArea
    //          << "regionId =" << m_searchRegionId;
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

// =========================================================
// ICON
// =========================================================

void TranscendenceVisionManager::loadIcon()
{
    const QString path = QCoreApplication::applicationDirPath() + "/images/transcendence_search.png";

    m_templateIcon.load(path);

    if (!m_templateIcon.isNull())
    {
        m_templateIcon = m_templateIcon.convertToFormat(QImage::Format_ARGB32);
    }
    else
    {
        qDebug() << "TRANSCENDENCE: impossibile caricare reference:" << path;
    }

    m_configured =
        !m_templateIcon.isNull() &&
        m_templateIcon.width() == TranscendenceVisionConfig::ICON_WIDTH &&
        m_templateIcon.height() == TranscendenceVisionConfig::ICON_HEIGHT &&
        m_searchArea.isValid() &&
        !m_searchArea.isEmpty() &&
        m_searchRegionId >= 0;

    // qDebug() << "TRANSCENDENCE CONFIGURED:" << m_configured
    //          << "icon =" << m_templateIcon.size()
    //          << "area =" << m_searchArea
    //          << "regionId =" << m_searchRegionId;
}

// =========================================================
// CONFIGURATION
// =========================================================

void TranscendenceVisionManager::configure()
{
    if (!overlayRoot)
        return;

    if (!captureSetup)
    {
        captureSetup = new TranscendenceCaptureSetup(overlayRoot);
        overlayRoot->registerOverlay(captureSetup);
    }

    captureSetup->setSearchArea(m_searchArea);
    captureSetup->show();
    captureSetup->raise();
    captureSetup->activateWindow();
    captureSetup->setFocus();

    overlayRoot->raiseAll();
}

// =========================================================
// SAVE REFERENCE ICON
// =========================================================

void TranscendenceVisionManager::saveCurrentIcon()
{
    if (!captureSetup)
        return;

    QRect iconRect = captureSetup->iconRect();

    if (iconRect.isNull() || iconRect.isEmpty())
    {
        captureSetup->showFeedback("ERRORE: area icona non valida");
        return;
    }

    // Nascondiamo il setup prima dello scatto
    captureSetup->hide();

    QTimer::singleShot(150, this, [this, iconRect]() {
        if (!captureSetup)
            return;

        QScreen *screen = QGuiApplication::primaryScreen();

        if (!screen)
        {
            captureSetup->show();
            captureSetup->showFeedback("ERRORE: schermo non disponibile");
            return;
        }

        // Cattura di riferimento tramite il fallback generico
        QImage icon = ScreenCapture::captureRegionReliable(screen, iconRect);

        captureSetup->show();
        captureSetup->raise();
        captureSetup->activateWindow();
        captureSetup->setFocus();

        if (icon.isNull())
        {
            captureSetup->showFeedback("ERRORE: cattura fallita");
            return;
        }

        QDir dir(QCoreApplication::applicationDirPath() + "/images");
        if (!dir.exists())
        {
            if (!dir.mkpath("."))
            {
                captureSetup->showFeedback("ERRORE: impossibile creare images");
                return;
            }
        }

        const QString path = QCoreApplication::applicationDirPath() + "/images/transcendence_search.png";

        if (!icon.save(path))
        {
            captureSetup->showFeedback("ERRORE: impossibile salvare icona");
            return;
        }

        loadIcon();
        captureSetup->showFeedback("ICON SAVED");
    });
}

// =========================================================
// ENABLE
// =========================================================

void TranscendenceVisionManager::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (!enabled)
    {
        stopAll();

        if (overlay)
            overlay->resetCooldown();

        return;
    }
}

// =========================================================
// STOP ALL
// =========================================================

void TranscendenceVisionManager::stopAll()
{
    m_delayTimer.stop();
    stopScanning();
}

// =========================================================
// COOLDOWN START
// =========================================================

void TranscendenceVisionManager::onCooldownStarted()
{
    if (!m_enabled || !m_configured)
        return;

    stopScanning();
    m_delayTimer.stop();
    m_delayTimer.start();
}

// =========================================================
// COOLDOWN RESET
// =========================================================

void TranscendenceVisionManager::onCooldownReset()
{
    stopAll();
}

// =========================================================
// START SCANNING
// =========================================================

void TranscendenceVisionManager::startScanning()
{
    if (!m_enabled || !m_configured || m_searchRegionId < 0)
        return;

    if (!ScreenCapture::isRegionRegistered(m_searchRegionId))
    {
        qDebug() << "TRANSCENDENCE: search region non piu' registrata";
        return;
    }

    m_scanTimer.start(TranscendenceVisionConfig::SCAN_INTERVAL_MS);
}

// =========================================================
// STOP SCANNING
// =========================================================

void TranscendenceVisionManager::stopScanning()
{
    m_scanTimer.stop();
}

// =========================================================
// SCAN
// =========================================================

void TranscendenceVisionManager::scanTick()
{
    if (!m_enabled || !m_configured || m_searchRegionId < 0)
    {
        stopScanning();
        return;
    }

    if (!ScreenCapture::isRegionRegistered(m_searchRegionId))
    {
        qDebug() << "TRANSCENDENCE: region ID non valido:" << m_searchRegionId;
        stopScanning();
        return;
    }

    // Acquisisci frame DXGI unico
    if (!ScreenCapture::beginFrame())
    {
        qDebug() << "TRANSCENDENCE: beginFrame() fallito";
        return;
    }

    // Estrai la sub-region definita tramite ID
    QImage area = ScreenCapture::captureRegion(m_searchRegionId);

    // Rilascia sempre il frame DXGI
    ScreenCapture::endFrame();

    if (area.isNull())
    {
        qDebug() << "TRANSCENDENCE: area catturata nulla";
        return;
    }

    QRect foundRect;
    double score = 0.0;

    const bool found = findIcon(area, foundRect, score);

    // qDebug() << "TRANSCENDENCE SCAN:"
    //          << "area =" << area.size()
    //          << "score =" << score
    //          << "found =" << found;

    if (!found)
        return;

    // qDebug() << "TRANSCENDENCE MATCH FOUND:"
    //          << "rect =" << foundRect
    //          << "score =" << score;

    stopScanning();

    if (overlay)
    {
        overlay->restartCooldown();
    }

    // qDebug() << "TRANSCENDENCE: cooldown riavviato";

    m_delayTimer.stop();
    m_delayTimer.start();
}

// =========================================================
// FIND ICON
// =========================================================

bool TranscendenceVisionManager::findIcon(
    const QImage &area,
    QRect &foundRect,
    double &score
    ) const
{
    score = 0.0;

    if (area.isNull() || m_templateIcon.isNull())
    {
        return false;
    }

    const int width = m_templateIcon.width();
    const int height = m_templateIcon.height();

    if (area.width() < width || area.height() < height)
    {
        return false;
    }

    for (int y = 0; y <= area.height() - height; ++y)
    {
        for (int x = 0; x <= area.width() - width; ++x)
        {
            const double current = compareAt(area, x, y);

            if (current > score)
            {
                score = current;
                foundRect = QRect(x, y, width, height);

                if (score >= TranscendenceVisionConfig::MATCH_THRESHOLD)
                {
                    return true;
                }
            }
        }
    }

    return score >= TranscendenceVisionConfig::MATCH_THRESHOLD;
}

// =========================================================
// COMPARE
// =========================================================

double TranscendenceVisionManager::compareAt(
    const QImage &area,
    int offsetX,
    int offsetY
    ) const
{
    const int width = m_templateIcon.width();
    const int height = m_templateIcon.height();
    const int total = width * height;

    if (total <= 0)
        return 0.0;

    constexpr int sampleCount = 16;

    const int maxDifferentSamples = static_cast<int>(
        (TranscendenceVisionConfig::FAST_TOLERANCE / 100.0) * sampleCount
        );

    const int sampleX[sampleCount] = {
        0, width / 4, width / 2, (width * 3) / 4,
        width - 1, width / 4, width / 2, (width * 3) / 4,
        0, width / 4, width / 2, (width * 3) / 4,
        width - 1, width / 4, width / 2, (width * 3) / 4
    };

    const int sampleY[sampleCount] = {
        0, 0, 0, 0,
        height / 4, height / 4, height / 4, height / 4,
        height / 2, height / 2, height / 2, height / 2,
        (height * 3) / 4, (height * 3) / 4, (height * 3) / 4, (height * 3) / 4
    };

    // Fast Sample Test
    int differentSamples = 0;

    for (int i = 0; i < sampleCount; ++i)
    {
        const QRgb *sourceLine = reinterpret_cast<const QRgb *>(
            area.constScanLine(offsetY + sampleY[i])
            );

        const QRgb *templateLine = reinterpret_cast<const QRgb *>(
            m_templateIcon.constScanLine(sampleY[i])
            );

        const QRgb sourcePixel = sourceLine[offsetX + sampleX[i]];
        const QRgb templatePixel = templateLine[sampleX[i]];

        const int dr = qAbs(qRed(sourcePixel) - qRed(templatePixel));
        const int dg = qAbs(qGreen(sourcePixel) - qGreen(templatePixel));
        const int db = qAbs(qBlue(sourcePixel) - qBlue(templatePixel));

        if (dr > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            dg > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            db > TranscendenceVisionConfig::PIXEL_TOLERANCE)
        {
            ++differentSamples;

            if (differentSamples > maxDifferentSamples)
            {
                return 0.0;
            }
        }
    }

    // Full Pixel Test
    const double maxDifferentRatio = 1.0 - (TranscendenceVisionConfig::MATCH_THRESHOLD / 100.0);
    const int maxDifferentPixels = static_cast<int>(maxDifferentRatio * total);

    int differentPixels = 0;

    for (int y = 0; y < height; ++y)
    {
        const QRgb *sourceLine = reinterpret_cast<const QRgb *>(
            area.constScanLine(offsetY + y)
            );

        const QRgb *templateLine = reinterpret_cast<const QRgb *>(
            m_templateIcon.constScanLine(y)
            );

        for (int x = 0; x < width; ++x)
        {
            const QRgb sourcePixel = sourceLine[offsetX + x];
            const QRgb templatePixel = templateLine[x];

            const int dr = qAbs(qRed(sourcePixel) - qRed(templatePixel));
            const int dg = qAbs(qGreen(sourcePixel) - qGreen(templatePixel));
            const int db = qAbs(qBlue(sourcePixel) - qBlue(templatePixel));

            if (dr > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                dg > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                db > TranscendenceVisionConfig::PIXEL_TOLERANCE)
            {
                ++differentPixels;

                if (differentPixels > maxDifferentPixels)
                {
                    const double ratio = static_cast<double>(differentPixels) / static_cast<double>(total);
                    return (1.0 - ratio) * 100.0;
                }
            }
        }
    }

    const double differentRatio = static_cast<double>(differentPixels) / static_cast<double>(total);
    return (1.0 - differentRatio) * 100.0;
}