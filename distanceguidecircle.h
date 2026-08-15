#ifndef DISTANCEGUIDECIRCLE_H
#define DISTANCEGUIDECIRCLE_H

#include <QWidget>
#include <QColor>
#include <QRect>
#include <QPoint>

class DistanceGuideCircle :
                            public QWidget
{
    Q_OBJECT

public:

    explicit DistanceGuideCircle(
        const QColor &color,
        QWidget *parent = nullptr
        );


    void setConfigurationMode(
        bool enabled
        );


    int configurationPositionX() const;


    int configurationPositionY() const;


    int configurationSize() const;


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

    enum class DragMode
    {
        None,
        Move,
        Resize
    };


    DragMode dragModeAt(
        const QPoint &position
        ) const;


    void updateCursor(
        const QPoint &position
        );


private:

    QColor m_color;


    bool m_configurationMode =
        false;


    bool m_dragging =
        false;


    DragMode m_dragMode =
        DragMode::None;


    QPoint m_dragStartGlobal;


    QRect m_dragStartGeometry;


    int m_borderSize =
        8;
};

#endif