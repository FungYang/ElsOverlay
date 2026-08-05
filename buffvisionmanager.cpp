#include "buffvisionmanager.h"

#include "globalkeyboard.h"
#include "buffvisioncore.h"
#include "buffvisionoverlay.h"
#include "buffvisionselector.h"
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
    keyboard(keyboard),
    overlayRoot(overlayRoot)
{

    core =
        new BuffVisionCore(this);



    capture =
        new BuffVisionCapture(this);


    capture->loadSettings();



    detector =
        new BuffVisionDetector(this);



#ifdef QT_DEBUG

    debugWindow =
        new BuffVisionDebug(
            this->overlayRoot
            );


    this->overlayRoot->registerOverlay(
        debugWindow
        );


    debugWindow->show();

#endif



    detector->loadReferences();


    configured =
        detector->referencesLoaded();



    overlay =
        new BuffVisionOverlay(
            core,
            this->overlayRoot
            );


    this->overlayRoot->registerOverlay(
        overlay
        );



    selector =
        new BuffVisionSelector(
            this->overlayRoot
            );


    this->overlayRoot->registerOverlay(
        selector
        );



    selector->show();

    this->overlayRoot->raiseAll();



    captureSetup = nullptr;



    connect(
        &visionTimer,
        &QTimer::timeout,
        this,
        [this]()
        {

            if(!enabled)
                return;


            if(!configured)
                return;



            QPixmap current1 =
                capture->captureCrop1();


            QPixmap current2 =
                capture->captureCrop2();



            VisionState state1 =
                detector->detectCrop1(
                    current1
                    );


            VisionState state2 =
                detector->detectCrop2(
                    current2
                    );



#ifdef QT_DEBUG

            debugWindow->setScores(
                detector->getCrop1State1Score(),
                detector->getCrop1State2Score(),
                detector->getCrop2State1Score(),
                detector->getCrop2State2Score()
                );

#endif



            if(lastCrop1State == VisionState::State1 &&
                state1 == VisionState::State2)
            {
                core->onCrop1Event();
            }



            if(lastCrop2State == VisionState::State1 &&
                state2 == VisionState::State2)
            {
                core->onCrop2Event();
            }



            if(state1 != VisionState::Unknown)
            {
                lastCrop1State = state1;
            }



            if(state2 != VisionState::Unknown)
            {
                lastCrop2State = state2;
            }

        }
        );


    // =========================
    // START TRACKING + AZIONI
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {

            // =========================
            // SALVATAGGIO REFERENCE CON P
            // =========================

            if(key == 'P')
            {

                if(!captureSetup)
                    return;



                captureSetup->setCaptureMode(
                    true
                    );



                QTimer::singleShot(
                    100,
                    this,
                    [this]()
                    {

                        capture->setCropAreas(
                            captureSetup->getCropRect1(),
                            captureSetup->getCropRect2()
                            );



                        if(referenceMode ==
                            CaptureReferenceMode::Reference1)
                        {

                            capture->saveReference1();


                            detector->loadReferences();


                            captureSetup->showFeedback(
                                "REFERENCE 1 SAVED"
                                );


                            referenceMode =
                                CaptureReferenceMode::Reference2;

                        }
                        else
                            if(referenceMode ==
                                CaptureReferenceMode::Reference2)
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



                        captureSetup->setCaptureMode(
                            false
                            );


                    }
                    );


                return;

            }



            // =========================
            // DA QUI IN POI SOLO GAMEPLAY
            // =========================


            if(!enabled)
                return;



            if(key == '0')
            {

                core->reset();


                lastCrop1State =
                    VisionState::Unknown;


                lastCrop2State =
                    VisionState::Unknown;


                core->startTracking();


                if(configured)
                {
                    visionTimer.start(50);
                }


                return;

            }



            if(key >= '1' &&
                key <= '6')
            {

                core->registerAction();

            }


        }
        );


    connect(
        selector,
        &BuffVisionSelector::enabled,
        this,
        [this]()
        {

            enabled = true;


            referenceMode =
                CaptureReferenceMode::Reference1;


            configurationConfirmed = false;



            visionTimer.start(50);



            selector->hide();



            overlay->show();



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



            captureSetup->show();



            this->overlayRoot->raiseAll();

        }
        );







    connect(
        selector,
        &BuffVisionSelector::disabled,
        this,
        [this]()
        {

            enabled = false;


            selector->hide();


            overlay->hide();


            core->reset();


            overlay->resetOverlay();

        }
        );







    connect(
        keyboard,
        &GlobalKeyboard::confirmPressed,
        this,
        [this]()
        {

            if(!enabled)
                return;


            if(!captureSetup)
                return;


            if(configurationConfirmed)
                return;



            captureSetup->saveSettings();


            captureSetup->hide();



            capture->loadSettings();


            detector->loadReferences();



            configured =
                detector->referencesLoaded();



            configurationConfirmed = true;



            overlay->show();


            this->overlayRoot->raiseAll();

        }
        );







    connect(
        keyboard,
        &GlobalKeyboard::resetPressed,
        this,
        [this]()
        {

            enabled = false;


            configurationConfirmed = false;



            core->reset();



            overlay->resetOverlay();


            overlay->hide();



            if(captureSetup)
            {

                captureSetup->resetSetup();

                captureSetup->hide();

            }



            selector->resetSelector();


            selector->show();



            visionTimer.stop();



            this->overlayRoot->raiseAll();

        }
        );

}



BuffVisionManager::~BuffVisionManager()
{

    delete overlay;

    delete selector;

    delete captureSetup;

}




bool BuffVisionManager::hasReferences() const
{

    return QFile::exists(
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