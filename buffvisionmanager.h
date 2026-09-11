#ifndef BUFFVISIONMANAGER_H
#define BUFFVISIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>

#include "buffvisioncapture.h"
#include "buffvisiondetector.h"
#include "overlayroot.h"

class BuffVisionDetectionWorker;

class GlobalKeyboard;

class BuffVisionCore;
class BuffVisionOverlay;
class BuffVisionCaptureSetup;

#ifdef QT_DEBUG
class BuffVisionDebug;
#endif

class BuffVisionManager : public QObject
{
    Q_OBJECT

public:

    explicit BuffVisionManager(
        GlobalKeyboard *keyboard,
        OverlayRoot *overlayRoot,
        QObject *parent = nullptr
        );

    ~BuffVisionManager();

    void startTracking();
    void resetTracking();
    void configure();
    void setEnabled(bool enabled);

private slots:
    // NUOVO: sostituiscono la lambda dentro visionTimer.
    void onCrop1FrameReady(QImage frame);
    void onCrop2FrameReady(QImage frame);
    void onNumberDetected(int cropId, int number);
    void onModelLoaded(bool ok);


private:

    OverlayRoot *overlayRoot = nullptr;

    GlobalKeyboard *keyboard = nullptr;

    BuffVisionCore *core = nullptr;

    BuffVisionCapture *capture = nullptr;

    BuffVisionDetector *detector = nullptr;

    BuffVisionOverlay *overlay = nullptr;

    BuffVisionCaptureSetup *captureSetup = nullptr;


#ifdef QT_DEBUG

    BuffVisionDebug *debugWindow = nullptr;

#endif

    // NUOVO
    QThread *m_detectionThread = nullptr;
    BuffVisionDetectionWorker *m_detectionWorker = nullptr;

    // Ultimo numero valido rilevato per ogni crop.
    //
    // 1000 = nessun valore valido.
    bool enabled = false;
    bool configured = false;

    int lastCrop1Number = 1000;
    int lastCrop2Number = 1000;

    int visionCycle = 0;
    qint64 crop1EventTime = -1;
    qint64 crop2EventTime = -1;
    int crop1EventCycle = -1;
    int crop2EventCycle = -1;
    QElapsedTimer eventTimer;

    void showSetup();
};

#endif