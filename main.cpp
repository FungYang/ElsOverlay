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

#include "buffvisionmanager.h"
#include "overlayroot.h"
#include "classconfigurationmanager.h"
#include "classbuffconfigwindow.h"

#include "distanceguidemanager.h"
#include "distanceguideconfigwindow.h"
#include "distanceguideoverlay.h"


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
        }
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
        }
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


    QObject::connect(
        &mainWindow,
        &MainWindow::buffTranscendenceToggled,
        overlay,
        [overlay](bool enabled)
        {
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
    // SKILL OVERLAY
    // ==================================================

    SkillOverlay *skills =
        new SkillOverlay(
            &keyboard,
            overlayRoot
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
            overlayRoot
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
        [overlay]()
        {
            overlay->startCooldown();
        }
        );


    // ==================================================
    // GLOBAL RESET
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        overlay,
        [overlay]()
        {
            qDebug()
            << "RESET ARRIVATO ALL'OVERLAY";

            overlay->resetCooldown();
        }
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::transcendenceResetPressed,
        overlay,
        [overlay]()
        {
            overlay->resetCooldown();
        }
        );


    // ==================================================
    // KEYBOARD SHORTCUTS
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        overlay,
        [overlay](int key)
        {
            if(key == '6')
            {
                overlay->startCooldown();
            }


            if(key == '8')
            {
                overlay->togglePause();
            }
        }
        );


    // ==================================================
    // BUFF SYSTEM
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        buffs,
        &BuffOverlay::handleKey
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        buffs,
        &BuffOverlay::resetAll
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