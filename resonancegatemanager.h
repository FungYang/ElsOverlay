#pragma once

#include <QObject>
#include <QThread>
#include <QImage>

class GlobalKeyboard;
class OverlayRoot;
class ResonanceGateCaptureSetup;
class ResonanceGateWorker;

class ResonanceGateManager : public QObject
{
    Q_OBJECT

public:
    explicit ResonanceGateManager(
        GlobalKeyboard *keyboard,
        OverlayRoot *overlayRoot,
        QObject *parent = nullptr
        );

    ~ResonanceGateManager();

    void configure();
    void setEnabled(bool enabled);

    bool isGateOpen() const { return m_gateOpen; } // per query sincrona se serve

signals:
    void gateOpened();  // Risonanza torna presente -> pausa terminata
    void gateClosed();  // Risonanza sparisce -> tutto in pausa
#ifdef QT_DEBUG
    void debugFrame(const QImage &image, bool isMatch);
#endif

private slots:
    void onReferenceLoaded(bool ok);
    void onCompared(bool isMatch);

private:
    enum class State { Unknown, Match, Mismatch };

    GlobalKeyboard *m_keyboard = nullptr;
    OverlayRoot *m_overlayRoot = nullptr;
    ResonanceGateCaptureSetup *m_captureSetup = nullptr;

    QThread *m_workerThread = nullptr;
    ResonanceGateWorker *m_worker = nullptr;

    bool m_enabled = false;
    bool m_configured = false;
    bool m_gateOpen = true; // Match = risonanza presente = gate aperto = nessuna pausa
    State m_state = State::Unknown;

    int m_regionId = -1;

    void registerRegion();
    void unregisterRegion();
};