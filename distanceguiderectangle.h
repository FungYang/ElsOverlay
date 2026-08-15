#ifndef DISTANCEGUIDERECTANGLE_H
#define DISTANCEGUIDERECTANGLE_H

#include <QWidget>
#include <QColor>
#include <QPoint>
#include <QRect>

class DistanceGuideRectangle :
                               public QWidget
{
    Q_OBJECT

public:

    explicit DistanceGuideRectangle(
        const QColor &color,
        QWidget *parent = nullptr
        );


    void setConfigurationMode(
        bool enabled
        );


    int configurationPositionX() const;

    int configurationPositionY() const;

    int configurationWidth() const;

    int configurationHeight() const;


    void setOpacity(
        int opacity
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

    enum class DragMode
    {
        None,
        Move,

        ResizeLeft,
        ResizeRight,
        ResizeTop,
        ResizeBottom,

        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight
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


    int m_opacity =
        255;


    // Zona sensibile per il resize.
    // Il bordo visivo resta di 3 px.
    static constexpr int m_borderSize =
        6;
};

#endif