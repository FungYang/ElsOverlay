#pragma once

#include <QObject>
#include <QRect>
#include <QImage>
#include <QTimer>
#include <QThread>
#include <QSize>
#include "transcendencevisionconfig.h"

class GlobalKeyboard;
class OverlayRoot;
class Overlay;
class TranscendenceCaptureSetup;
class TranscendencePrecisionCrop;
class TranscendenceVisionWorker;   // NUOVO


class TranscendenceVisionManager : public QObject
{
    Q_OBJECT

public:
    explicit TranscendenceVisionManager(
        GlobalKeyboard *keyboard,
        OverlayRoot *overlayRoot,
        Overlay *overlay,
        QObject *parent = nullptr
        );

    ~TranscendenceVisionManager();

    void configure();

    void setEnabled(bool enabled);

public slots:
    void onCooldownStarted();
    void onCooldownReset();

private slots:
    // NUOVO: sostituisce scanTick(). Arriva dal CaptureCoordinator
    // (Qt::QueuedConnection), gira sul thread GUI, ma non fa più
    // nessun calcolo pesante: inoltra solo il frame al worker.
    void onFrameReady(QImage area);

    // NUOVO: arriva dal TranscendenceVisionWorker (cross-thread,
    // quindi automaticamente in coda sul thread GUI).
    void onScanResult(bool found, QRect foundRect, double score, QImage area);

private:
    void loadSettings();
    void saveSettings();
    void loadIcon();

    void registerSearchRegion();
    void unregisterSearchRegion();

    void startScanning();
    void stopAll();
    void stopScanning();
    // scanTick() RIMOSSO: sostituito da onFrameReady()/onScanResult().

    void openPrecisionCrop();
    void savePreciseIcon(
        const QImage &icon,
        const QSize &size
        );
    void closePrecisionCrop();

    void saveCurrentIcon();

    // findIcon()/compareAt() RIMOSSI da qui: si spostano interamente
    // in TranscendenceVisionWorker, che gira su thread dedicato.

private:
    GlobalKeyboard *keyboard = nullptr;
    OverlayRoot *overlayRoot = nullptr;
    Overlay *overlay = nullptr;

    TranscendenceCaptureSetup *captureSetup = nullptr;
    TranscendencePrecisionCrop *precisionCrop = nullptr;

    QRect m_searchArea;
    int m_searchRegionId = -1;

    QImage m_templateIcon;
    int m_iconWidth =
        TranscendenceVisionConfig::ICON_WIDTH;

    int m_iconHeight =
        TranscendenceVisionConfig::ICON_HEIGHT;

    QTimer m_delayTimer;
    // m_scanTimer RIMOSSO: il tick lo guida CaptureCoordinator.

    bool m_enabled = false;
    bool m_configured = false;

    // NUOVO: thread + worker dedicati al calcolo pesante.
    QThread *m_workerThread = nullptr;
    TranscendenceVisionWorker *m_worker = nullptr;
};