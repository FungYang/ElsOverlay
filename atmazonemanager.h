#pragma once

#include <QObject>
#include <QImage>
#include <QThread>
#include <QTimer>
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

signals:
    void invariantEntered();
    void invariantExited();

private slots:
    void onReferencesLoaded(bool ok);
    void onRedZoneCompared(int index, bool isMatch);
    void onBlueZoneCompared(bool isMatch);

    void processPendingRedEvents();

#ifdef QT_DEBUG
    void onRedDebugFrame(int index, QImage frame, bool isMatch);
    void onBlueDebugFrame(QImage frame, bool isMatch);
#endif

private:
    static constexpr int RED_COUNT = 6;

    // Finestra temporale durante la quale RED e BLUE vengono
    // considerati contemporanei.
    static constexpr int GATE_DELAY_MS = 30;

    enum class ZoneState
    {
        Unknown,
        Match,
        Mismatch
    };

    GlobalKeyboard *m_keyboard = nullptr;
    OverlayRoot *m_overlayRoot = nullptr;
    BuffVisionCore *m_core = nullptr;

    AtmaZoneCaptureSetup *m_captureSetup = nullptr;

    bool m_enabled = false;
    bool m_configured = false;

    std::array<int, RED_COUNT> m_redRegionIds;
    int m_blueRegionId = -1;

    std::array<ZoneState, RED_COUNT> m_redStates;
    ZoneState m_blueState = ZoneState::Unknown;

    // RED events waiting for gate confirmation.
    std::array<bool, RED_COUNT> m_pendingRedEvents{};

    // Se BLUE entra in mismatch durante la finestra del gate,
    // gli eventi RED pendenti vengono invalidati.
    bool m_pendingEventsBlockedByBlue = false;

    QTimer m_gateTimer;

    QThread *m_workerThread = nullptr;
    AtmaZoneWorker *m_worker = nullptr;

    std::array<std::unique_ptr<AtmaRedZoneProxy>, RED_COUNT> m_redProxies;
    std::unique_ptr<AtmaRedZoneProxy> m_blueProxy;

#ifdef QT_DEBUG
    AtmaDebugWindow *m_debugWindow = nullptr;
#endif

    void registerAllRegions();
    void unregisterAllRegions();
    void subscribeAll();
    void unsubscribeAll();

    void clearPendingRedEvents();
};
