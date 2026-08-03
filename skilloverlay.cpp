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
            if(trackingActive){
                artifact->startCooldown();
                return;
            }

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

            sequenceState = SequenceState::WaitingG;
            selectedDirection = 0;

            concerto->resetCooldown();
            artifact->resetCooldown();
            nightParade->resetCooldown();
            settingSun->resetCooldown();
        }
        );
    // disposizione a croceg

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
            if(key == '6' &&
                sequenceState == SequenceState::WaitingG &&
                trackingActive)
            {
                artifact->startCooldown();
            }

            checkSequences(key);
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

void SkillOverlay::checkSequences(int key)
{
    if(!trackingActive)
    {
        sequenceState = SequenceState::WaitingG;
        return;
    }


    switch(sequenceState)
    {

    case SequenceState::WaitingG:

        if(key == 'G')
        {
            sequenceState = SequenceState::WaitingDirection;
        }

        break;



    case SequenceState::WaitingDirection:

        if(key == VK_UP ||
            key == VK_LEFT ||
            key == VK_DOWN)
        {
            selectedDirection = key;
            sequenceState = SequenceState::WaitingKey;
        }
        else if(key == VK_RIGHT)
        {
            // G + freccia destra non è una combo valida
            // resetta la sequenza
            sequenceState = SequenceState::WaitingG;
            selectedDirection = 0;
        }
        else if(key == 'G')
        {
            // se ripremo G rimaniamo in attesa della direzione
            sequenceState = SequenceState::WaitingDirection;
        }

        break;


    case SequenceState::WaitingKey:


        // =========================
        // CONCERTO
        // G + ↑ + CTRL / 6
        // =========================

        if(selectedDirection == VK_UP)
        {
            if(key == VK_LCONTROL ||
                key == '6')
            {
                concerto->startCooldown();

                sequenceState = SequenceState::WaitingG;
            }
        }



        // =========================
        // NIGHT PARADE
        // G + ← + F / T
        // =========================

        else if(selectedDirection == VK_LEFT)
        {
            if(key == 'F' ||
                key == 'T')
            {
                nightParade->startCooldown();

                sequenceState = SequenceState::WaitingG;
            }
        }



        // =========================
        // SETTING SUN
        // G + ↓ + CTRL / 6
        // =========================

        else if(selectedDirection == VK_DOWN)
        {
            if(key == VK_LCONTROL ||
                key == '6')
            {
                settingSun->startCooldown();

                sequenceState = SequenceState::WaitingG;
            }
        }


        break;

    }
}