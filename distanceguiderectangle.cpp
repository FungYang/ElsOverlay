#include "distanceguiderectangle.h"

#include <QMouseEvent>
#include <QPainter>


DistanceGuideRectangle::DistanceGuideRectangle(
    const QColor &color,
    QWidget *parent
    )
    : QWidget(parent),
    m_color(color)
{
    setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Tool |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setAttribute(
        Qt::WA_NoSystemBackground
        );
    setAttribute(
        Qt::WA_TransparentForMouseEvents,
        false
        );


    setMouseTracking(
        true
        );


    setMinimumSize(
        20,
        20
        );
}



void DistanceGuideRectangle::paintEvent(
    QPaintEvent *
    )
{
    QPainter painter(
        this
        );


    painter.setRenderHint(
        QPainter::Antialiasing,
        true
        );


    // Superficie praticamente invisibile ma hit-testabile.
    QColor transparentSurface =
        m_color;

    transparentSurface.setAlpha(
        1
        );


    painter.setPen(
        Qt::NoPen
        );


    painter.setBrush(
        transparentSurface
        );


    painter.drawRect(
        rect()
        );


    // Bordo visibile.
    QPen pen(
        m_color
        );


    pen.setWidth(
        3
        );


    painter.setPen(
        pen
        );


    painter.setBrush(
        Qt::NoBrush
        );


    QRect drawRect =
        rect();


    drawRect.adjust(
        1,
        1,
        -1,
        -1
        );


    painter.drawRect(
        drawRect
        );
}



DistanceGuideRectangle::DragMode
DistanceGuideRectangle::dragModeAt(
    const QPoint &position
    ) const
{
    const bool left =
        position.x() <= m_borderSize;

    const bool right =
        position.x() >=
        width() - m_borderSize;

    const bool top =
        position.y() <= m_borderSize;

    const bool bottom =
        position.y() >=
        height() - m_borderSize;


    if(left && top)
        return DragMode::ResizeTopLeft;

    if(right && top)
        return DragMode::ResizeTopRight;

    if(left && bottom)
        return DragMode::ResizeBottomLeft;

    if(right && bottom)
        return DragMode::ResizeBottomRight;


    if(left)
        return DragMode::ResizeLeft;

    if(right)
        return DragMode::ResizeRight;

    if(top)
        return DragMode::ResizeTop;

    if(bottom)
        return DragMode::ResizeBottom;


    // Tutta la zona interna:
    return DragMode::Move;
}



void DistanceGuideRectangle::updateCursor(
    const QPoint &position
    )
{
    if(!m_configurationMode)
    {
        setCursor(
            Qt::ArrowCursor
            );

        return;
    }


    switch(dragModeAt(position))
    {
    case DragMode::ResizeLeft:
    case DragMode::ResizeRight:
        setCursor(
            Qt::SizeHorCursor
            );
        break;


    case DragMode::ResizeTop:
    case DragMode::ResizeBottom:
        setCursor(
            Qt::SizeVerCursor
            );
        break;


    case DragMode::ResizeTopLeft:
    case DragMode::ResizeBottomRight:
        setCursor(
            Qt::SizeFDiagCursor
            );
        break;


    case DragMode::ResizeTopRight:
    case DragMode::ResizeBottomLeft:
        setCursor(
            Qt::SizeBDiagCursor
            );
        break;


    default:
        setCursor(
            Qt::SizeAllCursor
            );
        break;
    }
}



void DistanceGuideRectangle::mousePressEvent(
    QMouseEvent *event
    )
{
    if(!m_configurationMode)
    {
        event->ignore();
        return;
    }


    if(event->button() != Qt::LeftButton)
    {
        event->ignore();
        return;
    }


    const QPoint position =
        event->position().toPoint();


    m_dragMode =
        dragModeAt(
            position
            );


    m_dragStartGlobal =
        event->globalPosition().toPoint();


    m_dragStartGeometry =
        geometry();


    m_dragging =
        true;


    event->accept();
}



void DistanceGuideRectangle::mouseMoveEvent(
    QMouseEvent *event
    )
{
    const QPoint position =
        event->position().toPoint();


    if(!m_configurationMode)
    {
        event->ignore();
        return;
    }


    if(!m_dragging)
    {
        updateCursor(
            position
            );

        event->accept();
        return;
    }


    const QPoint delta =
        event->globalPosition().toPoint() -
        m_dragStartGlobal;


    QRect newGeometry =
        m_dragStartGeometry;


    switch(m_dragMode)
    {
    case DragMode::Move:

        newGeometry.translate(
            delta
            );

        break;


    case DragMode::ResizeLeft:

        newGeometry.setLeft(
            m_dragStartGeometry.left() +
            delta.x()
            );

        break;


    case DragMode::ResizeRight:

        newGeometry.setRight(
            m_dragStartGeometry.right() +
            delta.x()
            );

        break;


    case DragMode::ResizeTop:

        newGeometry.setTop(
            m_dragStartGeometry.top() +
            delta.y()
            );

        break;


    case DragMode::ResizeBottom:

        newGeometry.setBottom(
            m_dragStartGeometry.bottom() +
            delta.y()
            );

        break;


    case DragMode::ResizeTopLeft:

        newGeometry.setTop(
            m_dragStartGeometry.top() +
            delta.y()
            );

        newGeometry.setLeft(
            m_dragStartGeometry.left() +
            delta.x()
            );

        break;


    case DragMode::ResizeTopRight:

        newGeometry.setTop(
            m_dragStartGeometry.top() +
            delta.y()
            );

        newGeometry.setRight(
            m_dragStartGeometry.right() +
            delta.x()
            );

        break;


    case DragMode::ResizeBottomLeft:

        newGeometry.setBottom(
            m_dragStartGeometry.bottom() +
            delta.y()
            );

        newGeometry.setLeft(
            m_dragStartGeometry.left() +
            delta.x()
            );

        break;


    case DragMode::ResizeBottomRight:

        newGeometry.setBottom(
            m_dragStartGeometry.bottom() +
            delta.y()
            );

        newGeometry.setRight(
            m_dragStartGeometry.right() +
            delta.x()
            );

        break;


    case DragMode::None:
        return;
    }


    if(newGeometry.width() <
        minimumWidth())
    {
        newGeometry.setWidth(
            minimumWidth()
            );
    }


    if(newGeometry.height() <
        minimumHeight())
    {
        newGeometry.setHeight(
            minimumHeight()
            );
    }


    setGeometry(
        newGeometry
        );


    event->accept();
}


void DistanceGuideRectangle::mouseReleaseEvent(
    QMouseEvent *event
    )
{
    if(event->button() != Qt::LeftButton)
        return;


    m_dragging =
        false;


    m_dragMode =
        DragMode::None;


    event->accept();
}



void DistanceGuideRectangle::setConfigurationMode(
    bool enabled
    )
{
    m_configurationMode =
        enabled;


    setCursor(
        enabled
            ? Qt::SizeAllCursor
            : Qt::ArrowCursor
        );
}



int DistanceGuideRectangle::configurationPositionX() const
{
    return x();
}



int DistanceGuideRectangle::configurationPositionY() const
{
    return y();
}


void DistanceGuideRectangle::setOpacity(
    int opacity
    )
{
    if(opacity < 0)
        opacity = 0;


    if(opacity > 255)
        opacity = 255;


    m_opacity =
        opacity;


    update();
}

int DistanceGuideRectangle::configurationWidth() const
{
    return width();
}


int DistanceGuideRectangle::configurationHeight() const
{
    return height();
}