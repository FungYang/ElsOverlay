#ifndef BUFFVISIONMANAGER_H
#define BUFFVISIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

#include "buffvisioncapture.h"
#include "buffvisiondetector.h"
#include "overlayroot.h"

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

    void configure();
    void setEnabled(bool enabled);


private:

    OverlayRoot *overlayRoot = nullptr;

    GlobalKeyboard *keyboard = nullptr;

    BuffVisionCore *core = nullptr;

    BuffVisionCapture *capture = nullptr;

    BuffVisionDetector *detector = nullptr;

    BuffVisionOverlay *overlay = nullptr;

    BuffVisionCaptureSetup *captureSetup = nullptr;

    void startTracking();
    void resetTracking();

#ifdef QT_DEBUG

    BuffVisionDebug *debugWindow = nullptr;

#endif

    // Ultimo numero valido rilevato per ogni crop.
    //
    // 1000 = nessun valore valido.
    int lastCrop1Number = 1000;
    int lastCrop2Number = 1000;

    QElapsedTimer eventTimer;

    qint64 visionCycle = 0;

    qint64 crop1EventTime = -1;
    qint64 crop2EventTime = -1;

    qint64 crop1EventCycle = -1;
    qint64 crop2EventCycle = -1;

    bool configured = false;

    bool enabled = false;

    QTimer visionTimer;

    void showSetup();
};

#endif