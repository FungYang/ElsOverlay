#include "buffvisionmanager.h"
#include "globalkeyboard.h"
#include "buffvisioncore.h"
#include "buffvisionoverlay.h"
#include "buffvisioncapturesetup.h"
#include "buffvisiondetectionworker.h"
#include "capturecoordinator.h"
#include "overlayroot.h"
#include "buffvisionconfig.h"

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
    core = new BuffVisionCore(this);

    capture = new BuffVisionCapture(this);
    capture->loadSettings();

    // =========================
    // DETECTION WORKER (thread dedicato all'inferenza ONNX)
    // =========================
    m_detectionWorker = new BuffVisionDetectionWorker(); // niente parent
    m_detectionThread = new QThread(this);
    m_detectionWorker->moveToThread(m_detectionThread);

    connect(m_detectionWorker, &BuffVisionDetectionWorker::modelLoaded,
            this, &BuffVisionManager::onModelLoaded);
    connect(m_detectionWorker, &BuffVisionDetectionWorker::numberDetected,
            this, &BuffVisionManager::onNumberDetected);

    m_detectionThread->start();

    QScreen *screen = QGuiApplication::primaryScreen();
    QString modelName = "best_1080.onnx";

    if (screen && BuffVisionConfig::is2K(screen->size()))
        modelName = "best_2k.onnx";

    const QString modelPath =
        QCoreApplication::applicationDirPath() + "/models/" + modelName;

    QMetaObject::invokeMethod(m_detectionWorker, "loadModel", Qt::QueuedConnection,
                              Q_ARG(QString, modelPath));
    // configured verrà settato da onModelLoaded()

    // =========================
    // DEBUG (invariato)
    // =========================
#ifdef QT_DEBUG
    debugWindow = new BuffVisionDebug(this->overlayRoot);
    this->overlayRoot->registerOverlay(debugWindow);

    if (screen)
    {
        const QSize resolution = screen->size();
        const QSize cropSize = BuffVisionConfig::cropSizeForScreen(resolution);
        debugWindow->setResolution(resolution.width(), resolution.height(), cropSize);
    }

    debugWindow->hide();
#endif

    // =========================
    // ATMA OVERLAY (invariato)
    // =========================
    overlay = new BuffVisionOverlay(core, this->overlayRoot);
    this->overlayRoot->registerOverlay(overlay);
    overlay->hide();

    captureSetup = nullptr;

    // RIMOSSO: connect(&visionTimer, ...) — sostituito da
    // onCrop1FrameReady()/onCrop2FrameReady()/onNumberDetected().

    // =========================
    // KEYBOARD (invariato)
    // =========================
    connect(
        keyboard, &GlobalKeyboard::keyPressed, this,
        [this](int key)
        {
            if (key >= '1' && key <= '6')
            {
                if (!enabled) return;
                core->registerAction();
            }
        }
        );

    // ENTER - CONFERMA CONFIGURAZIONE
    connect(
        keyboard, &GlobalKeyboard::confirmPressed, this,
        [this]()
        {
            if (!captureSetup) return;
            if (!captureSetup->isVisible()) return;

            captureSetup->saveSettings();

            capture->setCropAreas(
                captureSetup->getCropRect1(),
                captureSetup->getCropRect2()
                );

            configured = true; // resta come nell'originale: qui non dipende dal modello

            captureSetup->hide();

            lastCrop1Number = 1000;
            lastCrop2Number = 1000;

            this->overlayRoot->raiseAll();
        }
        );

    // RESET GLOBALE (invariato)
    connect(
        keyboard, &GlobalKeyboard::resetPressed, this,
        [this]()
        {
            resetTracking();
            if (captureSetup) captureSetup->hide();
            this->overlayRoot->raiseAll();
        }
        );
}
void BuffVisionManager::onModelLoaded(bool ok)
{
    configured = ok; // NOTA: vedi avviso sotto sul doppio significato di 'configured'
}


// ============================================================
// START TRACKING
// ============================================================

