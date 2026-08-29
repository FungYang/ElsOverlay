#include "buffvisionmanager.h"

#include "globalkeyboard.h"
#include "buffvisioncore.h"
#include "buffvisionoverlay.h"
#include "buffvisioncapturesetup.h"
#include "overlayroot.h"

#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QElapsedTimer>


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


    capture->loadSettings();

    capture->initDuplication();



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


    // IMPORTANTE:
    // l'overlay parte nascosto.
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

            // Atma disabilitato:
            // nessuna elaborazione.
            if(!enabled)
            {
                visionTimer.stop();
                return;
            }


            // Nessuna reference:
            // nessuna elaborazione.
            if(!configured)
                return;

            QElapsedTimer perfTimer;
            perfTimer.start();

            // =========================
            // CAPTURE
            // =========================

            QPixmap current1 =
                capture->captureCrop1();


            QPixmap current2 =
                capture->captureCrop2();

            qint64 captureNs = perfTimer.nsecsElapsed();



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
            qint64 totalNs = perfTimer.nsecsElapsed();
            qint64 detectNs = totalNs - captureNs;





#ifdef QT_DEBUG
            qDebug() << "VISION CYCLE:"
                     << "CAPTURE =" << captureNs / 1000000.0 << "ms"
                     << "DETECT ="  << detectNs  / 1000000.0 << "ms"
                     << "TOTAL ="   << totalNs   / 1000000.0 << "ms"
                     << "(timer interval = 50 ms)";

            debugWindow->setScores(
                detector->getCrop1State1Score(),
                detector->getCrop1State2Score(),
                detector->getCrop2State1Score(),
                detector->getCrop2State2Score()
                );

#endif



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

            // =================================
            // START TRACKING
            // =================================

            if(key == '0')
            {

                // Atma deve essere ON.
                if(!enabled)
                    return;


                // Reference necessarie.
                if(!configured)
                    return;



                core->reset();


                lastCrop1State =
                    VisionState::Unknown;


                lastCrop2State =
                    VisionState::Unknown;



                core->startTracking();



                visionTimer.start(
                    50
                    );


                return;

            }



            // =================================
            // ACTIONS
            // =================================

            if(
                key >= '1' &&
                key <= '6'
                )
            {

                if(!enabled)
                    return;


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
                return;


            // Se non siamo in configurazione
            // non facciamo nulla.
            if(!captureSetup->isVisible())
                return;



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
                return;


            if(!captureSetup)
                return;


            if(!captureSetup->isVisible())
                return;



            saveCurrentReference();

        }
        );



    // =========================
    // RESET
    // =========================

    // =========================
    // RESET
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::resetPressed,
        this,
        [this]()
        {
            visionTimer.stop();

            core->reset();

            lastCrop1State =
                VisionState::Unknown;

            lastCrop2State =
                VisionState::Unknown;

            if(overlay)
            {
                overlay->resetOverlay();
            }

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



    // Configurazione parte sempre
    // dalla prima reference.

    referenceMode =
        CaptureReferenceMode::Reference1;



    captureSetup->show();


    captureSetup->raise();

    captureSetup->activateWindow();
    captureSetup->setFocus();


    this->overlayRoot->raiseAll();

}

void BuffVisionManager::setEnabled(
    bool enabled
    )
{

    this->enabled =
        enabled;



    if(!enabled)
    {

        // STOP IMMEDIATO
        visionTimer.stop();



        // STOP CORE
        core->reset();



        lastCrop1State =
            VisionState::Unknown;


        lastCrop2State =
            VisionState::Unknown;



        // NASCONDE ATMA
        overlay->hide();


        overlay->resetOverlay();



        return;

    }



    // ON:
    // mostriamo soltanto l'overlay.
    // NON parte il tracking.

    overlay->show();


    this->overlayRoot->raiseAll();

}

void BuffVisionManager::saveCurrentReference()
{
    if(!captureSetup)
        return;


    // =========================
    // IMPOSTA LE CROP ATTUALI
    // =========================

    capture->setCropAreas(
        captureSetup->getCropRect1(),
        captureSetup->getCropRect2()
        );


    // =========================
    // NASCONDI I RETTANGOLI
    // =========================

    captureSetup->setCaptureMode(true);


    // Diamo tempo a Qt di ridisegnare
    // la finestra senza i rettangoli.

    QTimer::singleShot(
        100,
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


                captureSetup->showFeedback(
                    "REFERENCE 1 SAVED"
                    );


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


                captureSetup->showFeedback(
                    "REFERENCE 2 SAVED"
                    );


                referenceMode =
                    CaptureReferenceMode::None;

            }


            // =========================
            // RIATTIVA RETTANGOLI
            // =========================

            captureSetup->setCaptureMode(false);

        }
        );
}

BuffVisionManager::~BuffVisionManager()
{

    delete overlay;

    delete captureSetup;

}

bool BuffVisionManager::hasReferences() const
{

    return
        QFile::exists(
            "BuffVision/Crop1_Ref1.png"
            )
        &&
        QFile::exists(
            "BuffVision/Crop1_Ref2.png"
            )
        &&
        QFile::exists(
            "BuffVision/Crop2_Ref1.png"
            )
        &&
        QFile::exists(
            "BuffVision/Crop2_Ref2.png"
            );

}