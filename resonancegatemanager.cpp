#include "resonancegatemanager.h"
#include "resonancegatecapturesetup.h"
#include "resonancegateworker.h"
#include "globalkeyboard.h"
#include "overlayroot.h"
#include "capturecoordinator.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>

ResonanceGateManager::ResonanceGateManager(
    GlobalKeyboard *keyboard,
    OverlayRoot *overlayRoot,
    QObject *parent
    )
    : QObject(parent), m_keyboard(keyboard), m_overlayRoot(overlayRoot)
{
    m_worker = new ResonanceGateWorker();
    m_workerThread = new QThread(this);
    m_worker->moveToThread(m_workerThread);

    connect(m_worker, &ResonanceGateWorker::referenceLoaded,
            this, &ResonanceGateManager::onReferenceLoaded);
    connect(m_worker, &ResonanceGateWorker::compared,
            this, &ResonanceGateManager::onCompared);
#ifdef QT_DEBUG
    connect(m_worker, &ResonanceGateWorker::debugFrame,
            this, &ResonanceGateManager::debugFrame);
#endif

    m_workerThread->start();

    connect(m_keyboard, &GlobalKeyboard::keyPressed, this,
            [this](int key)
            {
                if (key != 'P') return;
                if (!m_captureSetup || !m_captureSetup->isVisible()) return;
                m_captureSetup->captureReference();
            });

    connect(m_keyboard, &GlobalKeyboard::confirmPressed, this,
            [this]()
            {
                if (!m_captureSetup || !m_captureSetup->isVisible()) return;
                m_captureSetup->saveSettings();
                m_captureSetup->hide();
                if (m_overlayRoot) m_overlayRoot->raiseAll();
            });

}

ResonanceGateManager::~ResonanceGateManager()
{
    unregisterRegion();
    m_workerThread->quit();
    m_workerThread->wait();
    delete m_worker;
}

void ResonanceGateManager::configure()
{
    if (!m_overlayRoot) return;

    if (!m_captureSetup)
    {
        m_captureSetup = new ResonanceGateCaptureSetup(nullptr);
        m_overlayRoot->registerOverlay(m_captureSetup);
    }

    m_captureSetup->loadSettings();
    m_captureSetup->show();
    m_captureSetup->raise();
    m_captureSetup->activateWindow();
    m_captureSetup->setFocus();
    m_overlayRoot->raiseAll();
}

void ResonanceGateManager::setEnabled(bool value)
{
    m_enabled = value;

    if (!m_enabled)
    {
        unregisterRegion();

        m_configured = false;
        m_state = State::Unknown;

        if (!m_gateOpen)
        {
            m_gateOpen = true;
            emit gateOpened();
        }

        return;
    }

    if (!ResonanceGateCaptureSetup::referenceExists())
    {
        QMessageBox::warning(
            nullptr,
            "Resonance Gate",
            "La zona di controllo non è stata configurata. "
            "Disattiva e completa la configurazione."
            );

        m_enabled = false;
        return;
    }

    // Il Gate è abilitato ma il riferimento
    // deve ancora essere caricato.
    m_configured = false;
    m_state = State::Unknown;

    const QString imagesDir =
        QCoreApplication::applicationDirPath() + "/images/";

    QMetaObject::invokeMethod(
        m_worker,
        "loadReference",
        Qt::QueuedConnection,
        Q_ARG(QString, imagesDir)
        );
}
void ResonanceGateManager::onReferenceLoaded(bool ok)
{
    if (!m_enabled)
        return;

    if (!ok)
    {
        QMessageBox::warning(
            nullptr,
            "Resonance Gate",
            "Errore nel caricamento del riferimento. "
            "Riprova a configurare la zona."
            );

        m_enabled = false;
        return;
    }

    m_configured = true;

    registerRegion();

    if (m_regionId >= 0)
    {
        CaptureCoordinator::instance()->subscribe(
            m_regionId,
            150,
            m_worker,
            "compareFrame"
            );
    }
}
void ResonanceGateManager::onCompared(bool isMatch)

{
    if (!m_enabled || !m_configured) return;

    const State newState = isMatch ? State::Match : State::Mismatch;
    const State oldState = m_state;
    m_state = newState;

    if (oldState != State::Mismatch && newState == State::Mismatch)
    {
        m_gateOpen = false;
        emit gateClosed();
    }
    else if (oldState == State::Mismatch && newState == State::Match)
    {
        m_gateOpen = true;
        emit gateOpened();
    }
}

void ResonanceGateManager::registerRegion()
{
    unregisterRegion();

    if (!m_captureSetup)
    {
        m_captureSetup = new ResonanceGateCaptureSetup(nullptr);
        m_captureSetup->loadSettings();
        m_captureSetup->hide();
    }

    m_regionId = CaptureCoordinator::instance()->registerRegion(m_captureSetup->gateZoneRect());
}

void ResonanceGateManager::unregisterRegion()
{
    if (m_regionId >= 0)
    {
        CaptureCoordinator::instance()->unsubscribe(m_regionId);
        CaptureCoordinator::instance()->unregisterRegion(m_regionId);
        m_regionId = -1;
    }
}