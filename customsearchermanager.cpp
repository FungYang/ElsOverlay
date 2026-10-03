#include "customsearchermanager.h"

#include "overlayroot.h"
#include "capturecoordinator.h"
#include "customsearcherworker.h"
#include "customsearcherconfigwindow.h"
#include "customsearchertemplatecrop.h"
#include "customsearchersearcharea.h"
#include "customsearcheroverlay.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMetaObject>
#include <QScreen>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QtGlobal>
#include <QHash>


CustomSearcherManager::CustomSearcherManager(
    CaptureCoordinator *captureCoordinator,
    OverlayRoot *overlayRoot,
    QObject *parent
    )
    : QObject(parent),
    m_captureCoordinator(captureCoordinator),
    m_overlayRoot(overlayRoot)
{
    loadSettings();
    loadTemplates();

    /*
     * IMPORTANT:
     *
     * La Search Area viene caricata da QSettings.
     * In precedenza veniva caricata ma non registrata
     * nel CaptureCoordinator.
     *
     * Quindi dopo un riavvio m_regionId poteva rimanere -1.
     */
    if(m_captureCoordinator &&
        m_searchRegion.isValid())
    {
        m_regionId =
            m_captureCoordinator->registerRegion(
                m_searchRegion
                );
    }

    for(RuntimeTemplate &runtime : m_templates)
    {
        createOverlay(runtime);
    }

    m_worker = new CustomSearcherWorker();

    m_worker->moveToThread(
        &m_workerThread
        );

    connect(
        &m_workerThread,
        &QThread::finished,
        m_worker,
        &QObject::deleteLater
        );

    connect(
        m_worker,
        &CustomSearcherWorker::resultsReady,
        this,
        &CustomSearcherManager::onWorkerResults,
        Qt::QueuedConnection
        );

    m_workerThread.setObjectName(
        QStringLiteral("CustomSearcherWorkerThread")
        );

    m_workerThread.start();
}


CustomSearcherManager::~CustomSearcherManager()
{
    unsubscribeCapture();

    if(m_regionId >= 0 &&
        m_captureCoordinator)
    {
        m_captureCoordinator->unregisterRegion(
            m_regionId
            );

        m_regionId = -1;
    }

    for(RuntimeTemplate &runtime : m_templates)
    {
        if(runtime.cooldownTimer)
        {
            runtime.cooldownTimer->stop();
            runtime.cooldownTimer->deleteLater();
            runtime.cooldownTimer = nullptr;
        }
    }

    destroyOverlays();

    m_configWindow = nullptr;
    m_searchAreaWindow = nullptr;

    if(m_workerThread.isRunning())
    {
        m_workerThread.quit();
        m_workerThread.wait();
    }

    m_worker = nullptr;
}


void CustomSearcherManager::setEnabled(bool enabled)
{
    if(m_enabled == enabled)
        return;

    m_enabled = enabled;

    if(!m_enabled)
    {
        unsubscribeCapture();

        for(RuntimeTemplate &runtime : m_templates)
        {
            runtime.coolingDown = false;

            if(runtime.cooldownTimer)
            {
                runtime.cooldownTimer->stop();
                runtime.cooldownTimer->deleteLater();
                runtime.cooldownTimer = nullptr;
            }

            if(runtime.overlay)
            {
                runtime.overlay->stopCooldown();
                runtime.overlay->setVisibleBySearcher(false);
            }
        }

        /*
         * OFF = worker completamente vuoto.
         */
        if(m_worker)
        {
            QMetaObject::invokeMethod(
                m_worker,
                [worker = m_worker]()
                {
                    worker->clearTemplates();
                },
                Qt::QueuedConnection
                );
        }
    }
    else
    {
        for(RuntimeTemplate &runtime : m_templates)
        {
            if(runtime.overlay)
            {
                runtime.overlay->setVisibleBySearcher(true);
            }
        }

        /*
         * Ordine importante:
         *
         * 1. assicuriamo la regione
         * 2. configuriamo il worker
         * 3. sottoscriviamo la cattura
         */
        subscribeCapture();

        rebuildWorkerTemplates();
    }

    emit searcherStateChanged(
        m_enabled
        );
}


