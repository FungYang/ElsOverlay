#include "buffvisionmanager.h"

#include "globalkeyboard.h"
#include "buffvisioncore.h"
#include "buffvisionoverlay.h"
#include "buffvisioncapturesetup.h"
#include "overlayroot.h"
#include "buffvisionconfig.h"

#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>


#ifdef QT_DEBUG
#include "buffvisiondebug.h"
#endif


BuffVisionManager::BuffVisionManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRoot,
    QObject *parent
    )
    : QObject(parent),
    overlayRoot(overlayRoot),
    keyboard(keyboard)
{
    // =========================
    // CORE
    // =========================

    core =
        new BuffVisionCore(
            this
            );


    // =========================
    // CAPTURE
    // =========================

    capture =
        new BuffVisionCapture(
            this
            );


    if(!capture)
    {
        // qDebug()
        // << "BUFFVISION: ERRORE - BuffVisionCapture non creato";
    }
    else
    {
        if(!capture->loadSettings())
        {
            // qDebug()
            // << "BUFFVISION: caricamento capture settings fallito";
        }
    }


    // =========================
    // DETECTOR
    // =========================

    detector =
        new BuffVisionDetector(
            this
            );


    QScreen *screen =
        QGuiApplication::primaryScreen();

    QString modelName =
        "best_1080.onnx";

    if(screen)
    {
        const QSize resolution =
            screen->size();

        if(
            BuffVisionConfig::is2K(
                resolution
                )
            )
        {
            modelName =
                "best_2k.onnx";
        }
    }

    const QString modelPath =
        QCoreApplication::applicationDirPath() +
        "/models/" +
        modelName;


    configured =
        detector->loadModel(
            modelPath
            );


    if(configured)
    {
        // qDebug()
        // << "BUFFVISION: DigitDetector caricato";
    }
    else
    {
        // qDebug()
        // << "BUFFVISION: ERRORE - DigitDetector non caricato";
    }


    // =========================
    // DEBUG
    // =========================

#ifdef QT_DEBUG

    debugWindow =
        new BuffVisionDebug(
            this->overlayRoot
            );


    this->overlayRoot->registerOverlay(
        debugWindow
        );


    // =========================
    // RISOLUZIONE DEBUG
    // =========================




    if(screen)
    {
        const QSize resolution =
            screen->size();


        const QSize cropSize =
            BuffVisionConfig::cropSizeForScreen(
                resolution
                );


        debugWindow->setResolution(
            resolution.width(),
            resolution.height(),
            cropSize
            );
    }


    debugWindow->hide();

#endif


    // =========================
    // ATMA OVERLAY
    // =========================

    overlay =
        new BuffVisionOverlay(
            core,
            this->overlayRoot
            );


    this->overlayRoot->registerOverlay(
        overlay
        );


    /*
     * Atma parte OFF.
     *
     * Non viene avviato alcun tracking
     * durante la costruzione.
     */

    overlay->hide();


    // =========================
    // CAPTURE SETUP
    // =========================

    captureSetup = nullptr;


    // =========================
    // TIMER VISIONE
    // =========================

    connect(
        &visionTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            // =========================
            // ENABLED
            // =========================

            if(!enabled)
            {
                visionTimer.stop();

                return;
            }


            // =========================
            // CONFIGURED
            // =========================

            if(!configured)
            {
                return;
            }


            ++visionCycle;


            // =========================
            // CAPTURE
            //
            // UN SOLO AcquireNextFrame()
            // per CROP1 + CROP2.
            // =========================

            QPixmap current1;
            QPixmap current2;


            if(capture->beginCapture())
            {
                current1 =
                    capture->captureCrop1();


                current2 =
                    capture->captureCrop2();


                capture->endCapture();
            }
            else
            {
                // qDebug()
                // << "BUFFVISION: frame acquisition failed";
            }


            // =========================
            // DETECTION
            // =========================

            const int number1 =
                detector->detectCrop1(
                    current1
                    );


            const int number2 =
                detector->detectCrop2(
                    current2
                    );


            // =========================
            // DEBUG
            // =========================

            // qDebug()
            //     << "BUFFVISION:"
            //     << "crop1 =" << number1
            //     << "previous =" << lastCrop1Number
            //     << "| crop2 =" << number2
            //     << "previous =" << lastCrop2Number;


#ifdef QT_DEBUG

            if(debugWindow)
            {
                debugWindow->updateNumbers(
                    number1,
                    number2
                    );
            }

#endif


            // =========================
            // CROP 1 EVENT
            // =========================
            //
            // 1000 = nessuna lettura valida.
            //
            // Esempio:
            //
            // 50 -> 50 -> 49
            //
            // evento a 49.
            //
            // 50 -> 1000 -> 49
            //
            // evento a 49.
            //
            // 50 -> 1000
            //
            // nessun evento e previous rimane 50.
            //

            if(
                number1 != 1000 &&
                lastCrop1Number != 1000 &&
                number1 == lastCrop1Number - 1
                )
            {
                crop1EventTime =
                    eventTimer.elapsed();

                crop1EventCycle =
                    visionCycle;


                core->onCrop1Event();


#ifdef QT_DEBUG

                if(debugWindow)
                {
                    debugWindow->setLastEvent(
                        QString(
                            "Crop 1 -> %1"
                            )
                            .arg(number1)
                        );
                }

#endif
            }


            // =========================
            // CROP 2 EVENT
            // =========================

            if(
                number2 != 1000 &&
                lastCrop2Number != 1000 &&
                number2 == lastCrop2Number - 1
                )
            {
                crop2EventTime =
                    eventTimer.elapsed();

                crop2EventCycle =
                    visionCycle;


                core->onCrop2Event();


#ifdef QT_DEBUG

                if(debugWindow)
                {
                    debugWindow->setLastEvent(
                        QString(
                            "Crop 2 -> %1"
                            )
                            .arg(number2)
                        );
                }

#endif
            }


            // =========================
            // UPDATE PREVIOUS NUMBER
            // =========================
            //
            // 1000 NON sovrascrive
            // l'ultimo valore valido.
            //

            if(number1 != 1000)
            {
                lastCrop1Number =
                    number1;
            }


            if(number2 != 1000)
            {
                lastCrop2Number =
                    number2;
            }
        }
        );


    // =========================
    // KEYBOARD
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {
            // =========================
            // ACTIONS 1 - 6
            // =========================

            if(
                key >= '1' &&
                key <= '6'
                )
            {
                if(!enabled)
                {
                    return;
                }


                core->registerAction();
            }
        }
        );


    // =========================
    // ENTER
    // CONFERMA CONFIGURAZIONE
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::confirmPressed,
        this,
        [this]()
        {
            if(!captureSetup)
            {
                return;
            }


            if(!captureSetup->isVisible())
            {
                return;
            }


            // =========================
            // SALVA POSIZIONI CROP
            // =========================

            captureSetup->saveSettings();


            // =========================
            // APPLICA I CROP
            // =========================

            capture->setCropAreas(
                captureSetup->getCropRect1(),
                captureSetup->getCropRect2()
                );


            // =========================
            // CONFIGURAZIONE COMPLETATA
            // =========================

            configured = true;


            captureSetup->hide();


            // =========================
            // RESET LETTURE PRECEDENTI
            // =========================

            lastCrop1Number = 1000;
            lastCrop2Number = 1000;


            this->overlayRoot->raiseAll();
        }
        );


    // =========================
    // RESET GLOBALE
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::resetPressed,
        this,
        [this]()
        {
            resetTracking();


            if(captureSetup)
            {
                captureSetup->hide();
            }


            this->overlayRoot->raiseAll();
        }
        );
}


