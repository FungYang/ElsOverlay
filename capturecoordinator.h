#ifndef CAPTURECOORDINATOR_H
#define CAPTURECOORDINATOR_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QPointer>
#include <QRect>
#include <QImage>

#include "screencapture.h"

class CaptureCoordinator : public QObject
{
    Q_OBJECT

public:
    // Singleton: un solo coordinator per l'intera applicazione,
    // un solo thread "capture" per tutto il processo.
    static CaptureCoordinator *instance();

    // ==== API pubblica: chiamabile da QUALSIASI thread (GUI compresa) ====
    // Internamente si occupano di marshalling verso il thread capture.

    int registerRegion(const QRect &rect);
    void unregisterRegion(int regionId);
    bool updateRegion(int regionId, const QRect &rect);

    // receiver/slot ricevono: void slot(QImage frame) — sempre in coda
    // sul thread del receiver, quindi è SICURO anche se receiver vive
    // sul thread GUI o su un altro worker.
    void subscribe(
        int regionId,
        int intervalMs,
        QObject *receiver,
        const char *slot
        );

    void unsubscribe(int regionId);

private:
    explicit CaptureCoordinator(QObject *parent = nullptr);
    ~CaptureCoordinator();

    static CaptureCoordinator *s_instance;

    QThread *m_thread = nullptr;

private slots:
    void start();
    void tick();

    // Implementazioni reali, eseguite SEMPRE sul thread capture.
    int doRegisterRegion(const QRect &rect);
    void doUnregisterRegion(int regionId);
    bool doUpdateRegion(int regionId, const QRect &rect);

    void doSubscribe(
        int regionId,
        int intervalMs,
        QObject *receiver,
        QByteArray slot
        );

    void doUnsubscribe(int regionId);

private:
    struct Subscription
    {
        int regionId = -1;
        int intervalMs = 50;
        qint64 nextDue = 0;

        QPointer<QObject> receiver;
        QByteArray slot;
    };

    // Programma il timer in base alla prossima subscription
    // che deve essere eseguita.
    void scheduleNextTick();

    QVector<Subscription> m_subs;

    QTimer *m_timer = nullptr;

    quint64 m_frameId = 0;
};

#endif