bool CustomSearcherManager::isEnabled() const
{
    return m_enabled;
}


void CustomSearcherManager::setSearchRegion(
    const QRect &rect
    )
{
    const QRect normalizedRect =
        rect.normalized();

    if(m_searchRegion == normalizedRect)
    {
        /*
         * Se la regione è già uguale ma non è stata
         * ancora registrata, la registriamo comunque.
         */
        if(m_regionId < 0 &&
            m_captureCoordinator &&
            m_searchRegion.isValid())
        {
            m_regionId =
                m_captureCoordinator->registerRegion(
                    m_searchRegion
                    );

            if(m_enabled)
                subscribeCapture();
        }

        return;
    }

    m_searchRegion =
        normalizedRect;

    saveSearchRegion();

    if(!m_captureCoordinator)
        return;

    if(m_regionId < 0)
    {
        if(m_searchRegion.isValid())
        {
            m_regionId =
                m_captureCoordinator->registerRegion(
                    m_searchRegion
                    );

            if(m_enabled)
                subscribeCapture();
        }

        return;
    }

    if(!m_searchRegion.isValid())
    {
        unsubscribeCapture();

        m_captureCoordinator->unregisterRegion(
            m_regionId
            );

        m_regionId = -1;

        return;
    }

    m_captureCoordinator->updateRegion(
        m_regionId,
        m_searchRegion
        );
}


QRect CustomSearcherManager::searchRegion() const
{
    return m_searchRegion;
}


void CustomSearcherManager::setCaptureInterval(
    int intervalMs
    )
{
    intervalMs =
        qMax(
            15,
            intervalMs
            );

    if(m_captureIntervalMs == intervalMs)
        return;

    m_captureIntervalMs =
        intervalMs;

    if(m_enabled)
        subscribeCapture();
}


int CustomSearcherManager::captureInterval() const
{
    return m_captureIntervalMs;
}


void CustomSearcherManager::setTemplates(
    const QVector<TemplateConfig> &templates
    )
{
    /*
     * Conserviamo la geometria corrente degli overlay.
     *
     * La ConfigWindow non deve poter sovrascrivere
     * posizione/dimensione già impostate dall'utente.
     */
    QHash<int, QRect> previousOverlayRects;

    for(const RuntimeTemplate &runtime : m_templates)
    {
        if(runtime.config.id < 0)
            continue;

        if(runtime.config.overlayRect.isValid() &&
            !runtime.config.overlayRect.isEmpty())
        {
            previousOverlayRects.insert(
                runtime.config.id,
                runtime.config.overlayRect
                );
        }
    }

    /*
     * I vecchi cooldown non devono sopravvivere
     * alla sostituzione della configurazione.
     */
    for(RuntimeTemplate &runtime : m_templates)
    {
        if(runtime.cooldownTimer)
        {
            runtime.cooldownTimer->stop();
            runtime.cooldownTimer->deleteLater();
            runtime.cooldownTimer = nullptr;
        }

        runtime.coolingDown = false;
    }

    destroyOverlays();

    m_templates.clear();

    m_templates.reserve(
        templates.size()
        );

    for(const TemplateConfig &config : templates)
    {
        if(config.id < 0)
            continue;

        if(config.displayImage.isNull())
            continue;

        RuntimeTemplate runtime;

        runtime.config =
            config;

        /*
         * La geometria dell'overlay appartiene al Manager.
         *
         * Se questo template esisteva già, manteniamo
         * posizione e dimensione correnti.
         */
        if(previousOverlayRects.contains(config.id))
        {
            runtime.config.overlayRect =
                previousOverlayRects.value(
                    config.id
                    );
        }

        runtime.config.cooldownMs =
            qMax(
                0,
                runtime.config.cooldownMs
                );

        runtime.config.pixelTolerance = 7;
        runtime.config.matchThreshold = 97.5;

        runtime.config.displayImage =
            runtime.config.displayImage.convertToFormat(
                QImage::Format_ARGB32
                );

        if(!runtime.config.templateImage.isNull())
        {
            runtime.config.templateImage =
                runtime.config.templateImage.convertToFormat(
                    QImage::Format_ARGB32
                    );
        }

        m_templates.append(
            runtime
            );
    }

    for(RuntimeTemplate &runtime : m_templates)
    {
        if(!runtime.config.searchEnabled)
            continue;

        createOverlay(runtime);
    }

    saveTemplates();

    if(m_enabled)
    {
        rebuildWorkerTemplates();
    }
    else if(m_worker)
    {
        QMetaObject::invokeMethod(
            m_worker,
            [worker = m_worker]()
            {
                worker->clearTemplates();
            },
            Qt::QueuedConnection
            );
    }
}