// ============================================================
// START TRACKING
// ============================================================

void BuffVisionManager::startTracking()
{
    if(!enabled)
    {
        return;
    }


    if(!configured)
    {
        return;
    }


    // =========================
    // RESET CORE
    // =========================

    core->reset();


    // =========================
    // RESET NUMERI PRECEDENTI
    // =========================

    lastCrop1Number = 1000;
    lastCrop2Number = 1000;


    // =========================
    // AVVIO TRACKING
    // =========================

    core->startTracking();


    // =========================
    // RESET CONTATORI
    // =========================

    visionCycle = 0;


    crop1EventTime = -1;
    crop2EventTime = -1;


    crop1EventCycle = -1;
    crop2EventCycle = -1;


    eventTimer.restart();


    // =========================
    // AVVIO VISIONE
    // =========================

    visionTimer.start(
        50
        );
}


// ============================================================
// RESET TRACKING
// ============================================================

void BuffVisionManager::resetTracking()
{
    // =========================
    // STOP VISIONE
    // =========================

    visionTimer.stop();


    // =========================
    // RESET CORE
    // =========================

    core->reset();


    // =========================
    // RESET NUMERI
    // =========================

    lastCrop1Number = 1000;
    lastCrop2Number = 1000;


    // =========================
    // RESET CONTATORI
    // =========================

    visionCycle = 0;


    crop1EventTime = -1;
    crop2EventTime = -1;


    crop1EventCycle = -1;
    crop2EventCycle = -1;


    // =========================
    // RESET OVERLAY
    // =========================

    if(overlay)
    {
        overlay->resetOverlay();
    }


#ifdef QT_DEBUG

    if(debugWindow)
    {
        debugWindow->updateNumbers(
            1000,
            1000
            );

        debugWindow->setLastEvent(
            "---"
            );
    }

#endif


    // =========================
    // RIAVVIO
    // =========================

    if(enabled)
    {
        startTracking();
    }
}


