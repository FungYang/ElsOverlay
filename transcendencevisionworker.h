#pragma once

#include <QObject>
#include <QImage>
#include <QRect>
#include <QMutex>
#include <QThreadPool>


class TranscendenceVisionWorker : public QObject
{
    Q_OBJECT

public:
    explicit TranscendenceVisionWorker(QObject *parent = nullptr);
    ~TranscendenceVisionWorker() override;

    // Thread-safe.
    // Riceve un nuovo frame e mantiene solamente
    // l'ultimo frame disponibile quando il worker è occupato.
    void submitFrame(QImage area);

public slots:
    void processFrame(QImage area);

    void setTemplate(
        QImage templateIcon,
        int iconWidth,
        int iconHeight
        );

signals:
    void scanResult(
        bool found,
        QRect foundRect,
        double score,
        QImage area
        );

private:
    bool findIcon(
        const QImage &area,
        QRect &foundRect,
        double &score
        ) const;

    double compareAt(
        const QImage &area,
        int offsetX,
        int offsetY
        ) const;

private:
    // Template utilizzato dal matcher.
    QImage m_templateIcon;

    // ---------------------------------------------------------
    // Latest-frame-wins
    // ---------------------------------------------------------

    QMutex m_frameMutex;

    QImage m_pendingFrame;

    bool m_processing = false;

    // ---------------------------------------------------------
    // Persistent matcher thread pool
    // ---------------------------------------------------------

    mutable QThreadPool m_matchPool;
};