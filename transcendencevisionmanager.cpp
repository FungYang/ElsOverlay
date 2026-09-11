#include "transcendencevisionmanager.h"
#include "transcendencevisionworker.h"
#include "capturecoordinator.h"
#include "transcendencevisionconfig.h"
#include "transcendencecapturesetup.h"
#include "transcendenceprecisioncrop.h"
#include "globalkeyboard.h"
#include "overlayroot.h"
#include "overlay.h"

#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include <QKeyEvent>

TranscendenceVisionManager::TranscendenceVisionManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRootPtr,
    Overlay *overlay,
    QObject *parent
    )
    : QObject(parent),
    keyboard(keyboard),
    overlayRoot(overlayRootPtr),
    overlay(overlay)
{
    // NUOVO: worker + thread dedicati al calcolo pesante.
    m_worker = new TranscendenceVisionWorker(); // niente parent: deve poter cambiare thread
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);
    m_workerThread->start();

    connect(m_worker, &TranscendenceVisionWorker::scanResult,
            this, &TranscendenceVisionManager::onScanResult); // cross-thread, auto-queued

    loadSettings();
    loadIcon();

    m_delayTimer.setSingleShot(true);
    m_delayTimer.setInterval(TranscendenceVisionConfig::DELAY_MS);

    connect(
        &m_delayTimer,
        &QTimer::timeout,
        this,
        &TranscendenceVisionManager::startScanning
        );

    // RIMOSSO: connect(&m_scanTimer, &QTimer::timeout, this, &scanTick) — non esiste più.

    if (keyboard)
    {
        // P apre il ritaglio preciso. (INVARIATO)
        connect(
            keyboard,
            &GlobalKeyboard::keyPressed,
            this,
            [this](int key)
            {
                if (precisionCrop)
                {
                    if (key == Qt::Key_Left)
                    {
                        precisionCrop->moveLeft();
                        return;
                    }

                    if (key == Qt::Key_Right)
                    {
                        precisionCrop->moveRight();
                        return;
                    }

                    if (key == Qt::Key_Up)
                    {
                        precisionCrop->moveUp();
                        return;
                    }

                    if (key == Qt::Key_Down)
                    {
                        precisionCrop->moveDown();
                        return;
                    }

                    if (key == Qt::Key_Escape)
                    {
                        closePrecisionCrop();
                        return;
                    }

                    if (key == Qt::Key_Return || key == Qt::Key_Enter)
                    {
                        precisionCrop->setFocus();
                        QKeyEvent event(
                            QEvent::KeyPress,
                            Qt::Key_Return,
                            Qt::NoModifier
                            );
                        QCoreApplication::sendEvent(
                            precisionCrop,
                            &event
                            );
                        return;
                    }

                    if (key == 'P')
                        return;
                }

                if (key != 'P')
                    return;

                if (!captureSetup)
                    return;

                if (!captureSetup->isVisible())
                    return;

                openPrecisionCrop();
            }
            );

        // INVIO - CONFERMA CONFIGURAZIONE. (INVARIATO)
        connect(
            keyboard,
            &GlobalKeyboard::confirmPressed,
            this,
            [this]()
            {
                if (precisionCrop)
                {
                    precisionCrop->setFocus();

                    QKeyEvent event(
                        QEvent::KeyPress,
                        Qt::Key_Return,
                        Qt::NoModifier
                        );

                    QCoreApplication::sendEvent(
                        precisionCrop,
                        &event
                        );

                    return;
                }

                if (!captureSetup)
                    return;

                if (!captureSetup->isVisible())
                    return;

                const QRect newArea =
                    captureSetup->searchArea();

                if (newArea.isNull() || newArea.isEmpty())
                    return;

                m_searchArea = newArea;

                saveSettings();

                unregisterSearchRegion();
                registerSearchRegion();

                loadIcon();

                qDebug()
                    << "TRANSCENDENCE:"
                    << "search area aggiornata:"
                    << m_searchArea
                    << "regionId ="
                    << m_searchRegionId;

                captureSetup->hide();

                if (overlayRoot)
                    overlayRoot->raiseAll();
            }
            );
    }
}

