#pragma once

#include <QObject>
#include <QRect>
#include <QImage>
#include <QTimer>

class GlobalKeyboard;
class OverlayRoot;
class Overlay;
class TranscendenceCaptureSetup;
class TranscendencePrecisionCrop;

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

private:
    void loadSettings();
    void saveSettings();
    void loadIcon();

    void registerSearchRegion();
    void unregisterSearchRegion();

    void startScanning();
    void stopAll();
    void stopScanning();
    void scanTick();

    void openPrecisionCrop();
    void savePreciseIcon(const QImage &icon);
    void closePrecisionCrop();

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

private:
    GlobalKeyboard *keyboard = nullptr;
    OverlayRoot *overlayRoot = nullptr;
    Overlay *overlay = nullptr;

    TranscendenceCaptureSetup *captureSetup = nullptr;
    TranscendencePrecisionCrop *precisionCrop = nullptr;

    QRect m_searchArea;
    int m_searchRegionId = -1;

    QImage m_templateIcon;

    QTimer m_delayTimer;
    QTimer m_scanTimer;

    bool m_enabled = false;
    bool m_configured = false;
};