QVector<CustomSearcherManager::TemplateConfig>
CustomSearcherManager::templates() const
{
    QVector<TemplateConfig> result;

    result.reserve(
        m_templates.size()
        );

    for(const RuntimeTemplate &runtime : m_templates)
    {
        result.append(
            runtime.config
            );
    }

    return result;
}


void CustomSearcherManager::subscribeCapture()
{
    if (!m_captureCoordinator)
        return;

    /*
     * Difesa:
     * se per qualche motivo il regionId non esiste ancora,
     * registriamo la regione prima della subscription.
     */
    if (m_regionId < 0 && m_searchRegion.isValid())
    {
        m_regionId =
            m_captureCoordinator->registerRegion(m_searchRegion);
    }

    if (m_regionId < 0)
        return;

    if (!m_searchRegion.isValid())
        return;

    if (!m_enabled)
        return;

    /*
     * IMPORTANTE:
     *
     * CaptureCoordinator usa:
     *
     * QMetaObject::invokeMethod(
     *     receiver,
     *     slot,
     *     Qt::QueuedConnection,
     *     Q_ARG(QImage, frame)
     * );
     *
     * In questa overload di invokeMethod() il nome deve essere
     * SOLO quello del metodo.
     *
     * CORRETTO:
     *
     *     "onFrameCaptured"
     *
     * SBAGLIATO:
     *
     *     "onFrameCaptured(QImage)"
     *
     * SBAGLIATO:
     *
     *     SLOT(onFrameCaptured(QImage))
     */
    m_captureCoordinator->subscribe(
        m_regionId,
        m_captureIntervalMs,
        this,
        "onFrameCaptured"
        );
}


void CustomSearcherManager::unsubscribeCapture()
{
    if(!m_captureCoordinator)
        return;

    if(m_regionId < 0)
        return;

    m_captureCoordinator->unsubscribe(
        m_regionId
        );
}


void CustomSearcherManager::onFrameCaptured(
    QImage frame
    )
{
    if(!m_enabled)
        return;

    if(!m_worker)
        return;

    if(frame.isNull())
        return;

    if(!hasActiveTemplates())
        return;

    m_worker->submitFrame(
        frame
        );
}