TranscendenceVisionManager::~TranscendenceVisionManager()
{
    closePrecisionCrop();
    stopAll();

    unregisterSearchRegion();

    m_workerThread->quit();
    m_workerThread->wait();
    delete m_worker;
}

void TranscendenceVisionManager::registerSearchRegion()
{
    if (m_searchArea.isValid() && !m_searchArea.isEmpty())
        m_searchRegionId = CaptureCoordinator::instance()->registerRegion(m_searchArea);
}

void TranscendenceVisionManager::unregisterSearchRegion()
{
    if (m_searchRegionId >= 0)
    {
        CaptureCoordinator::instance()->unregisterRegion(m_searchRegionId);
        m_searchRegionId = -1;
    }
}

void TranscendenceVisionManager::loadSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    m_searchArea = QRect(
        settings.value("Transcendence/AreaX", 700).toInt(),
        settings.value("Transcendence/AreaY", 600).toInt(),
        settings.value("Transcendence/AreaW", 300).toInt(),
        settings.value("Transcendence/AreaH", 150).toInt()
        );

    m_iconWidth =
        settings.value(
                    "Transcendence/IconWidth",
                    TranscendenceVisionConfig::ICON_WIDTH
                    ).toInt();

    m_iconHeight =
        settings.value(
                    "Transcendence/IconHeight",
                    TranscendenceVisionConfig::ICON_HEIGHT
                    ).toInt();

    if (m_iconWidth < 1)
        m_iconWidth = TranscendenceVisionConfig::ICON_WIDTH;

    if (m_iconHeight < 1)
        m_iconHeight = TranscendenceVisionConfig::ICON_HEIGHT;

    unregisterSearchRegion();
    registerSearchRegion();
}

void TranscendenceVisionManager::saveSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    settings.setValue("Transcendence/AreaX", m_searchArea.x());
    settings.setValue("Transcendence/AreaY", m_searchArea.y());
    settings.setValue("Transcendence/AreaW", m_searchArea.width());
    settings.setValue("Transcendence/AreaH", m_searchArea.height());

    settings.setValue("Transcendence/IconWidth", m_iconWidth);
    settings.setValue("Transcendence/IconHeight", m_iconHeight);

    settings.sync();
}

void TranscendenceVisionManager::loadIcon()
{
    const QString path =
        QCoreApplication::applicationDirPath() +
        "/images/transcendence_search.png";

    m_templateIcon.load(path);

    if (!m_templateIcon.isNull())
    {
        m_templateIcon =
            m_templateIcon.convertToFormat(
                QImage::Format_ARGB32
                );
    }
    else
    {
        qDebug()
        << "TRANSCENDENCE: impossibile caricare reference:"
        << path;
    }

    m_configured =
        !m_templateIcon.isNull() &&
        m_templateIcon.width() == m_iconWidth &&
        m_templateIcon.height() == m_iconHeight &&
        m_searchArea.isValid() &&
        !m_searchArea.isEmpty() &&
        m_searchRegionId >= 0;

    // NUOVO: propaga il template al worker.
    QMetaObject::invokeMethod(
        m_worker, "setTemplate", Qt::QueuedConnection,
        Q_ARG(QImage, m_templateIcon),
        Q_ARG(int, m_iconWidth),
        Q_ARG(int, m_iconHeight)
        );
}

void TranscendenceVisionManager::configure()
{
    if (!overlayRoot)
        return;

    if (!captureSetup)
    {
        captureSetup =
            new TranscendenceCaptureSetup(overlayRoot);

        overlayRoot->registerOverlay(captureSetup);
    }

    captureSetup->setSearchArea(m_searchArea);
    captureSetup->show();
    captureSetup->raise();
    captureSetup->activateWindow();
    captureSetup->setFocus();

    overlayRoot->raiseAll();
}

