#pragma once

#include <QObject>
#include <QImage>
#include <QRect>

class TranscendenceVisionWorker : public QObject
{
    Q_OBJECT

public:
    explicit TranscendenceVisionWorker(QObject *parent = nullptr);

public slots:
    void processFrame(QImage area);
    void setTemplate(QImage templateIcon, int iconWidth, int iconHeight);

signals:
    void scanResult(bool found, QRect foundRect, double score, QImage area);

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

    QImage m_templateIcon;
};