
#include "buffvisionmanager.h"

#include "globalkeyboard.h"
#include "buffvisioncore.h"
#include "buffvisionoverlay.h"
#include "buffvisioncapturesetup.h"
#include "overlayroot.h"

#include <QTimer>
#include <QFile>
#include <QDebug>


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
        qDebug()
        << "BUFFVISION: ERRORE - BuffVisionCapture non creato";
    }
    else
    {
        if(!capture->loadSettings())
        {
            qDebug()
            << "BUFFVISION: caricamento capture settings fallito";
        }
    }


    // =========================
    // DETECTOR
    // =========================

    detector =
        new BuffVisionDetector(
            this
            );


    detector->loadReferences();


    configured =
        detector->referencesLoaded();


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
                qDebug()
                << "BUFFVISION: frame acquisition failed";
            }


            // =========================
            // DETECTION
            // =========================

            VisionState state1 =
                detector->detectCrop1(
                    current1
                    );


            VisionState state2 =
                detector->detectCrop2(
                    current2
                    );


        // =========================
        // DEBUG
        // =========================

#ifdef QT_DEBUG

            debugWindow->setScores(
                detector->getCrop1State1Score(),
                detector->getCrop1State2Score(),
                detector->getCrop2State1Score(),
                detector->getCrop2State2Score()
                );

#endif


            // =========================
            // STATE CHANGE DEBUG
            // =========================

            if(
                state1 != VisionState::Unknown
                &&
                state1 != lastCrop1State
                )
            {
                /*
                 * DEBUG opzionale.
                 */
            }


            if(
                state2 != VisionState::Unknown
                &&
                state2 != lastCrop2State
                )
            {
                /*
                 * DEBUG opzionale.
                 */
            }


            // =========================
            // CROP 1 EVENT
            // =========================

            if(
                lastCrop1State ==
                    VisionState::State1
                &&
                state1 ==
                    VisionState::State2
                )
            {
                crop1EventTime =
                    eventTimer.elapsed();

                crop1EventCycle =
                    visionCycle;


                if(crop2EventTime >= 0)
                {
                    /*
                     * DEBUG opzionale.
                     */
                }


                core->onCrop1Event();
            }


            // =========================
            // CROP 2 EVENT
            // =========================

            if(
                lastCrop2State ==
                    VisionState::State1
                &&
                state2 ==
                    VisionState::State2
                )
            {
                crop2EventTime =
                    eventTimer.elapsed();

                crop2EventCycle =
                    visionCycle;


                if(crop1EventTime >= 0)
                {
                    /*
                     * DEBUG opzionale.
                     */
                }


                core->onCrop2Event();
            }


            // =========================
            // UPDATE PREVIOUS STATE
            // =========================

            if(
                state1 !=
                VisionState::Unknown
                )
            {
                lastCrop1State =
                    state1;
            }


            if(
                state2 !=
                VisionState::Unknown
                )
            {
                lastCrop2State =
                    state2;
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


            captureSetup->saveSettings();


            captureSetup->hide();


            capture->loadSettings();


            detector->loadReferences();


            configured =
                detector->referencesLoaded();


            referenceMode =
                CaptureReferenceMode::None;


            this->overlayRoot->raiseAll();
        }
        );


    // =========================
    // P
    // SALVATAGGIO REFERENCE
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {
            if(key != 'P')
            {
                return;
            }


            if(!captureSetup)
            {
                return;
            }


            if(!captureSetup->isVisible())
            {
                return;
            }


            saveCurrentReference();
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


            referenceMode =
                CaptureReferenceMode::None;


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


    /*
     * Reset iniziale del core.
     */

    core->reset();


    /*
     * Stato iniziale delle reference.
     */

    lastCrop1State =
        VisionState::Unknown;


    lastCrop2State =
        VisionState::Unknown;


    /*
     * Avvio tracking.
     */

    core->startTracking();


    /*
     * Reset contatori.
     */

    visionCycle =
        0;


    crop1EventTime =
        -1;

    crop2EventTime =
        -1;


    crop1EventCycle =
        -1;

    crop2EventCycle =
        -1;


    eventTimer.restart();


    /*
     * Avvio visione.
     */

    visionTimer.start(
        50
        );
}


