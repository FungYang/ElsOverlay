#include "buffvisionselector.h"

#include <QPainter>
#include <QMouseEvent>
#include <QSettings>


BuffVisionSelector::BuffVisionSelector(
    QWidget *parent
    )
    : QWidget(parent)
{

    setFixedSize(
        150,
        90
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    move(
        settings.value(
                    "Overlay/BuffVisionSelector/position",
                    QPoint(500,100)
                    ).toPoint()
        );


    flowImage.load(
        "images/Flow.png"
        );



    flowRect =
        QRect(
            15,
            15,
            55,
            55
            );


    disableRect =
        QRect(
            90,
            15,
            55,
            55
            );

}



void BuffVisionSelector::paintEvent(
    QPaintEvent *
    )
{

    QPainter p(this);


    p.setRenderHint(
        QPainter::Antialiasing
        );



    p.setBrush(
        QColor(
            0,
            0,
            0,
            170
            )
        );


    p.setPen(
        Qt::NoPen
        );


    p.drawRoundedRect(
        rect(),
        10,
        10
        );



    // Flow

    p.drawPixmap(
        flowRect,
        flowImage
        );



    // X rossa di fianco

    QPen pen;

    pen.setColor(
        Qt::red
        );

    pen.setWidth(
        6
        );


    p.setPen(
        pen
        );


    p.drawLine(
        disableRect.topLeft(),
        disableRect.bottomRight()
        );


    p.drawLine(
        disableRect.topRight(),
        disableRect.bottomLeft()
        );

}



void BuffVisionSelector::mousePressEvent(
    QMouseEvent *event
    )
{

    QPoint pos =
        event->position().toPoint();



    if(flowRect.contains(pos))
    {

        emit enabled();

    }



    if(disableRect.contains(pos))
    {

        emit disabled();

    }

}



void BuffVisionSelector::resetSelector()
{

    show();

}
void BuffVisionSelector::showSelector()
{
    show();
    raise();
}