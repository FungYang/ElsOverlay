
#ifndef BUFFBOX_H
#define BUFFBOX_H

#include <QPoint>
#include <QSize>
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


    void sizeChanged(
        const QSize &size
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


    void enterEvent(
        QEnterEvent *event
        ) override;


    void leaveEvent(
        QEvent *event
        ) override;


private:

    enum class State
    {
        Ready,
        Active,
        Expired
    };


    enum class ResizeCorner
    {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
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


    bool m_resizing =
        false;


    ResizeCorner m_resizeCorner =
        ResizeCorner::None;


    QPoint m_dragOffset;


    QPoint m_resizeStartGlobal;


    QPoint m_resizeStartPosition;


    QSize m_resizeStartSize;


    int m_glowAlpha =
        80;


    bool m_glowIncreasing =
        true;


    // ========================================================
    // RIDIMENSIONAMENTO
    // ========================================================

    static constexpr int ResizeMargin = 8;


    static constexpr int MinimumSize = 20;


    ResizeCorner resizeCornerAt(
        const QPoint &position
        ) const;


    void updateResizeCursor(
        const QPoint &position
        );


    void resizeFromCorner(
        const QPoint &globalPosition
        );


    void cooldownFinished();


    QString keyName(
        int key
        ) const;
};


#endif
