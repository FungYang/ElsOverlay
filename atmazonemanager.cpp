#include "atmazonemanager.h"
#include "atmazonecapturesetup.h"
#include "atmazoneworker.h"
#include "atmaredzoneproxy.h"
#include "globalkeyboard.h"
#include "overlayroot.h"
#include "buffvisioncore.h"
#include "capturecoordinator.h"

#include <QCoreApplication>
#include <QMessageBox>
#ifdef QT_DEBUG
#include "atmadebugwindow.h"
#endif


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
    m_worker = new AtmaZoneWorker(); // niente parent: deve poter cambiare thread
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);

    connect(m_worker, &AtmaZoneWorker::referencesLoaded,
            this, &AtmaZoneManager::onReferencesLoaded);
    connect(m_worker, &AtmaZoneWorker::redCompared,
            this, &AtmaZoneManager::onRedZoneCompared);
    connect(m_worker, &AtmaZoneWorker::blueCompared,
            this, &AtmaZoneManager::onBlueZoneCompared);
    m_gateTimer.setSingleShot(true);
    connect( &m_gateTimer, &QTimer::timeout, this, &AtmaZoneManager::processPendingRedEvents );

#ifdef QT_DEBUG
    connect(m_worker, &AtmaZoneWorker::redDebugFrame,
            this, &AtmaZoneManager::onRedDebugFrame);

    connect(m_worker, &AtmaZoneWorker::blueDebugFrame,
            this, &AtmaZoneManager::onBlueDebugFrame);
#endif

    m_workerThread->start();

    // =========================
    // PROXY — zone rosse
    // =========================
    for (int i = 0; i < RED_COUNT; ++i)
    {
        m_redProxies[i] = std::make_unique<AtmaRedZoneProxy>(i, this);

        connect(m_redProxies[i].get(), &AtmaRedZoneProxy::forwardedFrame,
                this, [this](int index, QImage frame)
                {
                    QMetaObject::invokeMethod(
                        m_worker, "compareRedFrame", Qt::QueuedConnection,
                        Q_ARG(int, index), Q_ARG(QImage, frame)
                        );
                });
    }

    // =========================
    // PROXY — zona blu (indice -1, non usato)
    // =========================
    m_blueProxy = std::make_unique<AtmaRedZoneProxy>(-1, this);

    connect(m_blueProxy.get(), &AtmaRedZoneProxy::forwardedFrame,
            this, [this](int /*unused*/, QImage frame)
            {
                QMetaObject::invokeMethod(
                    m_worker, "compareBlueFrame", Qt::QueuedConnection,
                    Q_ARG(QImage, frame)
                    );
            });

    // =========================
    // TASTO P — cattura riferimenti
    // =========================
    connect(
        m_keyboard, &GlobalKeyboard::keyPressed, this,
        [this](int key)
        {
            if (key != 'P') return;
            if (!m_captureSetup) return;
            if (!m_captureSetup->isVisible()) return;

            m_captureSetup->captureAllReferences();
        }
        );

    // =========================
    // ENTER — conferma configurazione
    // =========================
    connect(
        m_keyboard, &GlobalKeyboard::confirmPressed, this,
        [this]()
        {
            if (!m_captureSetup) return;
            if (!m_captureSetup->isVisible()) return;

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
        m_captureSetup = new AtmaZoneCaptureSetup(nullptr);
        m_overlayRoot->registerOverlay(m_captureSetup);
    }

    m_captureSetup->loadSettings();
    m_captureSetup->show();
    m_captureSetup->raise();
    m_captureSetup->activateWindow();
    m_captureSetup->setFocus();

    m_overlayRoot->raiseAll();
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
        m_blueState = ZoneState::Unknown;

        emit invariantExited();
        return;
    }

    if (!AtmaZoneCaptureSetup::referencesExist())
    {
        QMessageBox::warning(nullptr, "Atma Zones",
                             "Le 7 zone non sono state configurate completamente. "
                             "Disattiva Atma e completa la configurazione.");
        m_enabled = false;
        return;
    }

    m_redStates.fill(ZoneState::Unknown);
    m_blueState = ZoneState::Unknown;

    const QString imagesDir = QCoreApplication::applicationDirPath() + "/images/";

    QMetaObject::invokeMethod(
        m_worker, "loadReferences", Qt::QueuedConnection,
        Q_ARG(QString, imagesDir)
        );
    // registerAllRegions()/subscribeAll() partono da onReferencesLoaded()
}

