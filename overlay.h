#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QSettings>
#include <QElapsedTimer>
class Overlay : public QWidget
{
    Q_OBJECT

public:

    Overlay(QWidget *parent = nullptr);

    void resetCooldown();
    void startCooldown();
    void togglePause();

protected:

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;


private:
    bool running = false;
    int cooldown = 20;
    bool paused = false;
    QElapsedTimer elapsedTimer;
     qint64 pausedElapsed = 0;

    QPoint dragPosition;

    QTimer timer;
};

#endif