void CustomSearcherManager::rebuildWorkerTemplates()
{
    if(!m_worker)
        return;

    if(!m_enabled)
    {
        QMetaObject::invokeMethod(
            m_worker,
            [worker = m_worker]()
            {
                worker->clearTemplates();
            },
            Qt::QueuedConnection
            );

        return;
    }

    QVector<CustomSearcherMultiFinder::Template>
        activeTemplates;

    activeTemplates.reserve(
        m_templates.size()
        );

    for(const RuntimeTemplate &runtime : m_templates)
    {
        // Template disabilitato dalla configurazione.
        if(!runtime.config.searchEnabled)
            continue;

        if(runtime.coolingDown)
            continue;

        if(runtime.config.templateImage.isNull())
            continue;

        CustomSearcherMultiFinder::Template templ;

        templ.id =
            runtime.config.id;

        templ.image =
            runtime.config.templateImage;

        templ.pixelTolerance =
            runtime.config.pixelTolerance;

        templ.matchThreshold =
            runtime.config.matchThreshold;

        activeTemplates.append(
            templ
            );
    }

    /*
     * Il worker vive su un thread separato.
     */
    QMetaObject::invokeMethod(
        m_worker,
        [worker = m_worker, activeTemplates]()
        {
            worker->setTemplates(
                activeTemplates
                );
        },
        Qt::QueuedConnection
        );
}


void CustomSearcherManager::onWorkerResults(
    const QVector<CustomSearcherMultiFinder::Result> &results
    )
{
    if(!m_enabled)
        return;

    if(results.isEmpty())
        return;

    for(const CustomSearcherMultiFinder::Result &result : results)
    {
        int index = -1;

        for(int i = 0;
             i < m_templates.size();
             ++i)
        {
            if(m_templates[i].config.id ==
                result.templateId)
            {
                index = i;
                break;
            }
        }

        if(index < 0)
            continue;

        RuntimeTemplate &runtime =
            m_templates[index];

        // Difesa da risultati già in coda nel worker.
        if(!runtime.config.searchEnabled)
            continue;

        if(runtime.coolingDown)
            continue;

        const QRect screenRect =
            result.rect.translated(
                m_searchRegion.topLeft()
                );

        emit templateFound(
            result.templateId,
            screenRect,
            result.score
            );

        startCooldown(
            result.templateId
            );
    }
}


void CustomSearcherManager::startCooldown(
    int templateId
    )
{
    for(RuntimeTemplate &runtime : m_templates)
    {
        if(runtime.config.id != templateId)
            continue;

        if(runtime.coolingDown)
            return;

        runtime.coolingDown = true;

        if(runtime.overlay)
        {
            runtime.overlay->startCooldown(
                runtime.config.cooldownMs
                );
        }

        if(runtime.cooldownTimer)
        {
            runtime.cooldownTimer->stop();
            runtime.cooldownTimer->deleteLater();
            runtime.cooldownTimer = nullptr;
        }

        const int cooldownMs =
            qMax(
                0,
                runtime.config.cooldownMs
                );

        if(cooldownMs <= 0)
        {
            runtime.coolingDown = false;

            if(runtime.overlay)
            {
                runtime.overlay->stopCooldown();
            }

            rebuildWorkerTemplates();

            return;
        }

        QTimer *timer =
            new QTimer(this);

        timer->setSingleShot(
            true
            );

        runtime.cooldownTimer =
            timer;

        connect(
            timer,
            &QTimer::timeout,
            this,
            [this, templateId]()
            {
                onCooldownFinished(
                    templateId
                    );
            }
            );

        timer->start(
            cooldownMs
            );

        /*
         * Il template appena trovato viene escluso
         * dal worker per tutta la durata del cooldown.
         */
        rebuildWorkerTemplates();

        return;
    }
}


void CustomSearcherManager::onCooldownFinished(
    int templateId
    )
{
    if(!m_enabled)
        return;

    for(RuntimeTemplate &runtime : m_templates)
    {
        if(runtime.config.id != templateId)
            continue;

        runtime.coolingDown = false;

        if(runtime.overlay)
        {
            runtime.overlay->stopCooldown();
        }

        if(runtime.cooldownTimer)
        {
            runtime.cooldownTimer->deleteLater();
            runtime.cooldownTimer = nullptr;
        }

        rebuildWorkerTemplates();

        return;
    }
}


bool CustomSearcherManager::hasActiveTemplates() const
{
    if(!m_enabled)
        return false;

    for(const RuntimeTemplate &runtime : m_templates)
    {
        if(!runtime.config.searchEnabled)
            continue;

        if(runtime.coolingDown)
            continue;

        if(runtime.config.templateImage.isNull())
            continue;

        return true;
    }

    return false;
}