void AtmaZoneManager::onReferencesLoaded(bool ok)
{
    if (!m_enabled) return; // l'utente potrebbe aver disattivato nel frattempo

    if (!ok)
    {
        QMessageBox::warning(nullptr, "Atma Zones",
                             "Errore nel caricamento dei riferimenti salvati. Riprova a configurare le zone.");
        m_enabled = false;
        return;
    }

    m_configured = true;

#ifdef QT_DEBUG
    if (!m_debugWindow)
        m_debugWindow = new AtmaDebugWindow();

    m_debugWindow->show();
    m_debugWindow->raise();
    m_debugWindow->activateWindow();
#endif

    registerAllRegions();
    subscribeAll();
}

// ============================================================
// REGISTER / SUBSCRIBE
// ============================================================

void AtmaZoneManager::registerAllRegions()
{
    unregisterAllRegions();

    if (!m_captureSetup)
    {
        m_captureSetup = new AtmaZoneCaptureSetup(nullptr);
        m_captureSetup->loadSettings();
        m_captureSetup->hide();
    }

    for (int i = 0; i < RED_COUNT; ++i)
    {
        const QRect rect = m_captureSetup->redZoneRect(i);
        m_redRegionIds[i] = CaptureCoordinator::instance()->registerRegion(rect);
    }

    m_blueRegionId =
        CaptureCoordinator::instance()->registerRegion(m_captureSetup->blueZoneRect());
}

void AtmaZoneManager::unregisterAllRegions()
{
    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (m_redRegionIds[i] >= 0)
        {
            CaptureCoordinator::instance()->unregisterRegion(m_redRegionIds[i]);
            m_redRegionIds[i] = -1;
        }
    }

    if (m_blueRegionId >= 0)
    {
        CaptureCoordinator::instance()->unregisterRegion(m_blueRegionId);
        m_blueRegionId = -1;
    }
}

void AtmaZoneManager::subscribeAll()
{
    constexpr int INTERVAL_MS = 150;

    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (m_redRegionIds[i] < 0) continue;

        CaptureCoordinator::instance()->subscribe(
            m_redRegionIds[i], INTERVAL_MS,
            m_redProxies[i].get(), "frameReady"
            );
    }

    if (m_blueRegionId >= 0)
    {
        CaptureCoordinator::instance()->subscribe(
            m_blueRegionId, INTERVAL_MS,
            m_blueProxy.get(), "frameReady"
            );
    }
}

void AtmaZoneManager::unsubscribeAll()
{
    for (int i = 0; i < RED_COUNT; ++i)
        if (m_redRegionIds[i] >= 0)
            CaptureCoordinator::instance()->unsubscribe(m_redRegionIds[i]);

    if (m_blueRegionId >= 0)
        CaptureCoordinator::instance()->unsubscribe(m_blueRegionId);
}

// ============================================================
// RED ZONE COMPARED (risultato dal worker, già in coda sul thread GUI)
// ============================================================

