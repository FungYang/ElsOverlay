#include "capturecoordinator.h"
#include <QDateTime>
#include <QMetaObject>
#include <QDebug>

CaptureCoordinator *CaptureCoordinator::s_instance = nullptr;

CaptureCoordinator *CaptureCoordinator::instance()
{
    if (!s_instance)
    {
        s_instance = new CaptureCoordinator();
    }
    return s_instance;
}

CaptureCoordinator::CaptureCoordinator(QObject *parent)
    : QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &CaptureCoordinator::start);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_thread->start();
}

CaptureCoordinator::~CaptureCoordinator()
{
    m_thread->quit();
    m_thread->wait();
}

void CaptureCoordinator::start()
{
    // Il timer va creato QUI: eredita l'affinità del thread capture.
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CaptureCoordinator::tick);
    m_timer->start(MASTER_TICK_MS);
}

// ============================================================
// API PUBBLICA — wrapper cross-thread
// ============================================================

int CaptureCoordinator::registerRegion(const QRect &rect)
{
    if (QThread::currentThread() == thread())
        return doRegisterRegion(rect);

    int result = -1;
    QMetaObject::invokeMethod(
        this, "doRegisterRegion",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(int, result),
        Q_ARG(QRect, rect)
        );
    return result;
}

void CaptureCoordinator::unregisterRegion(int regionId)
{
    QMetaObject::invokeMethod(
        this, "doUnregisterRegion",
        Qt::QueuedConnection,
        Q_ARG(int, regionId)
        );
}

bool CaptureCoordinator::updateRegion(int regionId, const QRect &rect)
{
    if (QThread::currentThread() == thread())
        return doUpdateRegion(regionId, rect);

    bool result = false;
    QMetaObject::invokeMethod(
        this, "doUpdateRegion",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, result),
        Q_ARG(int, regionId),
        Q_ARG(QRect, rect)
        );
    return result;
}

void CaptureCoordinator::subscribe(int regionId, int intervalMs, QObject *receiver, const char *slot)
{
    QMetaObject::invokeMethod(
        this, "doSubscribe",
        Qt::QueuedConnection,
        Q_ARG(int, regionId),
        Q_ARG(int, intervalMs),
        Q_ARG(QObject *, receiver),
        Q_ARG(QByteArray, QByteArray(slot))
        );
}

void CaptureCoordinator::unsubscribe(int regionId)
{
    QMetaObject::invokeMethod(
        this, "doUnsubscribe",
        Qt::QueuedConnection,
        Q_ARG(int, regionId)
        );
}

// ============================================================
// IMPLEMENTAZIONI REALI — eseguite SEMPRE sul thread capture
// ============================================================

int CaptureCoordinator::doRegisterRegion(const QRect &rect)
{
    return ScreenCapture::registerRegion(rect);
}

void CaptureCoordinator::doUnregisterRegion(int regionId)
{
    ScreenCapture::unregisterRegion(regionId);

    for (int i = m_subs.size() - 1; i >= 0; --i)
        if (m_subs[i].regionId == regionId)
            m_subs.removeAt(i);
}

bool CaptureCoordinator::doUpdateRegion(int regionId, const QRect &rect)
{
    return ScreenCapture::updateRegion(regionId, rect);
}

void CaptureCoordinator::doSubscribe(int regionId, int intervalMs, QObject *receiver, QByteArray slot)
{
    // Se esiste già una subscription per questa region, sostituiamola.
    for (auto &sub : m_subs)
    {
        if (sub.regionId == regionId)
        {
            sub.intervalMs = intervalMs;
            sub.receiver = receiver;
            sub.slot = slot;
            return;
        }
    }

    Subscription sub;
    sub.regionId = regionId;
    sub.intervalMs = intervalMs;
    sub.nextDue = QDateTime::currentMSecsSinceEpoch();
    sub.receiver = receiver;
    sub.slot = slot;

    m_subs.append(sub);
}

void CaptureCoordinator::doUnsubscribe(int regionId)
{
    for (int i = m_subs.size() - 1; i >= 0; --i)
        if (m_subs[i].regionId == regionId)
            m_subs.removeAt(i);
}

// ============================================================
// TICK — l'unico punto che chiama beginFrame/captureRegion/endFrame
// ============================================================

void CaptureCoordinator::tick()
{
    if (m_subs.isEmpty())
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    QVector<int> due;

    for (auto &sub : m_subs)
    {
        if (!sub.receiver)
            continue; // receiver distrutto, verrà ripulito più avanti

        if (now >= sub.nextDue)
        {
            due.append(sub.regionId);
            sub.nextDue = now + sub.intervalMs;
        }
    }

    if (due.isEmpty())
        return;

    if (!ScreenCapture::beginFrame())
        return;

    for (int regionId : due)
    {
        QImage frame = ScreenCapture::captureRegion(regionId);

        if (frame.isNull())
            continue;

        for (auto &sub : m_subs)
        {
            if (sub.regionId != regionId || !sub.receiver)
                continue;

            QMetaObject::invokeMethod(
                sub.receiver,
                sub.slot.constData(),
                Qt::QueuedConnection,
                Q_ARG(QImage, frame)
                );
        }
    }

    ScreenCapture::endFrame();

    // Pulizia receiver morti (moduli distrutti nel frattempo).
    for (int i = m_subs.size() - 1; i >= 0; --i)
        if (!m_subs[i].receiver)
            m_subs.removeAt(i);
}