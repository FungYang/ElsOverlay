#include "atmazonemanager.h"
#include "atmazonecapturesetup.h"
#include "atmazoneworker.h"
#include "atmaredzoneproxy.h"
#include "globalkeyboard.h"
#include "overlayroot.h"
#include "buffvisioncore.h"
#include "capturecoordinator.h"

#ifdef QT_DEBUG
#include "atmadebugwindow.h"
#endif

#include <QCoreApplication>
#include <QMessageBox>
#include <QHash>


AtmaZoneManager::AtmaZoneManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRoot,
    BuffVisionCore *core,
    QObject *parent
    )
    : QObject(parent),
    m_keyboard(keyboard),
    m_overlayRoot(overlayRoot),
    m_core(core)
{
    m_redRegionIds.fill(-1);
    m_redStates.fill(ZoneState::Unknown);


    // =========================
    // WORKER (confronto pixel su thread dedicato)
    // =========================

    m_worker = new AtmaZoneWorker();
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);

    connect(
        m_worker,
        &AtmaZoneWorker::referencesLoaded,
        this,
        &AtmaZoneManager::onReferencesLoaded
        );

    connect(
        m_worker,
        &AtmaZoneWorker::redCompared,
        this,
        &AtmaZoneManager::onRedZoneCompared
        );

#ifdef QT_DEBUG
    connect(
        m_worker,
        &AtmaZoneWorker::redDebugFrame,
        this,
        &AtmaZoneManager::onRedDebugFrame
        );
#endif

    m_workerThread->start();


    // =========================
    // PROXY — zone rosse
    // =========================

    for (int i = 0; i < RED_COUNT; ++i)
    {
        m_redProxies[i] =
            std::make_unique<AtmaRedZoneProxy>(i, this);

        connect(
            m_redProxies[i].get(),
            &AtmaRedZoneProxy::forwardedFrame,
            this,
            [this](
                int index,
                quint64 frameId,
                QImage frame
                )
            {
                QMetaObject::invokeMethod(
                    m_worker,
                    "compareRedFrame",
                    Qt::QueuedConnection,
                    Q_ARG(int, index),
                    Q_ARG(quint64, frameId),
                    Q_ARG(QImage, frame)
                    );
            }
            );
    }


#ifdef QT_DEBUG
    m_debugWindow = new AtmaDebugWindow();
#endif


    // =========================
    // TASTO P — cattura riferimenti
    // =========================

    connect(
        m_keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {
            if (key != 'P')
                return;

            if (!m_captureSetup ||
                !m_captureSetup->isVisible())
            {
                return;
            }

            m_captureSetup->captureAllReferences();
        }
        );


    // =========================
    // ENTER — conferma configurazione
    // =========================

    connect(
        m_keyboard,
        &GlobalKeyboard::confirmPressed,
        this,
        [this]()
        {
            if (!m_captureSetup ||
                !m_captureSetup->isVisible())
            {
                return;
            }

            m_captureSetup->saveSettings();
            m_captureSetup->hide();

            if (m_overlayRoot)
                m_overlayRoot->raiseAll();
        }
        );
}


AtmaZoneManager::~AtmaZoneManager()
{
    unsubscribeAll();
    unregisterAllRegions();

    m_workerThread->quit();
    m_workerThread->wait();

    delete m_worker;

#ifdef QT_DEBUG
    delete m_debugWindow;
#endif
}


// ============================================================
// CONFIGURE
// ============================================================

void AtmaZoneManager::configure()
{
    if (!m_overlayRoot)
        return;

    if (!m_captureSetup)
    {
        m_captureSetup =
            new AtmaZoneCaptureSetup(nullptr);

        m_overlayRoot->registerOverlay(
            m_captureSetup
            );
    }

    m_captureSetup->loadSettings();
    m_captureSetup->show();
    m_captureSetup->raise();
    m_captureSetup->activateWindow();
    m_captureSetup->setFocus();

    m_overlayRoot->raiseAll();

#ifdef QT_DEBUG
    if (m_debugWindow)
    {
        m_debugWindow->show();
        m_debugWindow->raise();
    }
#endif
}


// ============================================================
// ENABLED
// ============================================================

void AtmaZoneManager::setEnabled(bool value)
{
    m_enabled = value;

    if (!m_enabled)
    {
        unsubscribeAll();
        unregisterAllRegions();

        m_redStates.fill(ZoneState::Unknown);
        m_pendingFrames.clear();

        return;
    }

    if (!AtmaZoneCaptureSetup::referencesExist())
    {
        QMessageBox::warning(
            nullptr,
            "Atma Zones",
            "Le 6 zone rosse non sono state configurate completamente. "
            "Disattiva Atma e completa la configurazione."
            );

        m_enabled = false;
        return;
    }

    m_redStates.fill(ZoneState::Unknown);
    m_pendingFrames.clear();

    const QString imagesDir =
        QCoreApplication::applicationDirPath() + "/images/";

    QMetaObject::invokeMethod(
        m_worker,
        "loadReferences",
        Qt::QueuedConnection,
        Q_ARG(QString, imagesDir)
        );
}


