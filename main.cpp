#include <QApplication>
#include <QCoreApplication>
#include <QProcess>
#include <QThreadPool>
#include <QThread>

#include <windows.h>

#include "mainwindow.h"

#include "overlay.h"
#include "globalkeyboard.h"
#include "skilloverlay.h"
#include "buffoverlay.h"
#include "classselector.h"
#include "transcendencevisionmanager.h"

#include "buffvisionmanager.h"
#include "overlayroot.h"
#include "classconfigurationmanager.h"
#include "classbuffconfigwindow.h"

#include "distanceguidemanager.h"
#include "distanceguideconfigwindow.h"
#include "distanceguideoverlay.h"
#include "skillconfigwindow.h"
#include "specialcooldownmanager.h"
#include "specialcooldownconfigwindow.h"
#include "specialcooldownoverlay.h"
#include "atmazonemanager.h"
#include "buffvisionoverlay.h"
#include "buffvisioncore.h"
#include "resonancegatemanager.h"


    int main(
        int argc,
        char *argv[]
        )
{
    QApplication app(
        argc,
        argv
        );
    // ==================================================
    // THREAD POOL CAP
    // ==================================================
    //
    // QtConcurrent::run() (usato da TranscendenceVisionManager
    // per findIcon()) pesca da questo pool globale condiviso.
    // Limitiamo quanti core può occupare, per lasciare margine
    // al resto del sistema e ridurre il carico/calore sostenuto.

    QThreadPool::globalInstance()->setMaxThreadCount(
        qMax(2, QThread::idealThreadCount() / 2)
        );




    // ==================================================
    // QSETTINGS
    // ==================================================

    QCoreApplication::setOrganizationName(
        "ElsOverlay"
        );


    QCoreApplication::setApplicationName(
        "ElsOverlay"
        );


    // ==================================================
    // ROOT OVERLAY
    // ==================================================

    OverlayRoot *overlayRoot =
        new OverlayRoot();


    overlayRoot->show();


    // ==================================================
    // GLOBAL KEYBOARD
    // ==================================================

    GlobalKeyboard keyboard;


    // ==================================================
    // HIDE / SHOW OVERLAY
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        overlayRoot,
        [overlayRoot](int key)
        {
            if(key == VK_DELETE)
            {
                overlayRoot->toggleVisibility();
            }
        },
        Qt::QueuedConnection
        );


    // ==================================================
    // MAIN WINDOW
    // ==================================================

    MainWindow mainWindow;


    // ==================================================
    // OVERLAY CLICKABILITY
    // ==================================================

    QObject::connect(
        &mainWindow,
        &MainWindow::overlayClickabilityToggled,
        overlayRoot,
        &OverlayRoot::setClickable
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::transparencyChanged,
        overlayRoot,
        &OverlayRoot::setTransparency
        );

    // ==================================================
    // CLASS BUFF CONFIGURATION
    // ==================================================

    ClassConfigurationManager classConfigManager;


    ClassBuffConfigWindow classBuffConfigWindow(
        &classConfigManager,
        &keyboard
        );


    // ==================================================
    // KEY CONFIGURATION
    // ==================================================

    QObject::connect(
        &mainWindow,
        &MainWindow::pauseKeyChanged,
        &keyboard,
        &GlobalKeyboard::setPauseKey
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::resetKeyChanged,
        &keyboard,
        &GlobalKeyboard::setResetKey
        );


    // ==================================================
    // DISTANCE GUIDES
    // ==================================================

    DistanceGuideManager distanceGuideManager;


    DistanceGuideConfigWindow distanceGuideConfigWindow(
        &distanceGuideManager,
        &keyboard
        );


    DistanceGuideOverlay *distanceGuides =
        new DistanceGuideOverlay(
            &distanceGuideManager,
            overlayRoot
            );


    overlayRoot->registerOverlay(
        distanceGuides
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::distanceGuidesConfigRequested,
        &distanceGuideConfigWindow,
        [&distanceGuideConfigWindow]()
        {
            distanceGuideConfigWindow.refresh();

            distanceGuideConfigWindow.show();

            distanceGuideConfigWindow.raise();

            distanceGuideConfigWindow.activateWindow();
        }
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::distanceGuidesToggled,
        distanceGuides,
        &DistanceGuideOverlay::setEnabled
        );


    // ==================================================
    // DISTANCE GUIDES - MOVEMENT
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        &distanceGuideManager,
        [&distanceGuideManager](int key)
        {
            if(key == VK_LEFT)
            {
                distanceGuideManager.setMovementDirection(
                    MovementDirection::Left
                    );


                distanceGuideManager.setCharacterFacing(
                    CharacterFacing::Left
                    );


                distanceGuideManager.setCharacterMoving(
                    true
                    );


                return;
            }


            if(key == VK_RIGHT)
            {
                distanceGuideManager.setMovementDirection(
                    MovementDirection::Right
                    );


                distanceGuideManager.setCharacterFacing(
                    CharacterFacing::Right
                    );


                distanceGuideManager.setCharacterMoving(
                    true
                    );
            }
        },
        Qt::QueuedConnection
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyReleased,
        &distanceGuideManager,
        [&distanceGuideManager](int key)
        {
            if(key == VK_LEFT ||
                key == VK_RIGHT)
            {
                distanceGuideManager.setCharacterMoving(
                    false
                    );
            }
        },
        Qt::QueuedConnection
        );


    // ==================================================
    // TRANSCENDENCE OVERLAY
    // ==================================================

    Overlay *overlay =
        new Overlay(
            overlayRoot
            );


    overlayRoot->registerOverlay(
        overlay
        );


    if(!overlayRoot)
    {
        qDebug()
        << "MAIN: ERRORE - overlayRoot nullo";


        return -1;
    }


    if(!overlay)
    {
        qDebug()
        << "MAIN: ERRORE - Overlay non creato";


        return -1;
    }


    QObject::connect(
        &mainWindow,
        &MainWindow::buffTranscendenceToggled,
        overlay,
        [overlay](bool enabled)
        {
            if(!overlay)
            {
                return;
            }


            if(enabled)
            {
                overlay->show();
            }
            else
            {
                overlay->hide();
            }
        }
        );


    // ==================================================
    // SPECIAL COOLDOWN
    // ==================================================

    SpecialCooldownManager specialCooldownManager;


    specialCooldownManager.load();


    SpecialCooldownConfigWindow specialCooldownConfigWindow(
        &specialCooldownManager
        );


    SpecialCooldownOverlay *specialCooldowns =
        new SpecialCooldownOverlay(
            &specialCooldownManager,
            &keyboard,
            overlayRoot
            );

    overlayRoot->setSpecialCooldownOverlay(
        specialCooldowns
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::specialCooldownConfigRequested,
        &specialCooldownConfigWindow,
        [&specialCooldownConfigWindow]()
        {
            specialCooldownConfigWindow.show();

            specialCooldownConfigWindow.raise();

            specialCooldownConfigWindow.activateWindow();
        }
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::specialCooldownsToggled,
        specialCooldowns,
        &SpecialCooldownOverlay::setEnabled
        );


    // ==================================================
    // TRANSCENDENCE VISION
    // ==================================================

    TranscendenceVisionManager transcendenceVision(
        &keyboard,
        overlayRoot,
        overlay
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::transcendenceConfigRequested,
        &transcendenceVision,
        &TranscendenceVisionManager::configure
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::buffTranscendenceToggled,
        &transcendenceVision,
        &TranscendenceVisionManager::setEnabled
        );


    // ==================================================
    // SKILL OVERLAY
    // ==================================================

    SkillOverlay *skills =
        new SkillOverlay(
            &keyboard,
            overlayRoot
            );

    overlayRoot->setSkillOverlay(
        skills
        );


    overlayRoot->registerOverlay(
        skills
        );


    SkillConfigWindow *skillConfigWindow =
        new SkillConfigWindow(
            &mainWindow
            );


    QObject::connect(
        &mainWindow,
        &MainWindow::buffTitlesConfigRequested,
        skillConfigWindow,
        [skillConfigWindow]()
        {
            skillConfigWindow->show();

            skillConfigWindow->raise();

            skillConfigWindow->activateWindow();
        }
        );


    QObject::connect(
        skillConfigWindow,
        &SkillConfigWindow::configurationChanged,
        skills,
        &SkillOverlay::applyConfig
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::buffTitlesToggled,
        skills,
        [skills](bool enabled)
        {
            if(enabled)
            {
                skills->show();
            }
            else
            {
                skills->hide();
            }
        }
        );


    // ==================================================
    // CLASS BUFF OVERLAY
    // ==================================================

    BuffOverlay *buffs =
        new BuffOverlay(
            overlayRoot,
            overlayRoot
            );


    overlayRoot->registerOverlay(
        buffs
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::classBuffToggled,
        buffs,
        [&](bool enabled)
        {
            if(!enabled)
            {
                buffs->clearBuffs();

                return;
            }


            const QString activeId =
                classConfigManager.activeConfigurationId();


            if(activeId.isEmpty())
            {
                return;
            }


            const QList<ClassConfiguration> configurations =
                classConfigManager.configurations();


            for(const ClassConfiguration &configuration :
                 configurations)
            {
                if(configuration.id != activeId)
                {
                    continue;
                }


                buffs->loadConfiguration(
                    configuration
                    );


                return;
            }
        }
        );


    // ==================================================
    // CLASS SELECTOR
    // ==================================================

    ClassSelector selector;


    // ==================================================
    // MAIN SKILL OVERLAY
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::ctrlPressed,
        overlay,
        [overlay,&transcendenceVision]()
        {
            if(overlay->startCooldown())
            {
                transcendenceVision.onCooldownStarted();
            }
        },
        Qt::QueuedConnection
        );


    // ==================================================
    // GLOBAL RESET
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        overlay,
        [overlay,&transcendenceVision]()
        {
            overlay->resetCooldown();

            transcendenceVision.onCooldownReset();
        },
        Qt::QueuedConnection
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::transcendenceResetPressed,
        overlay,
        [overlay,&transcendenceVision]()
        {
            overlay->resetCooldown();

            transcendenceVision.onCooldownReset();
        },
        Qt::QueuedConnection
        );


    // ==================================================
    // KEYBOARD SHORTCUTS
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        overlay,
        [overlay, &transcendenceVision, skills](int key)
        {
            if(key == skills->cipollaKey())
            {
                overlay->startCooldown();

                transcendenceVision.onCooldownStarted();
            }
        },
        Qt::QueuedConnection
        );


    // ==================================================
    // BUFF SYSTEM
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        buffs,
        &BuffOverlay::handleKey,
        Qt::QueuedConnection
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        buffs,
        &BuffOverlay::resetAll,
        Qt::QueuedConnection
        );


    // ==================================================
    // CLASS BUFF CONFIGURATION WINDOW
    // ==================================================

    QObject::connect(
        &mainWindow,
        &MainWindow::classBuffConfigRequested,
        &classBuffConfigWindow,
        [&classBuffConfigWindow]()
        {
            classBuffConfigWindow.refresh();

            classBuffConfigWindow.show();

            classBuffConfigWindow.raise();

            classBuffConfigWindow.activateWindow();
        }
        );


    // ==================================================
    // ATMA BUFF VISION
    // ==================================================

    BuffVisionManager atma(
        &keyboard,
        overlayRoot
        );

    overlayRoot->setBuffVisionOverlay(
        atma.visionOverlay()
        );
    overlayRoot->setTransparency(
        mainWindow.transparencyValue()
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::atmaConfigRequested,
        &atma,
        &BuffVisionManager::configure
        );


    QObject::connect(
        &mainWindow,
        &MainWindow::atmaToggled,
        &atma,
        &BuffVisionManager::setEnabled
        );

    // ==================================================
    // ATMA ZONES (nuovo sistema a 7 regioni)
    // ==================================================

    AtmaZoneManager atmaZones(
        &keyboard,
        overlayRoot,
        atma.visionCore(), // vedi nota sotto: serve un getter pubblico
        nullptr
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::atmaZonesConfigRequested, // NUOVO segnale, vedi sotto
        &atmaZones,
        &AtmaZoneManager::configure
        );

    // Stesso toggle "Atma" attiva/disattiva ENTRAMBI i sistemi insieme.
    QObject::connect(
        &mainWindow,
        &MainWindow::atmaToggled,
        &atmaZones,
        &AtmaZoneManager::setEnabled
        );

    // ==================================================
    // ATMA GATE — pausa globale su invariante
    // ==================================================

    // ==================================================
    // RESONANCE GATE
    // ==================================================

    ResonanceGateManager resonanceGate(
        &keyboard,
        overlayRoot
        );
    QObject::connect(
        &mainWindow,
        &MainWindow::resonanceGateConfigRequested,
        &resonanceGate,
        &ResonanceGateManager::configure
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::resonanceGateToggled,
        &resonanceGate,
        &ResonanceGateManager::setEnabled
        );
