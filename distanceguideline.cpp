#include "distanceguideline.h"

#include <QMouseEvent>
#include <QPainter>

DistanceGuideLine::DistanceGuideLine(
    const QColor &color,
    QWidget *parent
    )
    : QWidget(parent),
    m_color(color),
    m_configurationMode(false),
    m_dragging(false)
{
    setFixedWidth(5);

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

    setCursor(
        Qt::SizeHorCursor
        );
}
void DistanceGuideLine::paintEvent(
    QPaintEvent *
    )
{
    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing,
        false
        );

    painter.setPen(
        Qt::NoPen
        );

    painter.setBrush(
        m_color
        );

    painter.drawRect(
        rect()
        );
}
void DistanceGuideLine::mousePressEvent(
    QMouseEvent *event
    )
{
    if(!m_configurationMode)
        return;

    if(event->button() != Qt::LeftButton)
        return;

    m_dragging = true;

    m_dragOffsetX =
        event->globalPosition().x() -
        frameGeometry().left();

    event->accept();
}
void DistanceGuideLine::mouseMoveEvent(
    QMouseEvent *event
    )
{
    if(!m_configurationMode)
        return;

    if(!m_dragging)
        return;

    const int newX =
        event->globalPosition().x() -
        m_dragOffsetX;

    move(
        newX,
        y()
        );

    event->accept();
}

void DistanceGuideLine::mouseReleaseEvent(
    QMouseEvent *event
    )
{
    if(event->button() != Qt::LeftButton)
        return;

    m_dragging = false;

    event->accept();
}
void DistanceGuideLine::setConfigurationMode(
    bool enabled
    )
{
    m_configurationMode =
        enabled;


    setCursor(
        enabled
            ? Qt::SizeHorCursor
            : Qt::ArrowCursor
        );
}

int DistanceGuideLine::configurationPositionX() const
{
    return x();
}