// ============================================================
// CONFIGURE
// ============================================================

void BuffVisionManager::configure()
{
    if(!captureSetup)
    {
        captureSetup =
            new BuffVisionCaptureSetup(
                this->overlayRoot
                );


        this->overlayRoot->registerOverlay(
            captureSetup
            );
    }


    // =========================
    // RICARICA POSIZIONI
    // =========================

    captureSetup->loadSettings();


#ifdef QT_DEBUG

    // =========================
    // AGGIORNA RISOLUZIONE DEBUG
    // =========================

    if(debugWindow)
    {
        QScreen *screen =
            QGuiApplication::primaryScreen();


        if(screen)
        {
            const QSize resolution =
                screen->size();


            const QSize cropSize =
                BuffVisionConfig::cropSizeForScreen(
                    resolution
                    );


            debugWindow->setResolution(
                resolution.width(),
                resolution.height(),
                cropSize
                );
        }
    }

#endif


    // =========================
    // MOSTRA SETUP
    // =========================

    captureSetup->show();

    captureSetup->raise();

    captureSetup->activateWindow();

    captureSetup->setFocus();


    this->overlayRoot->raiseAll();
}


// ============================================================
// ENABLED
// ============================================================

void BuffVisionManager::setEnabled(
    bool enabled
    )
{
    this->enabled =
        enabled;


    // =========================
    // OFF
    // =========================

    if(!enabled)
    {
        // Stop immediato della visione.

        visionTimer.stop();


        // Reset core.

        core->reset();


        // Reset numeri.

        lastCrop1Number = 1000;
        lastCrop2Number = 1000;


        // Nascondi Atma.

        overlay->hide();


        overlay->resetOverlay();


#ifdef QT_DEBUG

        if(debugWindow)
        {
            debugWindow->hide();
        }

#endif


        return;
    }


    // =========================
    // ON
    // =========================

    overlay->show();


    this->overlayRoot->raiseAll();


#ifdef QT_DEBUG

    if(debugWindow)
    {
        debugWindow->show();
        debugWindow->raise();
    }

#endif


    startTracking();
}


// ============================================================
// DESTRUCTOR
// ============================================================

BuffVisionManager::~BuffVisionManager()
{
}