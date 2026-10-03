#pragma once

#include <QObject>
#include <QImage>
#include <QPointer>
#include <QRect>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "customsearchermultifinder.h"

class CaptureCoordinator;
class CustomSearcherWorker;
class OverlayRoot;
class CustomSearcherConfigWindow;
class CustomSearcherSearchArea;
class CustomSearcherOverlay;

class CustomSearcherManager : public QObject
{
    Q_OBJECT

public:

    struct TemplateConfig
    {
        int id = -1;

        QImage displayImage;
        QImage templateImage;

        int cooldownMs = 0;

        // Internal defaults.
        int pixelTolerance = 7;
        double matchThreshold = 97.5;

        // Position and size of the visual overlay.
        QRect overlayRect;
        bool searchEnabled = true;
    };

    explicit CustomSearcherManager(
        CaptureCoordinator *captureCoordinator,
        OverlayRoot *overlayRoot,
        QObject *parent = nullptr
        );

    ~CustomSearcherManager() override;

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setSearchRegion(const QRect &rect);
    QRect searchRegion() const;

    void setCaptureInterval(int intervalMs);
    int captureInterval() const;

    void setTemplates(
        const QVector<TemplateConfig> &templates
        );

    QVector<TemplateConfig> templates() const;

signals:

    void templateFound(
        int templateId,
        const QRect &rect,
        double score
        );

    void searcherStateChanged(
        bool enabled
        );

    void searchRegionChanged(
        const QRect &rect
        );

    void templateCropReady(
        const QImage &image
        );

    void templateCropCanceled();

public slots:

    void configure();
    void configureSearchRegion();
    void captureTemplateCrop();

private slots:

    void onFrameCaptured(QImage frame);

    void onWorkerResults(
        const QVector<CustomSearcherMultiFinder::Result> &results
        );

    void onCooldownFinished(
        int templateId
        );

    void onOverlayGeometryChanged(
        int templateId,
        const QRect &rect
        );

private:

    struct RuntimeTemplate
    {
        TemplateConfig config;

        bool coolingDown = false;

        QPointer<QTimer> cooldownTimer;
        QPointer<CustomSearcherOverlay> overlay;
    };

    void rebuildWorkerTemplates();

    void startCooldown(
        int templateId
        );

    bool hasActiveTemplates() const;

    void subscribeCapture();
    void unsubscribeCapture();

    void loadSettings();
    void saveSearchRegion();

    void loadTemplates();
    void saveTemplates();
    void cleanupTemplateFiles();

    void createOverlay(
        RuntimeTemplate &runtime
        );

    void destroyOverlays();

    void saveOverlayGeometry(
        int templateId,
        const QRect &rect
        );

    QString templateImagesPath() const;

private:

    CaptureCoordinator *m_captureCoordinator = nullptr;

    QPointer<CustomSearcherConfigWindow> m_configWindow = nullptr;
    QPointer<CustomSearcherSearchArea> m_searchAreaWindow = nullptr;

    OverlayRoot *m_overlayRoot = nullptr;

    QThread m_workerThread;
    CustomSearcherWorker *m_worker = nullptr;

    bool m_enabled = false;

    int m_regionId = -1;

    QRect m_searchRegion;

    int m_captureIntervalMs = 150;

    QVector<RuntimeTemplate> m_templates;
};