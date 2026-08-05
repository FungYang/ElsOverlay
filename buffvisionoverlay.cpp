#include "buffvisionoverlay.h"

#include <QPainter>


BuffVisionOverlay::BuffVisionOverlay(
    BuffVisionCore *core,
    QWidget *parent
    )
    : QWidget(parent),
    core(core)
{

    setFixedSize(80,80);


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
                    "Overlay/BuffVision/position",
                    QPoint(700,300)
                    ).toPoint()
        );



    buffImage.load(
        "images/Flow.png"
        );
    grayBuffImage =
        createGrayImage(
            buffImage
            );


    connect(
        core,
        &BuffVisionCore::buff15Activated,
        this,
        [this]()
        {
            cooldown15 = 15;
            update();
        }
        );


    connect(
        core,
        &BuffVisionCore::buff15Expired,
        this,
        [this]()
        {
            cooldown15 = 0;
            update();
        }
        );
    connect(
        core,
        &BuffVisionCore::buff60Activated,
        this,
        [this]()
        {
            update();
        }
        );


    connect(
        core,
        &BuffVisionCore::buff60Expired,
        this,
        [this]()
        {
            update();
        }
        );



    connect(
        &timer,
        &QTimer::timeout,
        this,
        [this]()
        {

            refreshCounter += 100;


            if(refreshCounter >= 1000)
            {

                refreshCounter = 0;


                if(cooldown15 > 0)
                {
                    cooldown15--;
                }

            }


            update();

        }
        );



    timer.start(100);

}



void BuffVisionOverlay::paintEvent(QPaintEvent *)
{

    QPainter p(this);


    p.setRenderHint(
        QPainter::Antialiasing
        );



    const QPixmap &image =
        core->isBuff60Active()
            ? buffImage
            : grayBuffImage;



    p.drawPixmap(
        10,
        10,
        55,
        55,
        image
        );



    if(cooldown15 > 0)
    {

        QFont font;

        font.setBold(true);
        font.setPointSize(22);


        p.setFont(font);

        p.setPen(
            Qt::white
            );


        p.drawText(
            rect(),
            Qt::AlignCenter,
            QString::number(cooldown15)
            );

    }


}



QPixmap BuffVisionOverlay::createGrayImage(
    const QPixmap &source
    )
{

    QImage img =
        source.toImage()
            .convertToFormat(
                QImage::Format_ARGB32
                );


    for(int y=0;y<img.height();y++)
    {
        for(int x=0;x<img.width();x++)
        {

            QColor c =
                img.pixelColor(
                    x,
                    y
                    );


            int gray =
                qGray(c.rgb());


            img.setPixelColor(
                x,
                y,
                QColor(
                    gray,
                    gray,
                    gray,
                    c.alpha()
                    )
                );

        }
    }


    return QPixmap::fromImage(img);

}



void BuffVisionOverlay::mousePressEvent(
    QMouseEvent *event
    )
{

    dragPosition =
        event->globalPosition().toPoint()
        - frameGeometry().topLeft();

}



void BuffVisionOverlay::mouseMoveEvent(
    QMouseEvent *event
    )
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
        "Overlay/BuffVision/position",
        pos()
        );

}

void BuffVisionOverlay::resetOverlay()
{

    buff60Active = false;

    cooldown15 = 0;


    update();

}

void BuffVisionOverlay::setScores(
    double s1,
    double s2
    )
{
    score1 = s1;
    score2 = s2;

    update();
}