void TranscendenceVisionManager::openPrecisionCrop()
{
    if (!captureSetup)
        return;

    if (precisionCrop)
    {
        precisionCrop->raise();
        precisionCrop->activateWindow();
        precisionCrop->setFocus();
        return;
    }

    const QRect bigRect =
        captureSetup->iconRect();

    if (bigRect.width() != TranscendenceVisionConfig::ICON_BOX_WIDTH ||
        bigRect.height() != TranscendenceVisionConfig::ICON_BOX_HEIGHT)
    {
        captureSetup->showFeedback(
            "ERRORE: riquadro giallo non valido"
            );
        return;
    }

    captureSetup->hide();

    // NOTA: questa cattura per l'anteprima interattiva del crop resta
    // occasionale (un singolo screenshot su richiesta esplicita utente,
    // non un polling continuo), quindi va bene lasciarla com'era:
    // usa direttamente lo screen grab "reliable", non il coordinator,
    // perché è un'operazione una tantum e non compete con lo scan ciclico.
    QTimer::singleShot(
        120,
        this,
        [this, bigRect]()
        {
            if (!captureSetup)
                return;

            QScreen *screen =
                QGuiApplication::primaryScreen();

            if (!screen)
            {
                captureSetup->show();
                captureSetup->showFeedback(
                    "ERRORE: schermo non disponibile"
                    );
                return;
            }

            const QImage source =
                screen->grabWindow(
                          0,
                          bigRect.x(),
                          bigRect.y(),
                          bigRect.width(),
                          bigRect.height()
                          ).toImage().convertToFormat(QImage::Format_ARGB32);

            if (source.isNull())
            {
                captureSetup->show();
                captureSetup->raise();
                captureSetup->activateWindow();
                captureSetup->setFocus();

                captureSetup->showFeedback(
                    "ERRORE: cattura fallita"
                    );

                return;
            }

            precisionCrop =
                new TranscendencePrecisionCrop(
                    source,
                    QSize(m_iconWidth, m_iconHeight)
                    );

            connect(
                precisionCrop,
                &TranscendencePrecisionCrop::accepted,
                this,
                [this](const QImage &image, const QSize &size)
                {
                    savePreciseIcon(image, size);
                }
                );

            connect(
                precisionCrop,
                &TranscendencePrecisionCrop::canceled,
                this,
                [this]()
                {
                    closePrecisionCrop();

                    if (captureSetup)
                    {
                        captureSetup->show();
                        captureSetup->raise();
                        captureSetup->activateWindow();
                        captureSetup->setFocus();
                    }
                }
                );

            connect(
                precisionCrop,
                &QObject::destroyed,
                this,
                [this]()
                {
                    precisionCrop = nullptr;
                }
                );

            precisionCrop->show();
            precisionCrop->raise();
            precisionCrop->activateWindow();
            precisionCrop->setFocus();
        }
        );
}

void TranscendenceVisionManager::savePreciseIcon(
    const QImage &icon,
    const QSize &size
    )
{
    if (icon.isNull())
        return;

    if (size.width() <= 0 || size.height() <= 0)
        return;

    if (icon.size() != size)
    {
        qDebug()
        << "TRANSCENDENCE: dimensione crop incoerente:"
        << "image =" << icon.size()
        << "size =" << size;

        return;
    }

    m_iconWidth = size.width();
    m_iconHeight = size.height();

    saveSettings();

    QDir dir(
        QCoreApplication::applicationDirPath() + "/images"
        );

    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            if (captureSetup)
                captureSetup->showFeedback(
                    "ERRORE: impossibile creare images"
                    );

            return;
        }
    }

    const QString path =
        QCoreApplication::applicationDirPath() +
        "/images/transcendence_search.png";

    if (!icon.save(path))
    {
        if (captureSetup)
            captureSetup->showFeedback(
                "ERRORE: impossibile salvare icona"
                );

        return;
    }

    loadIcon();

    closePrecisionCrop();

    if (captureSetup)
    {
        captureSetup->show();
        captureSetup->raise();
        captureSetup->activateWindow();
        captureSetup->setFocus();
        captureSetup->showFeedback(
            QString("ICONA %1x%2 SALVATA")
                .arg(m_iconWidth)
                .arg(m_iconHeight)
            );
    }
}

