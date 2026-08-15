#ifndef BUFFVISIONMANAGER_H
#define BUFFVISIONMANAGER_H

#include <QObject>
#include <QTimer>

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

    enum class CaptureReferenceMode
    {
        None,
        Reference1,
        Reference2
    };


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


    CaptureReferenceMode referenceMode =
        CaptureReferenceMode::None;


    VisionState lastCrop1State =
        VisionState::Unknown;

    VisionState lastCrop2State =
        VisionState::Unknown;


    bool configured = false;

    bool enabled = false;


    QTimer visionTimer;


    bool hasReferences() const;


    void showSetup();

    void saveCurrentReference();

};

#endif