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
                return;


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
                // qDebug()
                // << "CROP1 STATE CHANGE"
                // << static_cast<int>(
                //        lastCrop1State
                //        )
                // << "->"
                // << static_cast<int>(
                //        state1
                //        )
                // << "| cycle ="
                // << visionCycle
                // << "| t ="
                // << eventTimer.elapsed()
                // << "ms";
            }


            if(
                state2 != VisionState::Unknown
                &&
                state2 != lastCrop2State
                )
            {
                // qDebug()
                // << "CROP2 STATE CHANGE"
                // << static_cast<int>(
                //        lastCrop2State
                //        )
                // << "->"
                // << static_cast<int>(
                //        state2
                //        )
                // << "| cycle ="
                // << visionCycle
                // << "| t ="
                // << eventTimer.elapsed()
                // << "ms";
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


                // qDebug()
                //     << "CROP1 EVENT FIRED"
                //     << "| cycle ="
                //     << crop1EventCycle
                //     << "| t ="
                //     << crop1EventTime
                //     << "ms";


                if(crop2EventTime >= 0)
                {
                    // qDebug()
                    // << ">>> DISTANCE CROP1-CROP2 ="
                    // << qAbs(
                    //        crop1EventTime -
                    //        crop2EventTime
                    //        )
                    // << "ms"
                    // << "| cycles ="
                    // << qAbs(
                    //        crop1EventCycle -
                    //        crop2EventCycle
                    //        );
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


                // qDebug()
                //     << "CROP2 EVENT FIRED"
                //     << "| cycle ="
                //     << crop2EventCycle
                //     << "| t ="
                //     << crop2EventTime
                //     << "ms";


                if(crop1EventTime >= 0)
                {
                    // qDebug()
                    // << ">>> DISTANCE CROP1-CROP2 ="
                    // << qAbs(
                    //        crop1EventTime -
                    //        crop2EventTime
                    //        )
                    // << "ms"
                    // << "| cycles ="
                    // << qAbs(
                    //        crop1EventCycle -
                    //        crop2EventCycle
                    //        );
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
               /* qDebug() << "TRACKING STARTED";  */ // <-- aggiungi temporaneamente


                visionCycle = 0;

                crop1EventTime = -1;
                crop2EventTime = -1;

                crop1EventCycle = -1;
                crop2EventCycle = -1;

                eventTimer.restart();

                // qDebug() << "================================";
                // qDebug() << "TRACKING STARTED - EVENT TIMER";
                // qDebug() << "================================";
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
    // NASCONDI COMPLETAMENTE
    // LA FINESTRA DI SETUP
    // =========================
    //
    // Nascondere solo i rettangoli disegnati
    // (setCaptureMode) non basta: il compositor
    // potrebbe non aver ancora ridisegnato la
    // finestra come trasparente nel momento
    // esatto della cattura. hide() garantisce
    // che la finestra non sia nella cattura.

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


            // Feedback dopo aver rimostrato
            // la finestra, cosi' e' visibile.

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

BuffVisionManager::~BuffVisionManager()
{


}

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