#include "screenmirrormanager.h"

#include "capturecoordinator.h"
#include "globalkeyboard.h"
#include "overlayroot.h"
#include "screenmirroroverlay.h"
#include "screenmirrorsetup.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QSettings>


ScreenMirrorManager::ScreenMirrorManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRoot,
    QObject *parent
    )
    : QObject(parent),
    m_keyboard(keyboard),
    m_overlayRoot(overlayRoot)
{
    // Regioni iniziali (se non c'e' nulla di salvato).
    m_source =
        QRect(100, 100, 300, 200);

    m_dest =
        QRect(500, 100, 300, 200);

    loadRects();


    // ========================================================
    // INVIO = CONFERMA CONFIGURAZIONE
    // ========================================================

    if(m_keyboard)
    {
        connect(
            m_keyboard,
            &GlobalKeyboard::confirmPressed,
            this,
            [this]()
            {
                if(!m_setup || !m_setup->isVisible())
                {
                    return;
                }

                m_source =
                    m_setup->sourceRect();

                m_dest =
                    m_setup->destRect();

                saveRects();

                m_setup->hide();

                // Applica subito le nuove regioni senza riavviare
                // il programma e senza dover fare OFF/ON.
                reloadConfiguration();

                if(m_overlayRoot)
                {
                    m_overlayRoot->raiseAll();
                }
            }
            );
    }
}


ScreenMirrorManager::~ScreenMirrorManager()
{
    stopCapture();

    delete m_overlay;
    delete m_setup;
}


// ============================================================
// SETTINGS
// ============================================================

QString ScreenMirrorManager::settingsPath()
{
    return QCoreApplication::applicationDirPath() +
           "/ElsOverlay.ini";
}


// Come le altre funzioni: le regioni dipendono dalla risoluzione.
QString ScreenMirrorManager::resolutionSuffix()
{
    QScreen *screen =
        QGuiApplication::primaryScreen();

    if(!screen)
    {
        return QString();
    }

    return QString("_%1x%2")
        .arg(screen->size().width())
        .arg(screen->size().height());
}


void ScreenMirrorManager::loadRects()
{
    QSettings settings(
        settingsPath(),
        QSettings::IniFormat
        );

    const QString suffix =
        resolutionSuffix();

    auto readRect =
        [&](const QString &name, const QRect &fallback)
    {
        const QString base =
            "ScreenMirror/" + name;

        QRect rect(
            settings.value(base + "X" + suffix, fallback.x()).toInt(),
            settings.value(base + "Y" + suffix, fallback.y()).toInt(),
            settings.value(base + "W" + suffix, fallback.width()).toInt(),
            settings.value(base + "H" + suffix, fallback.height()).toInt()
            );

        // Un valore rovinato (dimensioni nulle) non deve rompere
        // la cattura: si torna al default.
        if(rect.width() <= 0 || rect.height() <= 0)
        {
            rect = fallback;
        }

        return rect;
    };

    m_source =
        readRect("Source", m_source);

    m_dest =
        readRect("Dest", m_dest);
}


void ScreenMirrorManager::saveRects() const
{
    QSettings settings(
        settingsPath(),
        QSettings::IniFormat
        );

    const QString suffix =
        resolutionSuffix();

    auto writeRect =
        [&](const QString &name, const QRect &rect)
    {
        const QString base =
            "ScreenMirror/" + name;

        settings.setValue(base + "X" + suffix, rect.x());
        settings.setValue(base + "Y" + suffix, rect.y());
        settings.setValue(base + "W" + suffix, rect.width());
        settings.setValue(base + "H" + suffix, rect.height());
    };

    writeRect("Source", m_source);
    writeRect("Dest", m_dest);

    settings.sync();
}


// ============================================================
// CONFIGURE
// ============================================================

void ScreenMirrorManager::configure()
{
    if(!m_setup)
    {
        m_setup =
            new ScreenMirrorSetup(
                nullptr
                );

        if(m_overlayRoot)
        {
            m_overlayRoot->registerOverlay(
                m_setup
                );
        }
    }

    // Rilegge i valori salvati: cosi' chiudere con Esc scarta
    // le modifiche non confermate.
    loadRects();

    m_setup->setRects(
        m_source,
        m_dest
        );

    m_setup->show();
    m_setup->raise();
    m_setup->activateWindow();
    m_setup->setFocus();
}


// ============================================================
// ENABLE / RELOAD
// ============================================================

void ScreenMirrorManager::setEnabled(
    bool enabled
    )
{
    if(m_enabled == enabled)
    {
        return;
    }

    m_enabled =
        enabled;

    if(!m_enabled)
    {
        stopCapture();
        return;
    }

    loadRects();

    startCapture();
}


void ScreenMirrorManager::reloadConfiguration()
{
    if(!m_enabled)
    {
        return;
    }

    stopCapture();

    loadRects();

    startCapture();
}


// ============================================================
// CAPTURE
// ============================================================

void ScreenMirrorManager::startCapture()
{
    if(!m_overlay)
    {
        m_overlay =
            new ScreenMirrorOverlay(
                nullptr
                );

        // Registrato come tutti gli altri overlay: click-through
        // globale, tasto Canc, "porta tutto in primo piano".
        if(m_overlayRoot)
        {
            m_overlayRoot->registerOverlay(
                m_overlay
                );
        }
    }

    m_overlay->setRegion(
        m_dest
        );

    m_overlay->clearImage();


    // ------------------------------------------------------------
    // Regione sorgente sul CaptureCoordinator esistente.
    // ------------------------------------------------------------

    m_regionId =
        CaptureCoordinator::instance()->registerRegion(
            m_source
            );

    if(m_regionId >= 0)
    {
        CaptureCoordinator::instance()->subscribe(
            m_regionId,
            CAPTURE_INTERVAL_MS,
            this,
            "onFrameCaptured"
            );
    }


    // Se gli overlay sono nascosti con il tasto globale resta
    // nascosto (verra' mostrato insieme agli altri).
    if(!m_overlayRoot ||
        m_overlayRoot->areOverlaysVisible())
    {
        m_overlay->show();
    }
}


void ScreenMirrorManager::stopCapture()
{
    if(m_regionId >= 0)
    {
        CaptureCoordinator::instance()->unsubscribe(
            m_regionId
            );

        CaptureCoordinator::instance()->unregisterRegion(
            m_regionId
            );

        m_regionId = -1;
    }

    if(m_overlay)
    {
        m_overlay->hide();
    }
}


void ScreenMirrorManager::onFrameCaptured(
    quint64,
    QImage frame
    )
{
    if(!m_enabled || !m_overlay)
    {
        return;
    }

    m_overlay->setImage(
        frame
        );
}