void AtmaZoneManager::onReferencesLoaded(bool ok)
{
    if (!m_enabled)
        return;

    if (!ok)
    {
        QMessageBox::warning(
            nullptr,
            "Atma Zones",
            "Errore nel caricamento dei riferimenti salvati. "
            "Riprova a configurare le zone."
            );

        m_enabled = false;
        return;
    }

    m_configured = true;

    registerAllRegions();
    subscribeAll();
}


// ============================================================
// RED ZONE COMPARED
// ============================================================

void AtmaZoneManager::onRedZoneCompared(
    int index,
    quint64 frameId,
    bool isMatch
    )
{
    if (!m_enabled || !m_configured)
        return;

    if (m_detectionSuspended)
        return;

    if (index < 0 || index >= RED_COUNT)
        return;

    PendingFrame &frame =
        m_pendingFrames[frameId];

    frame.redMatch[index] = isMatch;
    frame.redReceived[index] = true;

    evaluateFrame(frameId);
}


// ============================================================
// GATE FRAME
// ============================================================

void AtmaZoneManager::setGateFrame(
    quint64 frameId,
    bool isMatch
    )
{
    if (!m_enabled || !m_configured)
        return;

    if (m_detectionSuspended)
        return;

    PendingFrame &frame =
        m_pendingFrames[frameId];

    frame.blueMatch = isMatch;
    frame.blueReceived = true;

    evaluateFrame(frameId);
}


// ============================================================
// FRAME EVALUATION
// ============================================================

void AtmaZoneManager::evaluateFrame(quint64 frameId)
{
    auto it =
        m_pendingFrames.find(frameId);

    if (it == m_pendingFrames.end())
        return;

    PendingFrame &frame =
        it.value();

    if (!frame.blueReceived)
        return;

    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (!frame.redReceived[i])
            return;
    }

    // ---------------------------------------------------------
    // Frame completo:
    //
    // Blue + tutte le 6 zone rosse appartengono allo stesso
    // frame di acquisizione.
    // ---------------------------------------------------------

    for (int i = 0; i < RED_COUNT; ++i)
    {
        const ZoneState oldState =
            m_redStates[i];

        const ZoneState newState =
            frame.redMatch[i]
                ? ZoneState::Match
                : ZoneState::Mismatch;

        m_redStates[i] = newState;

        if (oldState == ZoneState::Match &&
            newState == ZoneState::Mismatch &&
            frame.blueMatch &&
            m_core)
        {
            m_core->registerAction();
        }
    }

    m_pendingFrames.erase(it);
}


// ============================================================
// REGISTER / SUBSCRIBE
// ============================================================

void AtmaZoneManager::registerAllRegions()
{
    unregisterAllRegions();

    if (!m_captureSetup)
    {
        m_captureSetup =
            new AtmaZoneCaptureSetup(nullptr);

        m_captureSetup->loadSettings();
        m_captureSetup->hide();
    }

    for (int i = 0; i < RED_COUNT; ++i)
    {
        const QRect rect =
            m_captureSetup->redZoneRect(i);

        m_redRegionIds[i] =
            CaptureCoordinator::instance()->registerRegion(
                rect
                );
    }
}


void AtmaZoneManager::unregisterAllRegions()
{
    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (m_redRegionIds[i] >= 0)
        {
            CaptureCoordinator::instance()->unregisterRegion(
                m_redRegionIds[i]
                );

            m_redRegionIds[i] = -1;
        }
    }
}


void AtmaZoneManager::subscribeAll()
{
    constexpr int INTERVAL_MS = 150;

    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (m_redRegionIds[i] < 0)
            continue;

        CaptureCoordinator::instance()->subscribe(
            m_redRegionIds[i],
            INTERVAL_MS,
            m_redProxies[i].get(),
            "frameReady"
            );
    }
}


void AtmaZoneManager::unsubscribeAll()
{
    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (m_redRegionIds[i] >= 0)
        {
            CaptureCoordinator::instance()->unsubscribe(
                m_redRegionIds[i]
                );
        }
    }
}


// ============================================================
// DETECTION SUSPENDED
// ============================================================

void AtmaZoneManager::setDetectionSuspended(bool on)
{
    if (m_detectionSuspended == on)
        return;

    m_detectionSuspended = on;

    // Sia all'inizio che alla fine: butto via tutti i frame
    // parzialmente raccolti e riparto da stato sconosciuto,
    // così il primo confronto dopo il rilascio si limita
    // a riallinearsi senza generare eventi.

    m_pendingFrames.clear();
    m_redStates.fill(ZoneState::Unknown);
}


#ifdef QT_DEBUG

void AtmaZoneManager::onRedDebugFrame(
    int index,
    quint64 frameId,
    QImage frame,
    bool isMatch
    )
{
    qDebug()
    << "ATMA RED"
    << "index =" << index
    << "frameId =" << frameId
    << "match =" << isMatch;

    if (m_debugWindow)
    {
        m_debugWindow->updateRed(
            index,
            frame,
            isMatch
            );
    }
}


void AtmaZoneManager::updateBlueDebug(
    QImage frame,
    bool isMatch
    )
{
    if (m_debugWindow)
    {
        m_debugWindow->updateBlue(
            frame,
            isMatch
            );
    }
}

#endif