void AtmaZoneManager::onRedZoneCompared(int index, bool isMatch)
{
    if (!m_enabled || !m_configured)
        return;

    if (index < 0 || index >= RED_COUNT)
        return;

    const ZoneState newState =
        isMatch ? ZoneState::Match : ZoneState::Mismatch;

    const ZoneState oldState = m_redStates[index];

    m_redStates[index] = newState;

    // Ci interessa solamente il passaggio:
    // MATCH -> MISMATCH
    if (oldState != ZoneState::Match ||
        newState != ZoneState::Mismatch)
    {
        return;
    }

    qDebug() << "[ATMA] RED" << index + 1
             << "MATCH -> MISMATCH"
             << "BLUE STATE =" << static_cast<int>(m_blueState);

    // Se BLUE è già MISMATCH, l'evento è immediatamente bloccato.
    if (m_blueState == ZoneState::Mismatch)
    {
        qDebug() << "[ATMA] RED" << index + 1
                 << "BLOCKED - BLUE already MISMATCH";
        return;
    }

    // Mettiamo il RED in attesa della conferma del gate.
    m_pendingRedEvents[index] = true;

    // Se il timer non è già attivo, avviamo la finestra
    // temporale per raccogliere eventuali eventi BLUE contemporanei.
    if (!m_gateTimer.isActive())
    {
        m_pendingEventsBlockedByBlue = false;

        qDebug() << "[ATMA] Gate window START"
                 << GATE_DELAY_MS << "ms";

        m_gateTimer.start(GATE_DELAY_MS);
    }
}


// ============================================================
// BLUE ZONE COMPARED (invariante)
// ============================================================

void AtmaZoneManager::onBlueZoneCompared(bool isMatch)
{
    if (!m_enabled || !m_configured)
        return;

    const ZoneState newState =
        isMatch ? ZoneState::Match : ZoneState::Mismatch;

    const ZoneState oldState = m_blueState;

    m_blueState = newState;

    // BLUE è entrato in MISMATCH.
    if (oldState != ZoneState::Mismatch &&
        newState == ZoneState::Mismatch)
    {
        emit invariantEntered();

        // Se abbiamo RED pendenti, significa che RED e BLUE
        // sono entrati in mismatch durante la stessa finestra.
        // Blocchiamo gli eventi RED.
        bool hasPending = false;

        for (bool pending : m_pendingRedEvents)
        {
            if (pending)
            {
                hasPending = true;
                break;
            }
        }

        if (hasPending)
        {
            m_pendingEventsBlockedByBlue = true;

            qDebug() << "[ATMA] BLUE entered MISMATCH"
                     << "-> pending RED events BLOCKED";
        }
    }

    // BLUE è tornato in MATCH.
    else if (oldState == ZoneState::Mismatch &&
             newState == ZoneState::Match)
    {
        emit invariantExited();

        qDebug() << "[ATMA] BLUE MATCH";
    }
}

#ifdef QT_DEBUG

void AtmaZoneManager::onRedDebugFrame(
    int index,
    QImage frame,
    bool isMatch)
{
    if (!m_debugWindow)
        return;

    m_debugWindow->updateRed(index, frame, isMatch);
}

void AtmaZoneManager::onBlueDebugFrame(
    QImage frame,
    bool isMatch)
{
    if (!m_debugWindow)
        return;

    m_debugWindow->updateBlue(frame, isMatch);
}

#endif

void AtmaZoneManager::processPendingRedEvents()
{
    if (!m_enabled || !m_configured)
    {
        clearPendingRedEvents();
        return;
    }

    qDebug() << "[ATMA] Gate window END"
             << "BLUE STATE =" << static_cast<int>(m_blueState)
             << "BLOCKED =" << m_pendingEventsBlockedByBlue;

    // Se BLUE è/è stato MISMATCH durante la finestra,
    // nessun RED pendente deve generare l'evento.
    if (m_pendingEventsBlockedByBlue ||
        m_blueState == ZoneState::Mismatch)
    {
        qDebug() << "[ATMA] Pending RED events discarded by gate";

        clearPendingRedEvents();
        return;
    }

    // BLUE è rimasto MATCH per tutta la finestra.
    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (!m_pendingRedEvents[i])
            continue;

        qDebug() << "[ATMA] RED" << i + 1
                 << "confirmed -> registerAction()";

        if (m_core)
            m_core->registerAction();
    }

    clearPendingRedEvents();
}


void AtmaZoneManager::clearPendingRedEvents()
{
    m_pendingRedEvents.fill(false);
    m_pendingEventsBlockedByBlue = false;
}
