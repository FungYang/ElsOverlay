#include "distanceguidecircle.h"

#include <QMouseEvent>
#include <QPainter>


DistanceGuideCircle::DistanceGuideCircle(
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



void DistanceGuideCircle::paintEvent(
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


    QPen pen(
        m_color
        );


    pen.setWidth(
        3
        );


    painter.setPen(
        pen
        );


    QColor fillColor =
        m_color;


    fillColor.setAlpha(
        1
        );


    painter.setBrush(
        fillColor
        );


    QRect drawRect =
        rect();


    drawRect.adjust(
        1,
        1,
        -1,
        -1
        );


    painter.drawEllipse(
        drawRect
        );
}



DistanceGuideCircle::DragMode
DistanceGuideCircle::dragModeAt(
    const QPoint &position
    ) const
{
    const QPointF center(
        width() / 2.0,
        height() / 2.0
        );


    const double dx =
        position.x() -
        center.x();


    const double dy =
        position.y() -
        center.y();


    const double distance =
        qSqrt(
            dx * dx +
            dy * dy
            );


    const double radius =
        qMin(
            width(),
            height()
            ) / 2.0;


    if(qAbs(distance - radius) <=
        m_borderSize)
    {
        return DragMode::Resize;
    }


    if(distance < radius)
    {
        return DragMode::Move;
    }


    return DragMode::None;
}



void DistanceGuideCircle::updateCursor(
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
    case DragMode::Resize:

        setCursor(
            Qt::SizeFDiagCursor
            );

        break;


    case DragMode::Move:

        setCursor(
            Qt::SizeAllCursor
            );

        break;


    default:

        setCursor(
            Qt::ArrowCursor
            );

        break;
    }
}



void DistanceGuideCircle::mousePressEvent(
    QMouseEvent *event
    )
{
    if(!m_configurationMode)
        return;


    if(event->button() != Qt::LeftButton)
        return;


    m_dragMode =
        dragModeAt(
            event->position().toPoint()
            );


    if(m_dragMode ==
        DragMode::None)
    {
        return;
    }


    m_dragging =
        true;


    m_dragStartGlobal =
        event->globalPosition().toPoint();


    m_dragStartGeometry =
        geometry();


    event->accept();
}



void DistanceGuideCircle::mouseMoveEvent(
    QMouseEvent *event
    )
{
    const QPoint position =
        event->position().toPoint();


    if(!m_dragging)
    {
        updateCursor(
            position
            );

        return;
    }


    const QPoint delta =
        event->globalPosition().toPoint() -
        m_dragStartGlobal;


    QRect newGeometry =
        m_dragStartGeometry;


    if(m_dragMode ==
        DragMode::Move)
    {
        newGeometry.translate(
            delta
            );
    }
    else if(m_dragMode ==
             DragMode::Resize)
    {
        /*
         * Il cerchio deve rimanere sempre
         * perfettamente circolare.
         *
         * Usiamo lo spostamento massimo
         * dei due assi come variazione
         * del diametro.
         */

        const int originalSize =
            m_dragStartGeometry.width();


        const int deltaX =
            delta.x();


        const int deltaY =
            delta.y();


        const int deltaSize =
            qAbs(deltaX) >
                    qAbs(deltaY)
                ? deltaX
                : deltaY;


        int newSize =
            originalSize +
            deltaSize;


        if(newSize < minimumWidth())
        {
            newSize =
                minimumWidth();
        }


        /*
         * Manteniamo fisso il centro.
         */

        const QPoint center =
            m_dragStartGeometry.center();


        newGeometry =
            QRect(
                0,
                0,
                newSize,
                newSize
                );


        newGeometry.moveCenter(
            center
            );
    }


    setGeometry(
        newGeometry
        );


    event->accept();
}



void DistanceGuideCircle::mouseReleaseEvent(
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



void DistanceGuideCircle::setConfigurationMode(
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



int DistanceGuideCircle::configurationPositionX() const
{
    return geometry().center().x();
}



int DistanceGuideCircle::configurationPositionY() const
{
    return geometry().center().y();
}



int DistanceGuideCircle::configurationSize() const
{
    return width();
}