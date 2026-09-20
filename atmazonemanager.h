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

public slots:
    // Ricevuto dall'esterno (ResonanceGateManager), sostituisce
    // il vecchio m_blueState calcolato internamente.
    void setGateOpen(bool open);

#ifdef QT_DEBUG
    // Ricevuto dall'esterno (ResonanceGateManager::blueDebugFrame),
    // inoltrato alla finestra di debug condivisa.
    void updateBlueDebug(QImage frame, bool isMatch);
#endif

private slots:
    void onReferencesLoaded(bool ok);
    void onRedZoneCompared(int index, bool isMatch);

    void processPendingRedEvents();

#ifdef QT_DEBUG
    void onRedDebugFrame(int index, QImage frame, bool isMatch);
#endif

private:
    static constexpr int RED_COUNT = 6;

    // Finestra temporale durante la quale un evento rosso resta
    // "in attesa" prima di essere confermato/scartato in base allo
    // stato del gate (che può arrivare con un ritardo variabile,
    // dato che ora proviene da un manager esterno e asincrono).
    static constexpr int GATE_DELAY_MS = 30;

    enum class ZoneState { Unknown, Match, Mismatch };

    GlobalKeyboard *m_keyboard = nullptr;
    OverlayRoot *m_overlayRoot = nullptr;
    BuffVisionCore *m_core = nullptr;

    AtmaZoneCaptureSetup *m_captureSetup = nullptr;

    bool m_enabled = false;
    bool m_configured = false;

    // Stato del gate esterno: true = aperto (Risonanza presente,
    // nessuna pausa), false = chiuso (Risonanza assente).
    bool m_gateOpen = true;

    std::array<int, RED_COUNT> m_redRegionIds;
    std::array<ZoneState, RED_COUNT> m_redStates;

    std::array<bool, RED_COUNT> m_pendingRedEvents{};
    bool m_pendingEventsBlockedByGate = false;

    QTimer m_gateTimer;

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

    void clearPendingRedEvents();
};