// ============================================================
// RESET TRACKING
// ============================================================

void BuffVisionManager::resetTracking()
{
    /*
     * Fermiamo momentaneamente la visione.
     */

    visionTimer.stop();


    /*
     * Reset del core.
     */

    core->reset();


    /*
     * Reset degli stati.
     */

    lastCrop1State =
        VisionState::Unknown;


    lastCrop2State =
        VisionState::Unknown;


    /*
     * Reset contatori.
     */

    visionCycle =
        0;


    crop1EventTime =
        -1;

    crop2EventTime =
        -1;


    crop1EventCycle =
        -1;

    crop2EventCycle =
        -1;


    /*
     * Reset overlay.
     */

    if(overlay)
    {
        overlay->resetOverlay();
    }


    /*
     * Se Atma è ON,
     * il reset globale deve far ripartire
     * immediatamente il tracking.
     */

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


    /*
     * Configurazione parte sempre
     * dalla prima reference.
     */

    referenceMode =
        CaptureReferenceMode::Reference1;


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
        /*
         * Stop immediato della visione.
         */

        visionTimer.stop();


        /*
         * Reset del core.
         */

        core->reset();


        /*
         * Reset degli stati.
         */

        lastCrop1State =
            VisionState::Unknown;


        lastCrop2State =
            VisionState::Unknown;


        /*
         * Nascondi Atma.
         */

        overlay->hide();


        overlay->resetOverlay();


        return;
    }


    // =========================
    // ON
    // =========================

    /*
     * Mostra Atma.
     */

    overlay->show();


    this->overlayRoot->raiseAll();


    /*
     * Avvia immediatamente il tracking.
     */

    startTracking();
}


// ============================================================
// SAVE CURRENT REFERENCE
// ============================================================

void BuffVisionManager::saveCurrentReference()
{
    if(!captureSetup)
    {
        return;
    }


    // =========================
    // IMPOSTA LE CROP ATTUALI
    // =========================

    capture->setCropAreas(
        captureSetup->getCropRect1(),
        captureSetup->getCropRect2()
        );


    // =========================
    // NASCONDI COMPLETAMENTE
    // LA FINESTRA DI SETUP
    // =========================

    captureSetup->hide();


    QTimer::singleShot(
        150,
        this,
        [this]()
        {
            // =========================
            // REFERENCE 1
            // =========================

            if(
                referenceMode ==
                CaptureReferenceMode::Reference1
                )
            {
                capture->saveReference1();


                detector->loadReferences();


                referenceMode =
                    CaptureReferenceMode::Reference2;
            }


            // =========================
            // REFERENCE 2
            // =========================

            else if(
                referenceMode ==
                CaptureReferenceMode::Reference2
                )
            {
                capture->saveReference2();


                detector->loadReferences();


                configured =
                    detector->referencesLoaded();


                referenceMode =
                    CaptureReferenceMode::None;
            }


            // =========================
            // RIMOSTRA LA FINESTRA
            // =========================

            captureSetup->show();

            captureSetup->raise();

            captureSetup->activateWindow();

            captureSetup->setFocus();


            // =========================
            // FEEDBACK
            // =========================

            if(
                referenceMode ==
                CaptureReferenceMode::Reference2
                )
            {
                captureSetup->showFeedback(
                    "REFERENCE 1 SAVED"
                    );
            }
            else if(
                referenceMode ==
                CaptureReferenceMode::None
                )
            {
                captureSetup->showFeedback(
                    "REFERENCE 2 SAVED"
                    );
            }
        }
        );
}


// ============================================================
// DESTRUCTOR
// ============================================================

BuffVisionManager::~BuffVisionManager()
{
}


// ============================================================
// HAS REFERENCES
// ============================================================

bool BuffVisionManager::hasReferences() const
{
    QString basePath =
        QCoreApplication::applicationDirPath() +
        "/BuffVision";


    return
        QFile::exists(
            basePath + "/Crop1_Ref1.png"
            )
        &&
        QFile::exists(
            basePath + "/Crop1_Ref2.png"
            )
        &&
        QFile::exists(
            basePath + "/Crop2_Ref1.png"
            )
        &&
        QFile::exists(
            basePath + "/Crop2_Ref2.png"
            );
}
