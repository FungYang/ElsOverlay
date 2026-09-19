#pragma once

#include <QObject>
#include <QImage>

// Proxy leggero: inoltra il frame di UNA zona rossa specifica
// al manager, portando con sé l'indice fisso della zona.
// Necessario perché CaptureCoordinator::subscribe() chiama uno
// slot con firma fissa (QImage) — qui aggiungiamo l'indice
// mancante prima di richiamare il vero handler.
class AtmaRedZoneProxy : public QObject
{
    Q_OBJECT

public:
    explicit AtmaRedZoneProxy(int index, QObject *parent = nullptr)
        : QObject(parent), m_index(index)
    {
    }

    int index() const { return m_index; }

public slots:
    void frameReady(QImage frame)
    {
        emit forwardedFrame(m_index, frame);
    }

signals:
    void forwardedFrame(int index, QImage frame);

private:
    int m_index;
};