void CustomSearcherManager::configure()
{
    if(!m_configWindow)
    {
        m_configWindow =
            new CustomSearcherConfigWindow(
                this
                );

        m_configWindow->setAttribute(
            Qt::WA_DeleteOnClose,
            false
            );
    }

    m_configWindow->refresh();

    m_configWindow->show();
    m_configWindow->raise();
    m_configWindow->activateWindow();
}


void CustomSearcherManager::captureTemplateCrop()
{
    if(m_searchRegion.isNull() ||
        m_searchRegion.isEmpty())
    {
        emit templateCropCanceled();
        return;
    }

    if(m_searchAreaWindow)
    {
        m_searchAreaWindow->hide();
    }

    const QRect region =
        m_searchRegion;

    QTimer::singleShot(
        120,
        this,
        [this, region]()
        {
            QScreen *screen =
                QGuiApplication::primaryScreen();

            if(!screen)
            {
                if(m_searchAreaWindow)
                {
                    m_searchAreaWindow->show();
                    m_searchAreaWindow->raise();
                }

                emit templateCropCanceled();
                return;
            }

            const QImage source =
                screen->grabWindow(
                          0,
                          region.x(),
                          region.y(),
                          region.width(),
                          region.height()
                          )
                    .toImage()
                    .convertToFormat(
                        QImage::Format_ARGB32
                        );

            if(source.isNull())
            {
                if(m_searchAreaWindow)
                {
                    m_searchAreaWindow->show();
                    m_searchAreaWindow->raise();
                }

                emit templateCropCanceled();
                return;
            }

            auto *crop =
                new CustomSearcherTemplateCrop(
                    source,
                    QSize(28, 28)
                    );

            connect(
                crop,
                &CustomSearcherTemplateCrop::accepted,
                this,
                [this, crop](const QImage &image)
                {
                    emit templateCropReady(
                        image
                        );

                    crop->close();
                    crop->deleteLater();

                    if(m_searchAreaWindow)
                    {
                        m_searchAreaWindow->show();
                        m_searchAreaWindow->raise();
                    }

                    if(m_configWindow)
                    {
                        m_configWindow->show();
                        m_configWindow->raise();
                        m_configWindow->activateWindow();
                        m_configWindow->setFocus();
                    }
                }
                );

            connect(
                crop,
                &CustomSearcherTemplateCrop::canceled,
                this,
                [this, crop]()
                {
                    emit templateCropCanceled();

                    crop->close();
                    crop->deleteLater();

                    if(m_searchAreaWindow)
                    {
                        m_searchAreaWindow->show();
                        m_searchAreaWindow->raise();
                    }

                    if(m_configWindow)
                    {
                        m_configWindow->show();
                        m_configWindow->raise();
                        m_configWindow->activateWindow();
                        m_configWindow->setFocus();
                    }
                }
                );

            crop->show();
            crop->raise();
            crop->activateWindow();
            crop->setFocus();
        }
        );
}


