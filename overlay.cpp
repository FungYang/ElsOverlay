#include "overlay.h"

#include <QPainter>
#include <QMouseEvent>
#include <QSettings>
#include <QElapsedTimer>


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



    // =========================
    // TIMER DISPLAY
    // =========================

    connect(
        &timer,
        &QTimer::timeout,
        this,
        [this]()
        {

            if(!running)
                return;


            // =========================
            // PAUSA
            // =========================

            if(paused)
                return;



            // =========================
            // TEMPO TRASCORSO
            // =========================

            qint64 elapsed =
                pausedElapsed +
                elapsedTimer.elapsed();



            // =========================
            // SECONDI RIMANENTI
            // =========================

            int remaining =
                20 -
                static_cast<int>(
                    elapsed / 1000
                    );



            // =========================
            // RESET
            // =========================

            if(remaining <= 0)
            {

                cooldown = 20;

                pausedElapsed = 0;

                elapsedTimer.restart();

            }

            else
            {

                cooldown =
                    remaining;

            }



            update();

        }
        );



    // Il QTimer serve soltanto per
    // aggiornare frequentemente il display.
    //
    // Il tempo reale viene misurato
    // da QElapsedTimer.

    timer.start(100);

}



void Overlay::paintEvent(
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



    QFont font;

    font.setPointSize(18);

    font.setBold(true);


    p.setFont(font);


    p.setPen(
        Qt::white
        );


    p.drawText(
        rect(),
        Qt::AlignCenter,
        QString(
            "Trasc: %1"
            ).arg(cooldown)
        );

}



void Overlay::mousePressEvent(
    QMouseEvent *event
    )
{

    dragPosition =
        event->globalPosition().toPoint()
        -
        frameGeometry().topLeft();

}



void Overlay::mouseMoveEvent(
    QMouseEvent *event
    )
{

    move(
        event->globalPosition().toPoint()
        -
        dragPosition
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

    paused = false;

    cooldown = 20;

    pausedElapsed = 0;

    elapsedTimer.invalidate();

    update();

}



void Overlay::startCooldown()
{
    if(!enabled)
        return;

    if(running)
        return;

    running = true;

    paused = false;

    cooldown = 20;

    pausedElapsed = 0;

    elapsedTimer.restart();

    update();
}



void Overlay::togglePause()
{
    if(!enabled)
        return;

    if(!running)
        return;

    if(!paused)
    {
        pausedElapsed +=
            elapsedTimer.elapsed();

        elapsedTimer.invalidate();

        paused = true;
    }
    else
    {
        elapsedTimer.restart();

        paused = false;
    }

    update();
}
void Overlay::restartCooldown()
{
    if(!enabled)
        return;

    if(!running)
        return;

    if(paused)
        return;

    cooldown = 20;

    pausedElapsed = 0;

    elapsedTimer.restart();

    update();
}

void Overlay::setEnabled(bool value)
{
    enabled = value;

    if(!enabled)
    {
        running = false;
        paused = false;

        cooldown = 0;

        pausedElapsed = 0;

        elapsedTimer.invalidate();

        update();

        return;
    }

    // Quando viene riattivato NON parte automaticamente.
    // Rimane semplicemente pronto per una nuova attivazione.

    running = false;
    paused = false;

    cooldown = 0;

    pausedElapsed = 0;

    elapsedTimer.invalidate();

    update();
}