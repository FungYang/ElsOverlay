#ifndef BUFFBOX_H
#define BUFFBOX_H

#include <QPoint>
#include <QTimer>
#include <QWidget>

    class QMouseEvent;
class QPaintEvent;

class BuffBox : public QWidget
{
    Q_OBJECT

public:

    explicit BuffBox(
        int key,
        int cooldown,
        QWidget *parent = nullptr
        );


    int key() const;


    void startCooldown();

    void reset();


    void setConfigurationPosition(
        const QPoint &position
        );


    QPoint configurationPosition() const;


    void setConfigurationMode(
        bool enabled
        );


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


private:

    enum class State
    {
        Ready,
        Active,
        Expired
    };


    int m_key = 0;

    int m_cooldown = 0;


    QTimer m_timer;

    QTimer m_glowTimer;


    State m_state =
        State::Ready;


    bool m_configurationMode =
        false;


    bool m_dragging =
        false;


    QPoint m_dragOffset;


    int m_glowAlpha =
        80;


    bool m_glowIncreasing =
        true;


    void cooldownFinished();
    QString keyName(int key) const;

};

#endif
