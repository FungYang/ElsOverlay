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
    loadSettings();

    loadIcon();


    // =========================
    // DELAY TIMER (18s, single shot)
    // =========================

    m_delayTimer.setSingleShot(
        true
        );

    m_delayTimer.setInterval(
        TranscendenceVisionConfig::DELAY_MS
        );

    connect(
        &m_delayTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::startScanning
        );


    // =========================
    // SCAN TIMER
    // =========================

    connect(
        &m_scanTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::scanTick
        );


    // =========================
    // TASTO P: SALVA ICONA
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {
            if(key != 'P')
                return;

            if(!captureSetup)
                return;

            if(!captureSetup->isVisible())
                return;

            saveCurrentIcon();
        }
        );


    // =========================
    // INVIO: CONFERMA E CHIUDI
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::confirmPressed,
        this,
        [this]()
        {
            if(!captureSetup)
                return;

            if(!captureSetup->isVisible())
                return;


            m_searchArea =
                captureSetup->searchArea();


            saveSettings();


            captureSetup->hide();


            loadIcon();


            this->overlayRoot->raiseAll();
        }
        );
}


void TranscendenceVisionManager::loadSettings()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );

    m_searchArea =
        QRect(
            settings.value(
                        "Transcendence/AreaX",
                        700
                        ).toInt(),
            settings.value(
                        "Transcendence/AreaY",
                        600
                        ).toInt(),
            settings.value(
                        "Transcendence/AreaW",
                        300
                        ).toInt(),
            settings.value(
                        "Transcendence/AreaH",
                        150
                        ).toInt()
            );
}


void TranscendenceVisionManager::saveSettings()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );

    settings.setValue(
        "Transcendence/AreaX",
        m_searchArea.x()
        );

    settings.setValue(
        "Transcendence/AreaY",
        m_searchArea.y()
        );

    settings.setValue(
        "Transcendence/AreaW",
        m_searchArea.width()
        );

    settings.setValue(
        "Transcendence/AreaH",
        m_searchArea.height()
        );

    settings.sync();
}


void TranscendenceVisionManager::loadIcon()
{
    m_templateIcon.load(
        "images/transcendence_search.png"
        );

    if(!m_templateIcon.isNull())
    {
        m_templateIcon =
            m_templateIcon.convertToFormat(
                QImage::Format_ARGB32
                );
    }


    m_configured =
        !m_templateIcon.isNull()
        &&
        m_templateIcon.width() ==
            TranscendenceVisionConfig::ICON_WIDTH
        &&
        m_templateIcon.height() ==
            TranscendenceVisionConfig::ICON_HEIGHT
        &&
        m_searchArea.isValid()
        &&
        !m_searchArea.isEmpty();


    // qDebug()
    //     << "Transcendence configured:"
    //     << m_configured;
}


void TranscendenceVisionManager::configure()
{
    if(!captureSetup)
    {
        captureSetup =
            new TranscendenceCaptureSetup(
                overlayRoot
                );

        overlayRoot->registerOverlay(
            captureSetup
            );
    }


    captureSetup->setSearchArea(
        m_searchArea
        );


    captureSetup->show();

    captureSetup->raise();

    captureSetup->activateWindow();

    captureSetup->setFocus();


    overlayRoot->raiseAll();
}


void TranscendenceVisionManager::saveCurrentIcon()
{
    QDir dir(
        "images"
        );

    if(!dir.exists())
    {
        dir.mkpath(
            "."
            );
    }


    QRect iconRect =
        captureSetup->iconRect();


    captureSetup->setCaptureMode(
        true
        );


    // Diamo tempo a Qt di ridisegnare
    // senza i rettangoli, prima di catturare.

    QTimer::singleShot(
        100,
        this,
        [this, iconRect]()
        {
            QScreen *screen =
                QGuiApplication::primaryScreen();

            QImage icon =
                ScreenCapture::captureRegion(
                    screen,
                    iconRect
                    );


            captureSetup->setCaptureMode(
                false
                );


            if(icon.isNull())
            {
                captureSetup->showFeedback(
                    "ERRORE: cattura fallita"
                    );

                return;
            }


            icon.save(
                "images/transcendence_search.png"
                );


            captureSetup->showFeedback(
                "ICON SAVED"
                );
        }
        );
}


void TranscendenceVisionManager::setEnabled(
    bool enabled
    )
{
    m_enabled = enabled;


    if(!enabled)
    {
        stopAll();
    }
}


void TranscendenceVisionManager::stopAll()
{
    m_delayTimer.stop();

    stopScanning();
}


void TranscendenceVisionManager::onCooldownStarted()
{
    if(!m_enabled)
        return;

    if(!m_configured)
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
    if(!m_enabled)
        return;

    if(!m_configured)
        return;


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
    if(!m_enabled || !m_configured)
    {
        stopScanning();

        return;
    }


    QScreen *screen =
        QGuiApplication::primaryScreen();


    QImage area =
        ScreenCapture::captureRegion(
            screen,
            m_searchArea
            );


    if(area.isNull())
        return;


    QRect foundRect;

    double score = 0.0;


    bool found =
        findIcon(
            area,
            foundRect,
            score
            );


    if(!found)
        return;


    // qDebug()
    //     << "TRANSCENDENCE FOUND:"
    //     << score
    //     << "% at"
    //     << foundRect;


    // =========================
    // TROVATA: RESET COOLDOWN
    // =========================

    stopScanning();


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


    if(area.isNull() ||
        m_templateIcon.isNull())
    {
        return false;
    }


    const int width =
        m_templateIcon.width();

    const int height =
        m_templateIcon.height();


    if(area.width() < width ||
        area.height() < height)
    {
        return false;
    }


    for(int y = 0;
         y <= area.height() - height;
         ++y)
    {
        for(int x = 0;
             x <= area.width() - width;
             ++x)
        {
            double current =
                compareAt(
                    area,
                    x,
                    y
                    );


            if(current > score)
            {
                score = current;

                foundRect =
                    QRect(
                        x,
                        y,
                        width,
                        height
                        );


                if(score >=
                    TranscendenceVisionConfig::MATCH_THRESHOLD)
                {
                    return true;
                }
            }
        }
    }


    return
        score >=
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


    constexpr int sampleCount = 16;


    const int maxDifferentSamples =
        static_cast<int>(
            (TranscendenceVisionConfig::FAST_TOLERANCE / 100.0) *
            sampleCount
            );


    const int sampleX[sampleCount] =
        {
            0, width / 4, width / 2, (width * 3) / 4,
            width - 1, width / 4, width / 2, (width * 3) / 4,
            0, width / 4, width / 2, (width * 3) / 4,
            width - 1, width / 4, width / 2, (width * 3) / 4
        };

    const int sampleY[sampleCount] =
        {
            0, 0, 0, 0,
            height / 4, height / 4, height / 4, height / 4,
            height / 2, height / 2, height / 2, height / 2,
            (height * 3) / 4, (height * 3) / 4, (height * 3) / 4, (height * 3) / 4
        };


    int differentSamples = 0;


    for(int i = 0;
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


        if(dr > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            dg > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
            db > TranscendenceVisionConfig::PIXEL_TOLERANCE)
        {
            ++differentSamples;

            if(differentSamples > maxDifferentSamples)
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


    for(int y = 0;
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


        for(int x = 0;
             x < width;
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


            if(dr > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                dg > TranscendenceVisionConfig::PIXEL_TOLERANCE ||
                db > TranscendenceVisionConfig::PIXEL_TOLERANCE)
            {
                ++differentPixels;

                if(differentPixels > maxDifferentPixels)
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