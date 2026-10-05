#include "capturecoordinator.h"

#include <QDateTime>
#include <QMetaObject>
#include <QDebug>

#include <limits>


CaptureCoordinator *CaptureCoordinator::s_instance = nullptr;


// ============================================================
// SINGLETON
// ============================================================

CaptureCoordinator *CaptureCoordinator::instance()
{
    if (!s_instance)
    {
        s_instance = new CaptureCoordinator();
    }

    return s_instance;
}


// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================

CaptureCoordinator::CaptureCoordinator(QObject *parent)
    : QObject(parent)
{
    m_thread = new QThread();

    moveToThread(m_thread);

    connect(
        m_thread,
        &QThread::started,
        this,
        &CaptureCoordinator::start
        );

    connect(
        m_thread,
        &QThread::finished,
        m_thread,
        &QObject::deleteLater
        );

    m_thread->start();
}


CaptureCoordinator::~CaptureCoordinator()
{
    m_thread->quit();
    m_thread->wait();
}


// ============================================================
// THREAD INITIALIZATION
// ============================================================

void CaptureCoordinator::start()
{
    // Il timer viene creato sul thread capture.
    m_timer = new QTimer(this);

    // Il timer viene armato solo fino alla prossima
    // subscription da elaborare.
    m_timer->setSingleShot(true);

    connect(
        m_timer,
        &QTimer::timeout,
        this,
        &CaptureCoordinator::tick
        );

    scheduleNextTick();
}


// ============================================================
// API PUBBLICA — wrapper cross-thread
// ============================================================

int CaptureCoordinator::registerRegion(const QRect &rect)
{
    if (QThread::currentThread() == thread())
    {
        return doRegisterRegion(rect);
    }

    int result = -1;

    QMetaObject::invokeMethod(
        this,
        "doRegisterRegion",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(int, result),
        Q_ARG(QRect, rect)
        );

    return result;
}


void CaptureCoordinator::unregisterRegion(int regionId)
{
    QMetaObject::invokeMethod(
        this,
        "doUnregisterRegion",
        Qt::QueuedConnection,
        Q_ARG(int, regionId)
        );
}


bool CaptureCoordinator::updateRegion(
    int regionId,
    const QRect &rect
    )
{
    if (QThread::currentThread() == thread())
    {
        return doUpdateRegion(regionId, rect);
    }

    bool result = false;

    QMetaObject::invokeMethod(
        this,
        "doUpdateRegion",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, result),
        Q_ARG(int, regionId),
        Q_ARG(QRect, rect)
        );

    return result;
}


void CaptureCoordinator::subscribe(
    int regionId,
    int intervalMs,
    QObject *receiver,
    const char *slot
    )
{
    QMetaObject::invokeMethod(
        this,
        "doSubscribe",
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
        this,
        "doUnsubscribe",
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
    {
        if (m_subs[i].regionId == regionId)
        {
            m_subs.removeAt(i);
        }
    }

    scheduleNextTick();
}


bool CaptureCoordinator::doUpdateRegion(
    int regionId,
    const QRect &rect
    )
{
    return ScreenCapture::updateRegion(regionId, rect);
}


void CaptureCoordinator::doSubscribe(
    int regionId,
    int intervalMs,
    QObject *receiver,
    QByteArray slot
    )
{
    intervalMs = qMax(1, intervalMs);

    for (auto &sub : m_subs)
    {
        if (sub.regionId == regionId)
        {
            sub.intervalMs = intervalMs;
            sub.receiver = receiver;
            sub.slot = slot;

            scheduleNextTick();

            return;
        }
    }

    Subscription sub;

    sub.regionId = regionId;
    sub.intervalMs = intervalMs;

    sub.nextDue =
        QDateTime::currentMSecsSinceEpoch();

    sub.receiver = receiver;
    sub.slot = slot;

    m_subs.append(sub);

    scheduleNextTick();
}


void CaptureCoordinator::doUnsubscribe(int regionId)
{
    for (int i = m_subs.size() - 1; i >= 0; --i)
    {
        if (m_subs[i].regionId == regionId)
        {
            m_subs.removeAt(i);
        }
    }

    scheduleNextTick();
}


// ============================================================
// SCHEDULER DINAMICO
// ============================================================

void CaptureCoordinator::scheduleNextTick()
{
    if (!m_timer)
    {
        return;
    }

    for (int i = m_subs.size() - 1; i >= 0; --i)
    {
        if (!m_subs[i].receiver)
        {
            m_subs.removeAt(i);
        }
    }

    if (m_subs.isEmpty())
    {
        m_timer->stop();
        return;
    }

    const qint64 now =
        QDateTime::currentMSecsSinceEpoch();

    qint64 nextDelay =
        std::numeric_limits<qint64>::max();

    for (const auto &sub : m_subs)
    {
        const qint64 delay =
            sub.nextDue - now;

        if (delay <= 0)
        {
            nextDelay = 0;
            break;
        }

        if (delay < nextDelay)
        {
            nextDelay = delay;
        }
    }

    if (nextDelay == std::numeric_limits<qint64>::max())
    {
        m_timer->stop();
        return;
    }

    const int timerDelay =
        static_cast<int>(
            qMin<qint64>(
                nextDelay,
                qint64(std::numeric_limits<int>::max())
                )
            );

    m_timer->start(timerDelay);
}


// ============================================================
// TICK
// ============================================================

void CaptureCoordinator::tick()
{
    if (m_subs.isEmpty())
    {
        return;
    }

    const qint64 now =
        QDateTime::currentMSecsSinceEpoch();

    QVector<int> due;

    // Individuiamo tutte le ROI che devono essere catturate.
    for (auto &sub : m_subs)
    {
        if (!sub.receiver)
        {
            continue;
        }

        if (now >= sub.nextDue)
        {
            due.append(sub.regionId);

            sub.nextDue =
                now + qMax(1, sub.intervalMs);
        }
    }


    if (!due.isEmpty())
    {
        // Tutte le ROI dovute vengono passate insieme.
        //
        // ScreenCapture:
        //   AcquireNextFrame()
        //   N x CopySubresourceRegion()
        //   1 x Map()
        //   N x lettura CPU
        //   1 x Unmap()
        if(ScreenCapture::beginFrame(due))
        {
            const quint64 frameId = ++m_frameId;

            for(int regionId : due)
            {
                QImage frame =
                    ScreenCapture::captureRegion(regionId);

                if(frame.isNull())
                    continue;

                for(auto &sub : m_subs)
                {
                    if(sub.regionId != regionId ||
                        !sub.receiver)
                    {
                        continue;
                    }

                    QMetaObject::invokeMethod(
                        sub.receiver,
                        sub.slot.constData(),
                        Qt::QueuedConnection,
                        Q_ARG(quint64, frameId),
                        Q_ARG(QImage, frame)
                        );
                }
            }

            ScreenCapture::endFrame();
        }
    }


    // Pulizia receiver morti.
    for(int i = m_subs.size() - 1; i >= 0; --i)
    {
        if(!m_subs[i].receiver)
        {
            m_subs.removeAt(i);
        }
    }


    scheduleNextTick();
}