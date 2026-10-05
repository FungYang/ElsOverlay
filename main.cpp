#include <QApplication>
#include <QCoreApplication>
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
#include "capturecoordinator.h"
#include "customsearchermanager.h"

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


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ==================================================
    // THREAD POOL CAP
    // ==================================================

    QThreadPool::globalInstance()->setMaxThreadCount(
        qMax(2, QThread::idealThreadCount() / 2)
        );

    // ==================================================
    // QSETTINGS
    // ==================================================

    QCoreApplication::setOrganizationName("ElsOverlay");
    QCoreApplication::setApplicationName("ElsOverlay");

    // ==================================================
    // ROOT OVERLAY
    // ==================================================

    OverlayRoot *overlayRoot = new OverlayRoot();

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

    overlayRoot->registerOverlay(distanceGuides);

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

                distanceGuideManager.setCharacterMoving(true);

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

                distanceGuideManager.setCharacterMoving(true);
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
            if(key == VK_LEFT || key == VK_RIGHT)
            {
                distanceGuideManager.setCharacterMoving(false);
            }
        },
        Qt::QueuedConnection
        );

    // ==================================================
    // TRANSCENDENCE OVERLAY
    // ==================================================

    Overlay *overlay = new Overlay(overlayRoot);

    overlayRoot->registerOverlay(overlay);

    if(!overlayRoot)
    {
        qDebug() << "MAIN: ERRORE - overlayRoot nullo";
        return -1;
    }

    if(!overlay)
    {
        qDebug() << "MAIN: ERRORE - Overlay non creato";
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

    overlayRoot->setSpecialCooldownOverlay(specialCooldowns);

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
    // CUSTOM SEARCHER
    // ==================================================

    CustomSearcherManager customSearcher(
        CaptureCoordinator::instance(),
        overlayRoot
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::customSearcherConfigRequested,
        &customSearcher,
        &CustomSearcherManager::configure
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::customSearcherToggled,
        &customSearcher,
        &CustomSearcherManager::setEnabled
        );

    // ==================================================
    // CUSTOM SEARCHER - GLOBAL RESET
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        &customSearcher,
        &CustomSearcherManager::reset,
        Qt::QueuedConnection
        );

    // ==================================================
    // SKILL OVERLAY
    // ==================================================

    SkillOverlay *skills =
        new SkillOverlay(
            &keyboard,
            overlayRoot
            );

    overlayRoot->setSkillOverlay(skills);

    overlayRoot->registerOverlay(skills);

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

    BuffOverlay *buffs =
        new BuffOverlay(
            overlayRoot,
            overlayRoot
            );

    overlayRoot->registerOverlay(buffs);

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

            for(const ClassConfiguration &configuration : configurations)
            {
                if(configuration.id != activeId)
                {
                    continue;
                }

                buffs->loadConfiguration(configuration);

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
        [overlay, &transcendenceVision]()
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
        [overlay, &transcendenceVision]()
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
        [overlay, &transcendenceVision]()
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
    // ATMA ZONES
    // ==================================================

    AtmaZoneManager atmaZones(
        &keyboard,
        overlayRoot,
        atma.visionCore(),
        nullptr
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::atmaZonesConfigRequested,
        &atmaZones,
        &AtmaZoneManager::configure
        );

    QObject::connect(
        &mainWindow,
        &MainWindow::atmaToggled,
        &atmaZones,
        &AtmaZoneManager::setEnabled
        );

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

    // ==================================================
    // RESTORE TOGGLE STATES
    // ==================================================

    mainWindow.loadToggleStates();

    // ==================================================
    // RESONANCE GATE -> PAUSA / RIPRESA
    // ==================================================

    bool overlayHidden = false;

    // Stato del checkbox "Pausa"
    bool resonanceGatePauseCooldown =
        mainWindow.resonanceGatePauseCooldownEnabled();

    auto pauseAll = [&]()
    {
        atma.visionCore()->pauseAtmaCooldowns();
        atma.visionOverlay()->pauseAtmaCooldown();
        atmaZones.setGateOpen(false);
        overlay->pauseAtmaGate();
        skills->pauseAtmaGate();
        transcendenceVision.pauseAtmaGate();
    };

    auto resumeAll = [&]()
    {
        atma.visionCore()->resumeAtmaCooldowns();
        atma.visionOverlay()->resumeAtmaCooldown();
        atmaZones.setGateOpen(true);
        overlay->resumeAtmaGate();
        skills->resumeAtmaGate();
        transcendenceVision.resumeAtmaGate();
    };
    auto pauseCooldowns = [&]()
    {
        atma.visionCore()->pauseAtmaCooldowns();
        atma.visionOverlay()->pauseAtmaCooldown();

        overlay->pauseAtmaGate();
        skills->pauseAtmaGate();
        transcendenceVision.pauseAtmaGate();
    };

    auto resumeCooldowns = [&]()
    {
        atma.visionCore()->resumeAtmaCooldowns();
        atma.visionOverlay()->resumeAtmaCooldown();

        overlay->resumeAtmaGate();
        skills->resumeAtmaGate();
        transcendenceVision.resumeAtmaGate();
    };

    // ==================================================
    // CAMBIO CHECKBOX "PAUSA"
    // ==================================================

    QObject::connect(
        &mainWindow,
        &MainWindow::resonanceGatePauseCooldownChanged,
        overlayRoot,
        [&](bool enabled)
        {
            resonanceGatePauseCooldown = enabled;

            if(!enabled)
            {
                resumeAll();
                return;
            }

            if(resonanceGate.isGateOpen())
            {
                resumeAll();
            }
            else
            {
                pauseAll();
            }
        },
        Qt::QueuedConnection
        );

    // ==================================================
    // GATE CLOSED
    // ==================================================

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateClosed,
        overlayRoot,
        [&]()
        {
            // SEMPRE: il Gate decide se Atma può usare le pozioni
            atmaZones.setGateOpen(false);

            // SOLO Pausa decide se fermare i cooldown
            if(!overlayHidden &&
                resonanceGatePauseCooldown)
            {
                pauseCooldowns();
            }
        }
        );

    // ==================================================
    // GATE OPENED
    // ==================================================

    QObject::connect(
        &resonanceGate,
        &ResonanceGateManager::gateOpened,
        overlayRoot,
        [&]()
        {
            // SEMPRE: il Gate decide se Atma può usare le pozioni
            atmaZones.setGateOpen(true);

            // SOLO Pausa decide se riprendere i cooldown
            if(resonanceGatePauseCooldown)
            {
                resumeCooldowns();
            }
        }
        );

    // ==================================================
    // HIDE / SHOW OVERLAY (DELETE)
    // ==================================================
    //
    // Mentre l'overlay e' nascosto:
    //  - il gate viene ignorato
    //  - i cooldown continuano a scorrere
    //  - non si registrano nuovi eventi Atma
    //
    // IMPORTANTE:
    // questa logica NON dipende dal checkbox "Pausa".
    // DELETE mantiene quindi il comportamento originale.
    // ==================================================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        overlayRoot,
        [&, overlayRoot](int key)
        {
            if(key != VK_DELETE)
            {
                return;
            }

            overlayRoot->toggleVisibility();

            overlayHidden = !overlayHidden;

            atma.visionCore()->setDetectionSuspended(
                overlayHidden
                );

            atmaZones.setDetectionSuspended(
                overlayHidden
                );

            if(overlayHidden)
            {
                // Overlay nascosto:
                // il Gate viene ignorato e i cooldown continuano.
                resumeAll();
            }
            else if(resonanceGatePauseCooldown &&
                     !resonanceGate.isGateOpen())
            {
                // Overlay nuovamente visibile:
                // il Gate può mettere in pausa i cooldown
                // SOLO se il checkbox "Pausa" è attivo.
                pauseAll();
            }
            else
            {
                // Checkbox "Pausa" disattivato oppure Gate aperto:
                // i cooldown devono continuare.
                resumeAll();
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
    // APPLICATION LOOP
    // ==================================================

    return app.exec();
}