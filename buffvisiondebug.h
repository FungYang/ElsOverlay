#ifndef BUFFVISIONDEBUG_H
#define BUFFVISIONDEBUG_H

#include <QWidget>
#include <QString>
#include <QSize>

class QLabel;

class BuffVisionDebug : public QWidget
{
    Q_OBJECT

public:

    explicit BuffVisionDebug(
        QWidget *parent = nullptr
        );

    void updateNumbers(
        int crop1,
        int crop2
        );

    void setResolution(
        int width,
        int height,
        const QSize &cropSize
        );

    void setLastEvent(
        const QString &event
        );

private:

    QLabel *resolutionLabel;

    QLabel *crop1Label;
    QLabel *crop2Label;

    QLabel *lastEventLabel;
};

#endif