#ifdef QT_DEBUG
    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::debugFrame,
        &atmaZones,
        &AtmaZoneManager::updateBlueDebug
        );
#endif

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        atma.visionCore(),
        &BuffVisionCore::pauseAtmaCooldowns
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        atma.visionCore(),
        &BuffVisionCore::resumeAtmaCooldowns
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        atma.visionOverlay(),
        &BuffVisionOverlay::pauseAtmaCooldown
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        atma.visionOverlay(),
        &BuffVisionOverlay::resumeAtmaCooldown
        );
    // Resonance Gate -> Atma Zones
    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        &atmaZones,
        [&atmaZones]()
        {
            atmaZones.setGateOpen(false);
        }
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        &atmaZones,
        [&atmaZones]()
        {
            atmaZones.setGateOpen(true);
        }
        );
    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        overlay,
        &Overlay::pauseAtmaGate
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        overlay,
        &Overlay::resumeAtmaGate
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        skills,
        &SkillOverlay::pauseAtmaGate
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        skills,
        &SkillOverlay::resumeAtmaGate
        );
    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        &transcendenceVision,
        &TranscendenceVisionManager::pauseAtmaGate
        );

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        &transcendenceVision,
        &TranscendenceVisionManager::resumeAtmaGate
        );

    // QObject::connect(&atmaZones, &AtmaZoneManager::invariantEntered,
    //                  specialCooldowns, &SpecialCooldownOverlay::pauseAll);
    // QObject::connect(&atmaZones, &AtmaZoneManager::invariantExited,
    //                  specialCooldowns, &SpecialCooldownOverlay::resumeAll);

    // QObject::connect(&atmaZones, &AtmaZoneManager::invariantEntered,
    //                  buffs, &BuffOverlay::pauseAll);
    // QObject::connect(&atmaZones, &AtmaZoneManager::invariantExited,
    //                  buffs, &BuffOverlay::resumeAll);



    // ==================================================
    // BUFF TRACKER
    // ==================================================

    QObject::connect(
        &mainWindow,
        &MainWindow::buffTrackerConfigRequested,
        []()
        {
            const QString path =
                QCoreApplication::applicationDirPath() +
                "/buffoverla.exe";


            QProcess::startDetached(
                path
                );
        }
        );


    // ==================================================
    // RESTORE TOGGLE STATES
    // ==================================================

    mainWindow.loadToggleStates();


    // ==================================================
    // SHOW MAIN WINDOW
    // ==================================================

    mainWindow.show();


    mainWindow.raise();


    mainWindow.activateWindow();


    // ==================================================
    // APPLICATION LOOP
    // ==================================================

    return app.exec();
}
