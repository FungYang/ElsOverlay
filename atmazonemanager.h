#pragma once

#include <QObject>
#include <QImage>
#include <QThread>
#include <QHash>
#include <array>
#include <memory>

class GlobalKeyboard;
class OverlayRoot;
class AtmaZoneCaptureSetup;
class BuffVisionCore;
class AtmaZoneWorker;
class AtmaRedZoneProxy;

#ifdef QT_DEBUG
class AtmaDebugWindow;
#endif

class AtmaZoneManager : public QObject
{
    Q_OBJECT

public:
    explicit AtmaZoneManager(
        GlobalKeyboard *keyboard,
        OverlayRoot *overlayRoot,
        BuffVisionCore *core,
        QObject *parent = nullptr
        );

    ~AtmaZoneManager();

    void configure();
    void setEnabled(bool enabled);
    void setDetectionSuspended(bool on);

public slots:

    // Ricevuto dall'esterno (ResonanceGateManager).
    // Il frameId identifica lo stesso frame di acquisizione
    // utilizzato dalle zone rosse.
    void setGateFrame(
        quint64 frameId,
        bool isMatch
        );

#ifdef QT_DEBUG
    // Ricevuto dall'esterno (ResonanceGateManager).
    // Inoltrato alla finestra di debug condivisa.
    void updateBlueDebug(
        QImage frame,
        bool isMatch
        );
#endif

private slots:

    void onReferencesLoaded(bool ok);

    void onRedZoneCompared(
        int index,
        quint64 frameId,
        bool isMatch
        );

#ifdef QT_DEBUG
    void onRedDebugFrame(
        int index,
        quint64 frameId,
        QImage frame,
        bool isMatch
        );
#endif

private:

    static constexpr int RED_COUNT = 6;

    enum class ZoneState
    {
        Unknown,
        Match,
        Mismatch
    };

    struct PendingFrame
    {
        std::array<bool, RED_COUNT> redMatch{};
        std::array<bool, RED_COUNT> redReceived{};

        bool blueReceived = false;
        bool blueMatch = false;
    };

    GlobalKeyboard *m_keyboard = nullptr;
    OverlayRoot *m_overlayRoot = nullptr;
    BuffVisionCore *m_core = nullptr;

    AtmaZoneCaptureSetup *m_captureSetup = nullptr;

    bool m_enabled = false;
    bool m_configured = false;
    bool m_detectionSuspended = false;

    std::array<int, RED_COUNT> m_redRegionIds;
    std::array<ZoneState, RED_COUNT> m_redStates;

    QHash<quint64, PendingFrame> m_pendingFrames;

    QThread *m_workerThread = nullptr;
    AtmaZoneWorker *m_worker = nullptr;

    std::array<std::unique_ptr<AtmaRedZoneProxy>, RED_COUNT> m_redProxies;

#ifdef QT_DEBUG
    AtmaDebugWindow *m_debugWindow = nullptr;
#endif

    void registerAllRegions();
    void unregisterAllRegions();
    void subscribeAll();
    void unsubscribeAll();

    void evaluateFrame(quint64 frameId);
};