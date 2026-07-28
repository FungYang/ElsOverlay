#include "overlay.h"

#include <QPainter>
#include <QMouseEvent>


Overlay::Overlay(QWidget *parent)
    : QWidget(parent)
{

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    resize(130,40);


    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );

    QPoint savedPosition =
        settings.value(
                    "Overlay/Transcendence/position",
                    QPoint(500,300)
                    ).toPoint();

    move(savedPosition);



    connect(
        &timer,
        &QTimer::timeout,
        this,
        [this]()
        {
            if(running)
            {
                cooldown--;

                if(cooldown <= 0)
                {
                    cooldown = 20;
                }

                update();
            }
        }
        );


    timer.start(1000);
}



void Overlay::paintEvent(QPaintEvent *)
{

    QPainter p(this);

    p.setRenderHint(
        QPainter::Antialiasing
        );


    p.setBrush(
        QColor(0,0,0,170)
        );

    p.setPen(Qt::NoPen);


    p.drawRoundedRect(
        rect(),
        10,
        10
        );


    QFont font;
    font.setPointSize(18);
    font.setBold(true);

    p.setFont(font);

    p.setPen(Qt::white);

    p.drawText(
        rect(),
        Qt::AlignCenter,
        QString("Trasc: %1").arg(cooldown)
        );
}



void Overlay::mousePressEvent(QMouseEvent *event)
{
    dragPosition =
        event->globalPosition().toPoint()
        - frameGeometry().topLeft();
}



void Overlay::mouseMoveEvent(QMouseEvent *event)
{
    move(
        event->globalPosition().toPoint()
        - dragPosition
        );


    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );

    settings.setValue(
        "Overlay/Transcendence/position",
        pos()
        );
}

void Overlay::resetCooldown()
{
    running = false;
    cooldown = 20;
    update();
}

void Overlay::startCooldown()
{
    running = true;
}