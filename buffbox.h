#ifndef BUFFBOX_H
#define BUFFBOX_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QChar>


class BuffBox : public QWidget
{
    Q_OBJECT

public:

    explicit BuffBox(
        QChar key,
        int cooldown,
        QWidget *parent = nullptr
        );


    void startCooldown();

    void reset();

    QChar key() const;


    void setConfigurationPosition(
        const QPoint &position
        );


    QPoint configurationPosition() const;

    void setConfigurationMode(bool enabled);

    bool isInConfigurationMode() const;


signals:

    void positionChanged(
        const QPoint &position
        );


protected:

    void paintEvent(
        QPaintEvent *event
        ) override;


    void mousePressEvent(
        QMouseEvent *event
        ) override;


    void mouseMoveEvent(
        QMouseEvent *event
        ) override;


    void mouseReleaseEvent(
        QMouseEvent *event
        ) override;


private slots:

    void cooldownFinished();


private:

    enum class State
    {
        Ready,
        Active,
        Expired
    };


    QChar m_key;

    int m_cooldown = 0;

    QTimer m_timer;

    State m_state =
        State::Ready;


    bool m_dragging = false;

    QPoint m_dragOffset;


    QTimer m_glowTimer;

    int m_glowAlpha = 80;

    bool m_glowIncreasing = true;

    bool m_configurationMode;

};

#endif