void BuffVisionManager::startTracking()
{
    if (!enabled || !configured)
        return;

    core->reset();

    lastCrop1Number = 1000;
    lastCrop2Number = 1000;

    core->startTracking();

    visionCycle = 0;
    crop1EventTime = -1;
    crop2EventTime = -1;
    crop1EventCycle = -1;
    crop2EventCycle = -1;
    eventTimer.restart();

    // Sostituisce visionTimer.start(50): iscrizione al coordinator
    // per entrambe le region, stesso intervallo di prima.
    CaptureCoordinator::instance()->subscribe(
        capture->crop1RegionId(), 50, this, "onCrop1FrameReady"
        );
    CaptureCoordinator::instance()->subscribe(
        capture->crop2RegionId(), 50, this, "onCrop2FrameReady"
        );
}


// ============================================================
// RESET TRACKING
// ============================================================

void BuffVisionManager::resetTracking()
{
    CaptureCoordinator::instance()->unsubscribe(capture->crop1RegionId());
    CaptureCoordinator::instance()->unsubscribe(capture->crop2RegionId());

    core->reset();

    lastCrop1Number = 1000;
    lastCrop2Number = 1000;

    visionCycle = 0;
    crop1EventTime = -1;
    crop2EventTime = -1;
    crop1EventCycle = -1;
    crop2EventCycle = -1;

    if (overlay) overlay->resetOverlay();

#ifdef QT_DEBUG
    if (debugWindow)
    {
        debugWindow->updateNumbers(1000, 1000);
        debugWindow->setLastEvent("---");
    }
#endif

    if (enabled)
        startTracking();
}

// =========================
// FRAME READY -> inoltra al detection worker
// =========================

void BuffVisionManager::onCrop1FrameReady(QImage frame)
{
    if (!enabled || !configured) return;

    QMetaObject::invokeMethod(
        m_detectionWorker, "detectFrame", Qt::QueuedConnection,
        Q_ARG(int, 1), Q_ARG(QImage, frame)
        );
}

void BuffVisionManager::onCrop2FrameReady(QImage frame)
{
    if (!enabled || !configured) return;

    QMetaObject::invokeMethod(
        m_detectionWorker, "detectFrame", Qt::QueuedConnection,
        Q_ARG(int, 2), Q_ARG(QImage, frame)
        );
}

// =========================
// RISULTATO DETECTION (arriva sul thread GUI, cross-thread auto-queued)
// =========================

void BuffVisionManager::onNumberDetected(int cropId, int number)
{
    if (!enabled || !configured) return;

    ++visionCycle;

#ifdef QT_DEBUG
    if (debugWindow)
    {
        if (cropId == 1) debugWindow->updateNumbers(number, lastCrop2Number);
        else debugWindow->updateNumbers(lastCrop1Number, number);
    }
#endif

    if (cropId == 1)
    {
        if (number != 1000 && lastCrop1Number != 1000 && number == lastCrop1Number - 1)
        {
            crop1EventTime = eventTimer.elapsed();
            crop1EventCycle = visionCycle;
            core->onCrop1Event();

#ifdef QT_DEBUG
            if (debugWindow)
                debugWindow->setLastEvent(QString("Crop 1 -> %1").arg(number));
#endif
        }

        if (number != 1000)
            lastCrop1Number = number;
    }
    else // cropId == 2
    {
        if (number != 1000 && lastCrop2Number != 1000 && number == lastCrop2Number - 1)
        {
            crop2EventTime = eventTimer.elapsed();
            crop2EventCycle = visionCycle;
            core->onCrop2Event();

#ifdef QT_DEBUG
            if (debugWindow)
                debugWindow->setLastEvent(QString("Crop 2 -> %1").arg(number));
#endif
        }

        if (number != 1000)
            lastCrop2Number = number;
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

void BuffVisionManager::setEnabled(bool value)
{
    enabled = value;

    if (!enabled)
    {
        CaptureCoordinator::instance()->unsubscribe(capture->crop1RegionId());
        CaptureCoordinator::instance()->unsubscribe(capture->crop2RegionId());

        core->reset();
        lastCrop1Number = 1000;
        lastCrop2Number = 1000;

        overlay->hide();
        overlay->resetOverlay();

#ifdef QT_DEBUG
        if (debugWindow) debugWindow->hide();
#endif
        return;
    }

    overlay->show();
    this->overlayRoot->raiseAll();

#ifdef QT_DEBUG
    if (debugWindow) { debugWindow->show(); debugWindow->raise(); }
#endif

    startTracking();
}


// ============================================================
// DESTRUCTOR
// ============================================================

BuffVisionManager::~BuffVisionManager()
{
    m_detectionThread->quit();
    m_detectionThread->wait();
    delete m_detectionWorker;
}