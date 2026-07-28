#include "skilloverlay.h"
#include <QPoint>
#include <QMouseEvent>


SkillOverlay::SkillOverlay(
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    keyboard(keyboard)
{

    setFixedSize(180,165);

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(Qt::WA_TranslucentBackground);

    setStyleSheet(
        "background: transparent;"
        );
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );

    move(
        settings.value(
                    "Overlay/BuffGroup/position",
                    QPoint(500,300)
                    ).toPoint()
        );

    concerto =
        new SkillBox(
            "images/concerto.png",
            "Concerto",
            60,
            this
            );

    artifact =
        new SkillBox(
            "images/artifact.png",
            "Artifact",
            20,
            this
            );


    nightParade =
        new SkillBox(
            "images/nightparade.png",
            "NightParade",
            25,
            this
            );


    settingSun =
        new SkillBox(
            "images/settingsun.png",
            "SettingSun",
            25,
            this
            );
    connect(
        keyboard,
        &GlobalKeyboard::ctrlPressed,
        this,
        [this]()
        {
            trackingActive = true;

            artifact->startCooldown();
            concerto->startCooldown();
        }
        );

    connect(
        keyboard,
        &GlobalKeyboard::resetPressed,
        this,
        [this]()
        {
            trackingActive = false;

            keyBuffer.clear();

            concerto->resetCooldown();
            artifact->resetCooldown();
            nightParade->resetCooldown();
            settingSun->resetCooldown();
        }
        );
    // disposizione a croce

    concerto->move(60,0);


    nightParade->move(20,50);


    artifact->move(100,50);


    settingSun->move(60,100);
    timer = new QTimer(this);

    connect(
        timer,
        &QTimer::timeout,
        this,
        [this]()
        {
            concerto->tick();
            artifact->tick();
            nightParade->tick();
            settingSun->tick();
        }
        );
    connect(
        keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {
            if(key == '6')
            {
                artifact->startCooldown();
            }


            keyBuffer.append(key);


            if(keyBuffer.size() > 3)
            {
                keyBuffer.removeFirst();
            }


            checkSequences();
        }
        );


    timer->start(1000);

}
void SkillOverlay::mousePressEvent(QMouseEvent *event)
{

    dragPosition =
        event->globalPosition().toPoint()
        - frameGeometry().topLeft();

}



void SkillOverlay::mouseMoveEvent(QMouseEvent *event)
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
        "Overlay/BuffGroup/position",
        pos()
        );

}

void SkillOverlay::resetAllCooldowns()
{
    concerto->resetCooldown();
    artifact->resetCooldown();
    nightParade->resetCooldown();
    settingSun->resetCooldown();
}

void SkillOverlay::checkSequences()
{
    if(!trackingActive)
    {
        keyBuffer.clear();
        return;
    }

    if(keyBuffer.size() > 3)
    {
        keyBuffer.removeFirst();
    }


    if(keyBuffer.size() == 3)
    {

        // Concerto: G + Freccia Su + LCTRL
        if(
            keyBuffer[0] == 'G' &&
            keyBuffer[1] == VK_UP &&
            keyBuffer[2] == VK_LCONTROL
            )
        {
            concerto->startCooldown();

            keyBuffer.clear();
            return;
        }


        // Concerto: G + Freccia Su + 6
        if(
            keyBuffer[0] == 'G' &&
            keyBuffer[1] == VK_UP &&
            keyBuffer[2] == '6'
            )
        {
            concerto->startCooldown();

            keyBuffer.clear();
            return;
        }


        // NightParade: G + Freccia Sinistra + F
        if(
            keyBuffer[0] == 'G' &&
            keyBuffer[1] == VK_LEFT &&
            keyBuffer[2] == 'F'
            )
        {
            nightParade->startCooldown();

            keyBuffer.clear();
            return;
        }


        // NightParade: G + Freccia Sinistra + T
        if(
            keyBuffer[0] == 'G' &&
            keyBuffer[1] == VK_LEFT &&
            keyBuffer[2] == 'T'
            )
        {
            nightParade->startCooldown();

            keyBuffer.clear();
            return;
        }


        // SettingSun: G + Freccia Giù + LCTRL
        if(
            keyBuffer[0] == 'G' &&
            keyBuffer[1] == VK_DOWN &&
            keyBuffer[2] == VK_LCONTROL
            )
        {
            settingSun->startCooldown();

            keyBuffer.clear();
            return;
        }


        // SettingSun: G + Freccia Giù + 6
        if(
            keyBuffer[0] == 'G' &&
            keyBuffer[1] == VK_DOWN &&
            keyBuffer[2] == '6'
            )
        {
            settingSun->startCooldown();

            keyBuffer.clear();
            return;
        }

    }

}