void CustomSearcherManager::configureSearchRegion()
{
    QRect initialArea =
        m_searchRegion;

    if(!initialArea.isValid() ||
        initialArea.isEmpty())
    {
        QScreen *screen =
            QGuiApplication::primaryScreen();

        const QRect screenGeometry =
            screen
                ? screen->geometry()
                : QRect(0, 0, 1920, 1080);

        initialArea =
            QRect(
                screenGeometry.center().x() - 150,
                screenGeometry.center().y() - 75,
                300,
                150
                );
    }

    auto *searchArea =
        new CustomSearcherSearchArea(
            initialArea
            );

    m_searchAreaWindow =
        searchArea;

    connect(
        searchArea,
        &CustomSearcherSearchArea::accepted,
        this,
        [this, searchArea](const QRect &area)
        {
            setSearchRegion(area);

            emit searchRegionChanged(
                area
                );

            if(m_searchAreaWindow == searchArea)
            {
                m_searchAreaWindow = nullptr;
            }

            searchArea->deleteLater();

            if(m_configWindow)
            {
                m_configWindow->raise();
                m_configWindow->activateWindow();
                m_configWindow->setFocus();
            }
        }
        );

    connect(
        searchArea,
        &CustomSearcherSearchArea::canceled,
        this,
        [this, searchArea]()
        {
            if(m_searchAreaWindow == searchArea)
            {
                m_searchAreaWindow = nullptr;
            }

            searchArea->deleteLater();

            if(m_configWindow)
            {
                m_configWindow->raise();
                m_configWindow->activateWindow();
                m_configWindow->setFocus();
            }
        }
        );

    searchArea->show();
    searchArea->raise();
    searchArea->activateWindow();
    searchArea->setFocus();
}


void CustomSearcherManager::loadSettings()
{
    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher")
        );

    const int x =
        settings.value(
                    QStringLiteral("SearchRegionX"),
                    700
                    ).toInt();

    const int y =
        settings.value(
                    QStringLiteral("SearchRegionY"),
                    600
                    ).toInt();

    const int width =
        settings.value(
                    QStringLiteral("SearchRegionWidth"),
                    300
                    ).toInt();

    const int height =
        settings.value(
                    QStringLiteral("SearchRegionHeight"),
                    150
                    ).toInt();

    settings.endGroup();

    m_searchRegion =
        QRect(
            x,
            y,
            width,
            height
            ).normalized();
}


void CustomSearcherManager::saveSearchRegion()
{
    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher")
        );

    settings.setValue(
        QStringLiteral("SearchRegionX"),
        m_searchRegion.x()
        );

    settings.setValue(
        QStringLiteral("SearchRegionY"),
        m_searchRegion.y()
        );

    settings.setValue(
        QStringLiteral("SearchRegionWidth"),
        m_searchRegion.width()
        );

    settings.setValue(
        QStringLiteral("SearchRegionHeight"),
        m_searchRegion.height()
        );

    settings.endGroup();

    settings.sync();
}


void CustomSearcherManager::saveTemplates()
{
    const QString customSearcherPath =
        templateImagesPath();

    QDir directory;

    if(!directory.mkpath(customSearcherPath))
        return;

    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher")
        );

    settings.beginWriteArray(
        QStringLiteral("Templates")
        );

    for(int i = 0;
         i < m_templates.size();
         ++i)
    {
        const TemplateConfig &config =
            m_templates[i].config;

        settings.setArrayIndex(
            i
            );

        const QString displayFileName =
            QStringLiteral(
                "template_%1_display.png"
                ).arg(config.id);

        const QString templateFileName =
            QStringLiteral(
                "template_%1_match.png"
                ).arg(config.id);

        const QString displayPath =
            QDir(customSearcherPath)
                .filePath(
                    displayFileName
                    );

        const QString templatePath =
            QDir(customSearcherPath)
                .filePath(
                    templateFileName
                    );

        if(!config.displayImage.isNull())
        {
            config.displayImage
                .convertToFormat(
                    QImage::Format_ARGB32
                    )
                .save(
                    displayPath,
                    "PNG"
                    );
        }

        if(!config.templateImage.isNull())
        {
            config.templateImage
                .convertToFormat(
                    QImage::Format_ARGB32
                    )
                .save(
                    templatePath,
                    "PNG"
                    );
        }

        settings.setValue(
            QStringLiteral("Id"),
            config.id
            );

        settings.setValue(
            QStringLiteral("DisplayImage"),
            displayFileName
            );

        settings.setValue(
            QStringLiteral("TemplateImage"),
            config.templateImage.isNull()
                ? QString()
                : templateFileName
            );

        settings.setValue(
            QStringLiteral("CooldownMs"),
            config.cooldownMs
            );

        // Stato della checkbox: template attivo/disattivo.
        settings.setValue(
            QStringLiteral("SearchEnabled"),
            config.searchEnabled
            );

        settings.setValue(
            QStringLiteral("OverlayX"),
            config.overlayRect.x()
            );

        settings.setValue(
            QStringLiteral("OverlayY"),
            config.overlayRect.y()
            );

        settings.setValue(
            QStringLiteral("OverlayWidth"),
            config.overlayRect.width()
            );

        settings.setValue(
            QStringLiteral("OverlayHeight"),
            config.overlayRect.height()
            );
    }

    settings.endArray();
    settings.endGroup();

    settings.sync();

    cleanupTemplateFiles();
}


