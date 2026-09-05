#include <QApplication>
#include <QCoreApplication>
#include <QProcess>

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


    int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

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
    // MAIN WINDOW
    // ==================================================

    MainWindow mainWindow;


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
    // SHOW MAIN WINDOW
    // ==================================================

    mainWindow.show();
    mainWindow.raise();
    mainWindow.activateWindow();


    // ==================================================
    // TRANSCENDENCE OVERLAY
    // ==================================================

    Overlay *overlay =
        new Overlay(
            overlayRoot
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
                return;

            if(enabled)
                overlay->show();
            else
                overlay->hide();
        }
        );


    // SPECIAL COOLDOWN


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

    // qDebug()
    //     << "MAIN: prima TranscendenceVisionManager";


    TranscendenceVisionManager transcendenceVision(
        &keyboard,
        overlayRoot,
        overlay
        );


    // qDebug()
    //     << "MAIN: dopo TranscendenceVisionManager";


    // qDebug() << "MAIN: prima connect transcendenceConfig";
    QObject::connect(
        &mainWindow,
        &MainWindow::transcendenceConfigRequested,
        &transcendenceVision,
        &TranscendenceVisionManager::configure
        );
    // qDebug() << "MAIN: dopo connect transcendenceConfig";

    // qDebug() << "MAIN: prima connect transcendenceToggle";
    QObject::connect(
        &mainWindow,
        &MainWindow::buffTranscendenceToggled,
        &transcendenceVision,
        &TranscendenceVisionManager::setEnabled
        );
    // qDebug() << "MAIN: dopo connect transcendenceToggle";




    // // ==================================================
    // // SKILL OVERLAY
    // // ==================================================
    // qDebug() << "MAIN: prima SkillOverlay";

    SkillOverlay *skills =
        new SkillOverlay(
            &keyboard,
            overlayRoot
            );
    // qDebug() << "MAIN: dopo SkillOverlay";
    SkillConfigWindow *skillConfigWindow =
        new SkillConfigWindow(&mainWindow);

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
    // qDebug() << "MAIN: prima BuffOverlay";

    BuffOverlay *buffs =
        new BuffOverlay(
            overlayRoot
            );
    // qDebug() << "MAIN: dopo BuffOverlay";


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
                return;


            const QList<ClassConfiguration> configurations =
                classConfigManager.configurations();


            for(const ClassConfiguration &configuration :
                 configurations)
            {
                if(configuration.id != activeId)
                    continue;


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
            if (overlay->startCooldown())
                transcendenceVision.onCooldownStarted();
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
            // qDebug()
            // << "RESET ARRIVATO ALL'OVERLAY";

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
        [overlay,&transcendenceVision](int key)
        {
            if(key == '6')
            {
                overlay->startCooldown();
                transcendenceVision.onCooldownStarted();
            }


            if(key == '8')
            {
                overlay->togglePause();
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

    //qDebug() << "MAIN: prima BuffVisionManager";
    BuffVisionManager atma(
        &keyboard,
        overlayRoot
        );
    //qDebug() << "MAIN: dopo BuffVisionManager";


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
    // APPLICATION LOOP
    // ==================================================

    return app.exec();
}