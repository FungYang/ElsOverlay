#ifndef BUFFBOX_H
#define BUFFBOX_H

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <QChar>
#include <QVector>
#include <QPainter>
#include <QSettings>


class BuffBox : public QWidget
{
    Q_OBJECT

public:

    explicit BuffBox(
        QChar key,
        QVector<int> cooldowns,
        QString className,
        QString boxId,
        QWidget *parent = nullptr
        );


    void startCooldown();
    void reset();
    bool hasSavedPosition() const;
    void confirmPlacement();
    QChar key() const;


protected:

    void paintEvent(QPaintEvent *event) override;


    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private slots:

    void cooldownFinished();


private:

    void drawDebugCountdown(QPainter& painter);

    enum class State
    {
        Placement,
        Ready,
        Active,
        Expired
    };


    QChar m_key;

    QVector<int> m_cooldowns;
    int m_currentState;

    QTimer m_timer;

    State m_state;

    bool m_configured;

    bool m_dragging;

    QPoint m_dragOffset;

    int m_remainingTime;

    QTimer m_glowTimer;

    int m_glowAlpha;
    bool m_glowIncreasing;
    QString className;
    QString boxId;


};

#endif