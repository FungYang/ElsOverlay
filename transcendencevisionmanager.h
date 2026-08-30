#pragma once

#include <QObject>
#include <QRect>
#include <QImage>
#include <QTimer>

class GlobalKeyboard;
class OverlayRoot;
class Overlay;
class TranscendenceCaptureSetup;


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

    void configure();

    void setEnabled(
        bool enabled
        );

public slots:

    // Chiamato dagli stessi eventi che avviano
    // il cooldown sull'overlay (ctrlPressed, tasto '6').
    void onCooldownStarted();

    // Chiamato dagli stessi eventi che resettano
    // il cooldown (resetPressed, transcendenceResetPressed).
    void onCooldownReset();

private:

    void loadSettings();
    void saveSettings();

    void loadIcon();

    void startDelay();
    void stopAll();

    void startScanning();
    void stopScanning();

    void scanTick();

    void saveCurrentIcon();

    bool findIcon(
        const QImage &area,
        QRect &foundRect,
        double &score
        ) const;

    double compareAt(
        const QImage &area,
        int offsetX,
        int offsetY
        ) const;


    GlobalKeyboard *keyboard = nullptr;
    OverlayRoot *overlayRoot = nullptr;
    Overlay *overlay = nullptr;

    TranscendenceCaptureSetup *captureSetup = nullptr;


    QRect m_searchArea;
    QImage m_templateIcon;

    QTimer m_delayTimer;
    QTimer m_scanTimer;

    bool m_enabled = false;
    bool m_configured = false;
};