void CustomSearcherManager::loadTemplates()
{
    m_templates.clear();

    const QString basePath =
        templateImagesPath();

    QDir directory(
        basePath
        );

    if(!directory.exists())
        return;

    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher")
        );

    const int count =
        settings.beginReadArray(
            QStringLiteral("Templates")
            );

    for(int i = 0;
         i < count;
         ++i)
    {
        settings.setArrayIndex(
            i
            );

        TemplateConfig config;

        config.id =
            settings.value(
                        QStringLiteral("Id"),
                        -1
                        ).toInt();

        const QString displayFileName =
            settings.value(
                        QStringLiteral("DisplayImage")
                        ).toString();

        const QString templateFileName =
            settings.value(
                        QStringLiteral("TemplateImage")
                        ).toString();

        config.cooldownMs =
            qMax(
                0,
                settings.value(
                            QStringLiteral("CooldownMs"),
                            0
                            ).toInt()
                );

        // Se la chiave non esiste nelle vecchie configurazioni,
        // il template rimane abilitato.
        config.searchEnabled =
            settings.value(
                        QStringLiteral("SearchEnabled"),
                        true
                        ).toBool();

        const int overlayX =
            settings.value(
                        QStringLiteral("OverlayX"),
                        100
                        ).toInt();

        const int overlayY =
            settings.value(
                        QStringLiteral("OverlayY"),
                        100
                        ).toInt();

        const int overlayWidth =
            settings.value(
                        QStringLiteral("OverlayWidth"),
                        0
                        ).toInt();

        const int overlayHeight =
            settings.value(
                        QStringLiteral("OverlayHeight"),
                        0
                        ).toInt();

        config.overlayRect =
            QRect(
                overlayX,
                overlayY,
                overlayWidth,
                overlayHeight
                );

        config.pixelTolerance = 7;
        config.matchThreshold = 97.5;

        if(!displayFileName.isEmpty())
        {
            const QString displayPath =
                directory.filePath(
                    displayFileName
                    );

            QImage image(
                displayPath
                );

            if(!image.isNull())
            {
                config.displayImage =
                    image.convertToFormat(
                        QImage::Format_ARGB32
                        );
            }
        }

        if(config.displayImage.isNull())
            continue;

        if(!templateFileName.isEmpty())
        {
            const QString templatePath =
                directory.filePath(
                    templateFileName
                    );

            QImage image(
                templatePath
                );

            if(!image.isNull())
            {
                config.templateImage =
                    image.convertToFormat(
                        QImage::Format_ARGB32
                        );
            }
        }

        RuntimeTemplate runtime;

        runtime.config =
            config;

        m_templates.append(
            runtime
            );
    }

    settings.endArray();
    settings.endGroup();
}


