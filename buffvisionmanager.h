#ifndef BUFFVISIONMANAGER_H
#define BUFFVISIONMANAGER_H

#include <QObject>
#include "buffvisioncapture.h"
#include "buffvisiondetector.h"
#include <QTimer>
#include "overlayroot.h"


class GlobalKeyboard;

class BuffVisionCore;
class BuffVisionOverlay;
class BuffVisionSelector;
class BuffVisionCaptureSetup;
class BuffVisionCapture;
class BuffVisionDetector;
class OverlayRoot;
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



private:

    enum class CaptureReferenceMode
    {
        None,
        Reference1,
        Reference2
    };

    OverlayRoot *overlayRoot = nullptr;

    CaptureReferenceMode referenceMode =
        CaptureReferenceMode::None;

    BuffVisionCapture *capture = nullptr;

    GlobalKeyboard *keyboard = nullptr;


    BuffVisionCore *core = nullptr;


    BuffVisionOverlay *overlay = nullptr;


    BuffVisionSelector *selector = nullptr;


    BuffVisionCaptureSetup *captureSetup = nullptr;

    BuffVisionDetector *detector = nullptr;

    #ifdef QT_DEBUG
        BuffVisionDebug *debugWindow = nullptr;
    #endif

    VisionState crop1State =
        VisionState::Unknown;


    VisionState crop2State =
        VisionState::Unknown;

    VisionState lastCrop1State = VisionState::Unknown;
    VisionState lastCrop2State = VisionState::Unknown;



    bool configured = false;

    bool enabled = false;

    bool configurationConfirmed = false;

    QTimer visionTimer;

    bool hasReferences() const;

    void showSelector();

    void showSetup();

    void saveCurrentReference();


};

#endif