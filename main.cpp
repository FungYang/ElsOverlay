#include <QApplication>
#include <QTimer>
#include <QProcess>
#include <QCoreApplication>

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


    // =========================
    // ROOT OVERLAY CONTAINER
    // =========================

    OverlayRoot *overlayRoot =
        new OverlayRoot();


    overlayRoot->show();


    // =========================
    // OGGETTI PRINCIPALI
    // =========================

    GlobalKeyboard keyboard;


    // =========================
    // FINESTRA CONFIGURAZIONE
    // =========================

    MainWindow mainWindow;

    ClassConfigurationManager classConfigManager;

    ClassBuffConfigWindow classBuffConfigWindow(
        &classConfigManager,
        &keyboard
        );



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
    // =========================
    // DISTANCE GUIDES - MOVIMENTO
    // =========================

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

                distanceGuideManager.setCharacterMoving(
                    true
                    );

                return;
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

    mainWindow.show();
    mainWindow.raise();
    mainWindow.activateWindow();


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


            const QList<ClassConfiguration>
                configurations =
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



    ClassSelector selector;



    // =========================
    // OVERLAY SKILL PRINCIPALE
    // =========================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::ctrlPressed,
        overlay,
        [overlay]()
        {
            overlay->startCooldown();
        }
        );



    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        overlay,
        [overlay]()
        {
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
                return;
            }
        }
        );



    // =========================
    // SISTEMA BUFF
    // =========================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        buffs,
        &BuffOverlay::handleKey
        );



    // =========================
    // SELETTORE CLASSI
    // =========================




    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        buffs,
        &BuffOverlay::resetAll
        );


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






    // =========================
    // BUFF VISION ATMA
    // =========================

    BuffVisionManager atma(
        &keyboard,
        overlayRoot
        );

    // =========================
    // ATMA - CONFIGURAZIONE
    // =========================

    QObject::connect(
        &mainWindow,
        &MainWindow::atmaConfigRequested,
        &atma,
        &BuffVisionManager::configure
        );


    // =========================
    // ATMA - TOGGLE
    // =========================

    QObject::connect(
        &mainWindow,
        &MainWindow::atmaToggled,
        &atma,
        &BuffVisionManager::setEnabled
        );

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



    return app.exec();
}