void TranscendenceVisionManager::closePrecisionCrop()
{
    if (!precisionCrop)
        return;

    precisionCrop->close();
    precisionCrop->deleteLater();
    precisionCrop = nullptr;
}

void TranscendenceVisionManager::saveCurrentIcon()
{
    openPrecisionCrop();
}

void TranscendenceVisionManager::setEnabled(bool enabled)
{
    m_enabled = enabled;

    if (!enabled)
    {
        stopAll();

        if (overlay)
            overlay->setEnabled(false);

        return;
    }

    if (overlay)
        overlay->setEnabled(true);
}

void TranscendenceVisionManager::stopAll()
{
    m_delayTimer.stop();
    stopScanning();
}

void TranscendenceVisionManager::onCooldownStarted()
{
    if (!m_enabled || !m_configured)
        return;

    stopScanning();
    m_delayTimer.stop();
    m_delayTimer.start();
}

void TranscendenceVisionManager::onCooldownReset()
{
    stopAll();
}

void TranscendenceVisionManager::startScanning()
{
    if (!m_enabled ||
        !m_configured ||
        m_searchRegionId < 0)
    {
        return;
    }

    CaptureCoordinator::instance()->subscribe(
        m_searchRegionId,
        TranscendenceVisionConfig::SCAN_INTERVAL_MS,
        this,
        "onFrameReady"
        );
}

void TranscendenceVisionManager::stopScanning()
{
    if (m_searchRegionId >= 0)
        CaptureCoordinator::instance()->unsubscribe(m_searchRegionId);
}

// SOSTITUISCE la vecchia scanTick(). Gira sul thread GUI (consegna via
// Qt::QueuedConnection dal CaptureCoordinator), ma qui non c'è più
// nessun calcolo pesante: si limita a inoltrare il frame al worker.
void TranscendenceVisionManager::onFrameReady(QImage area)
{
    if (!m_enabled || !m_configured || area.isNull())
        return;

    QMetaObject::invokeMethod(
        m_worker, "processFrame", Qt::QueuedConnection,
        Q_ARG(QImage, area)
        );
}

// NUOVO: riceve il risultato dal worker (cross-thread, auto-queued sul
// thread GUI). Qui è sicuro toccare overlay/widget.
void TranscendenceVisionManager::onScanResult(bool found, QRect foundRect, double score, QImage area)
{
#ifdef QT_DEBUG
    qDebug()
        << "TRANSCENDENCE: score ="
        << score
        << "soglia ="
        << TranscendenceVisionConfig::MATCH_THRESHOLD
        << "found ="
        << found;

    {
        static QElapsedTimer dumpTimer;

        const bool shouldDump =
            found ||
            !dumpTimer.isValid() ||
            dumpTimer.elapsed() >= 1000;

        if (shouldDump)
        {
            const QString debugDir =
                QCoreApplication::applicationDirPath() +
                "/images/debug";

            QDir().mkpath(debugDir);

            const QString areaSuffix =
                found ? "_found" : "";

            area.save(
                debugDir +
                "/last_captured_area" +
                areaSuffix +
                ".png"
                );

            if (!foundRect.isNull() &&
                foundRect.width() == m_templateIcon.width() &&
                foundRect.height() == m_templateIcon.height())
            {
                const QImage candidate =
                    area.copy(foundRect);

                candidate.save(
                    debugDir +
                    "/last_best_candidate" +
                    areaSuffix +
                    ".png"
                    );

                // ... il resto del blocco diagnostico bordo/interno
                // (media R/G/B, max, ecc.) resta identico a quello che
                // mi avevi mandato: copialo qui invariato, usa 'area',
                // 'foundRect' e m_templateIcon come prima.
            }

            dumpTimer.restart();
        }
    }
#endif

    if (!found)
        return;

    stopScanning();

    if (overlay)
        overlay->restartCooldown();

    m_delayTimer.stop();
    m_delayTimer.start();
}