void CustomSearcherManager::cleanupTemplateFiles()
{
    const QString basePath =
        templateImagesPath();

    QDir directory(
        basePath
        );

    if(!directory.exists())
        return;

    QSet<QString> usedFiles;

    for(const RuntimeTemplate &runtime :
         m_templates)
    {
        const int id =
            runtime.config.id;

        if(id < 0)
            continue;

        usedFiles.insert(
            QStringLiteral(
                "template_%1_display.png"
                ).arg(id)
            );

        if(!runtime.config.templateImage.isNull())
        {
            usedFiles.insert(
                QStringLiteral(
                    "template_%1_match.png"
                    ).arg(id)
                );
        }
    }

    const QStringList files =
        directory.entryList(
            QStringList()
                << QStringLiteral(
                       "template_*_display.png"
                       )
                << QStringLiteral(
                       "template_*_match.png"
                       ),
            QDir::Files
            );

    for(const QString &fileName :
         files)
    {
        if(usedFiles.contains(fileName))
            continue;

        directory.remove(
            fileName
            );
    }
}


QString CustomSearcherManager::templateImagesPath() const
{
    const QString imagesPath =
        QDir(
            QCoreApplication::applicationDirPath()
            ).filePath(
                QStringLiteral("images")
                );

    const QString customSearcherPath =
        QDir(
            imagesPath
            ).filePath(
                QStringLiteral("custom_searcher")
                );

    return customSearcherPath;
}


void CustomSearcherManager::onOverlayGeometryChanged(
    int templateId,
    const QRect &rect
    )
{
    for(RuntimeTemplate &runtime :
         m_templates)
    {
        if(runtime.config.id != templateId)
            continue;

        runtime.config.overlayRect =
            rect;

        saveOverlayGeometry(
            templateId,
            rect
            );

        return;
    }
}


void CustomSearcherManager::saveOverlayGeometry(
    int templateId,
    const QRect &rect
    )
{
    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher")
        );

    const int size =
        settings.beginReadArray(
            QStringLiteral("Templates")
            );

    for(int i = 0;
         i < size;
         ++i)
    {
        settings.setArrayIndex(
            i
            );

        if(settings.value(
                        QStringLiteral("Id")
                        ).toInt() != templateId)
        {
            continue;
        }

        settings.setValue(
            QStringLiteral("OverlayX"),
            rect.x()
            );

        settings.setValue(
            QStringLiteral("OverlayY"),
            rect.y()
            );

        settings.setValue(
            QStringLiteral("OverlayWidth"),
            rect.width()
            );

        settings.setValue(
            QStringLiteral("OverlayHeight"),
            rect.height()
            );

        break;
    }

    settings.endArray();
    settings.endGroup();

    settings.sync();
}


void CustomSearcherManager::createOverlay(
    RuntimeTemplate &runtime
    )
{
    // Se il template è disabilitato, non creare l'overlay.
    if(!runtime.config.searchEnabled)
        return;

    if(runtime.overlay)
        return;

    runtime.overlay =
        new CustomSearcherOverlay(
            runtime.config.id,
            runtime.config.displayImage
            );

    if(m_overlayRoot)
    {
        m_overlayRoot->registerCustomSearcherOverlay(
            runtime.overlay
            );
    }

    connect(
        runtime.overlay,
        &CustomSearcherOverlay::geometryChangeFinished,
        this,
        &CustomSearcherManager::onOverlayGeometryChanged
        );

    QRect rect =
        runtime.config.overlayRect;

    if(rect.width() <= 0 ||
        rect.height() <= 0)
    {
        const QSize imageSize =
            runtime.config.displayImage.size();

        if(imageSize.isValid())
        {
            rect =
                QRect(
                    100,
                    100,
                    imageSize.width(),
                    imageSize.height()
                    );

            runtime.config.overlayRect =
                rect;
        }
    }

    runtime.overlay->setScreenRect(
        rect
        );

    runtime.overlay->setVisibleBySearcher(
        m_enabled
        );
}


void CustomSearcherManager::destroyOverlays()
{
    for(RuntimeTemplate &runtime :
         m_templates)
    {
        if(!runtime.overlay)
            continue;

        if(m_overlayRoot)
        {
            m_overlayRoot->unregisterCustomSearcherOverlay(
                runtime.overlay
                );
        }

        runtime.overlay->deleteLater();
        runtime.overlay = nullptr;
    }
}