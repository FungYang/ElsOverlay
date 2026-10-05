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

    enum class SearchZone
    {
        Zone1 = 0,
        Zone2 = 1
    };


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

        // Template enabled/disabled.
        bool searchEnabled = true;

        // Search zone assigned to this template.
        SearchZone searchZone = SearchZone::Zone1;
    };


    explicit CustomSearcherManager(
        CaptureCoordinator *captureCoordinator,
        OverlayRoot *overlayRoot,
        QObject *parent = nullptr
        );

    ~CustomSearcherManager() override;


    // ---------------------------------------------------------
    // Searcher state
    // ---------------------------------------------------------

    void setEnabled(bool enabled);
    bool isEnabled() const;
    void reset();


    // ---------------------------------------------------------
    // Search zones
    // ---------------------------------------------------------

    /*
     * Legacy/compatibility API.
     *
     * setSearchRegion() operates on Zone 1.
     */
    void setSearchRegion(
        const QRect &rect
        );

    /*
     * Zone 2.
     */
    void setSearchRegion2(
        const QRect &rect
        );

    /*
     * Legacy/compatibility API.
     *
     * searchRegion() returns Zone 1.
     */
    QRect searchRegion() const;

    QRect searchRegion1() const;
    QRect searchRegion2() const;


    // ---------------------------------------------------------
    // Capture
    // ---------------------------------------------------------

    void setCaptureInterval(
        int intervalMs
        );

    int captureInterval() const;


    // ---------------------------------------------------------
    // Templates
    // ---------------------------------------------------------

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


    /*
     * Legacy/compatibility signal.
     *
     * This refers to Zone 1.
     */
    void searchRegionChanged(
        const QRect &rect
        );


    /*
     * Signal emitted when Zone 2 ROI changes.
     */
    void searchRegion2Changed(
        const QRect &rect
        );


    void templateCropReady(
        const QImage &image
        );


    void templateCropCanceled();


public slots:

    /*
     * Opens the template configuration window.
     */
    void configure();


    /*
     * Legacy search-area configuration.
     *
     * Configures Zone 1.
     */
    void configureSearchRegion();


    /*
     * Configures the independent ROI of Zone 2.
     */
    void configureSearchRegion2();


    /*
     * Captures a template from the currently configured
     * search area.
     *
     * At the moment this uses Zone 1.
     */
    void captureTemplateCrop(SearchZone zone);


private slots:

    // ---------------------------------------------------------
    // Capture callbacks
    // ---------------------------------------------------------

    /*
     * Legacy callback.
     *
     * Kept so existing connections/code do not break.
     * Internally it forwards to Zone 1.
     */
    void onFrameCaptured(
        QImage frame
        );


    /*
     * Capture callback for Zone 1.
     */
    void onFrameCapturedZone1(
        QImage frame
        );


    /*
     * Capture callback for Zone 2.
     */
    void onFrameCapturedZone2(
        QImage frame
        );


    // ---------------------------------------------------------
    // Worker
    // ---------------------------------------------------------

    void onWorkerResultsZone1(
        const QVector<CustomSearcherMultiFinder::Result> &results
        );

    void onWorkerResultsZone2(
        const QVector<CustomSearcherMultiFinder::Result> &results
        );


    // ---------------------------------------------------------
    // Cooldown
    // ---------------------------------------------------------

    void onCooldownFinished(
        int templateId
        );


    // ---------------------------------------------------------
    // Overlay
    // ---------------------------------------------------------

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


    // ---------------------------------------------------------
    // Worker management
    // ---------------------------------------------------------

    void rebuildWorkerTemplates();


    // ---------------------------------------------------------
    // Cooldown
    // ---------------------------------------------------------

    void startCooldown(
        int templateId
        );


    // ---------------------------------------------------------
    // State
    // ---------------------------------------------------------

    bool hasActiveTemplates() const;


    // ---------------------------------------------------------
    // CaptureCoordinator
    // ---------------------------------------------------------

    void subscribeCapture();
    void unsubscribeCapture();


    // ---------------------------------------------------------
    // Settings
    // ---------------------------------------------------------

    void loadSettings();
    void saveSearchRegion();


    // ---------------------------------------------------------
    // Template persistence
    // ---------------------------------------------------------

    void loadTemplates();
    void saveTemplates();
    void cleanupTemplateFiles();


    // ---------------------------------------------------------
    // Overlay management
    // ---------------------------------------------------------

    void createOverlay(
        RuntimeTemplate &runtime
        );

    void destroyOverlays();


    void saveOverlayGeometry(
        int templateId,
        const QRect &rect
        );


    // ---------------------------------------------------------
    // Files
    // ---------------------------------------------------------

    QString templateImagesPath() const;


private:

    // ---------------------------------------------------------
    // Dependencies
    // ---------------------------------------------------------

    CaptureCoordinator *m_captureCoordinator = nullptr;

    QPointer<CustomSearcherConfigWindow> m_configWindow = nullptr;

    QPointer<CustomSearcherSearchArea> m_searchAreaWindow = nullptr;

    OverlayRoot *m_overlayRoot = nullptr;


    // ---------------------------------------------------------
    // Worker thread
    // ---------------------------------------------------------

    QThread m_workerThread;

    CustomSearcherWorker *m_worker = nullptr;


    // ---------------------------------------------------------
    // Searcher state
    // ---------------------------------------------------------

    bool m_enabled = false;


    // ---------------------------------------------------------
    // Search regions
    //
    // Region 1 = Zone1
    // Region 2 = Zone2
    // ---------------------------------------------------------

    int m_regionId1 = -1;
    int m_regionId2 = -1;

    QRect m_searchRegion1;
    QRect m_searchRegion2;


    // ---------------------------------------------------------
    // Capture interval
    // ---------------------------------------------------------

    int m_captureIntervalMs = 150;
    static constexpr int SEARCH_STOP_THRESHOLD_MS = 46000;


    // ---------------------------------------------------------
    // Templates
    // ---------------------------------------------------------

    QVector<RuntimeTemplate> m_templates;
};