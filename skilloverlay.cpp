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


    setAttribute(
        Qt::WA_TranslucentBackground
        );


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
            30,
            this
            );



    // =========================
    // CONTROLLO CTRL
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::keyPressed,
        this,
        [this](int key)
        {

            if(!trackingActive)
            {
                checkSequences(key);
                return;
            }


            if(key == '6' &&
                sequenceState == SequenceState::WaitingG)
            {
                artifact->startCooldown();

                if(currentTitle == ActiveTitle::SettingSun)
                {
                    settingSun->startCooldown();
                }
            }


            // Night Parade con F/T
            if(sequenceState == SequenceState::WaitingG &&
                currentTitle == ActiveTitle::NightParade)
            {
                if(key == 'F' || key == 'T')
                {
                    nightParade->startCooldown();
                }
            }


            checkSequences(key);

        },
        Qt::QueuedConnection
        );

    connect(
        keyboard,
        &GlobalKeyboard::ctrlPressed,
        this,
        [this]()
        {

            // Primo awakening
            if(!trackingActive)
            {

                trackingActive = true;

                artifact->startCooldown();

                concerto->startCooldown();

                return;

            }



            // Artifact sempre con CTRL

            artifact->startCooldown();



            // Titolo selezionato

            switch(currentTitle)
            {

            case ActiveTitle::Concerto:

                concerto->startCooldown();

                break;


            case ActiveTitle::SettingSun:
                settingSun->startCooldown();

                break;


            case ActiveTitle::NightParade:

                // Night Parade non parte con CTRL.
                // Si attiva esclusivamente con F/T.

                break;


            case ActiveTitle::Other:
            case ActiveTitle::None:

                break;

            }

        },
        Qt::QueuedConnection
        );




    // =========================
    // RESET CTRL DESTRO
    // =========================

    connect(
        keyboard,
        &GlobalKeyboard::resetPressed,
        this,
        [this]()
        {

            trackingActive = false;

            sequenceState =
                SequenceState::WaitingG;


            selectedDirection = 0;


            currentTitle =
                ActiveTitle::None;


            concerto->resetCooldown();
            artifact->resetCooldown();
            nightParade->resetCooldown();
            settingSun->resetCooldown();

        },
        Qt::QueuedConnection
        );
    connect(
        keyboard,
        &GlobalKeyboard::transcendenceResetPressed,
        this,
        [this]()
        {
            trackingActive = false;

        },
        Qt::QueuedConnection
        );




    // posizione croce

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

        sequenceState =
            SequenceState::WaitingG;

        return;

    }



    switch(sequenceState)
    {


        // =========================
        // ASPETTA G
        // =========================

    case SequenceState::WaitingG:


        if(key == 'G')
        {

            sequenceState =
                SequenceState::WaitingDirection;

        }

        break;



        // =========================
        // SCELTA TITOLO
        // =========================

    case SequenceState::WaitingDirection:


        if(key == VK_UP)
        {

            currentTitle =
                ActiveTitle::Concerto;

        }


        else if(key == VK_LEFT)
        {

            currentTitle =
                ActiveTitle::NightParade;

        }


        else if(key == VK_DOWN)
        {

            currentTitle =
                ActiveTitle::SettingSun;

        }


        else if(key == VK_RIGHT)
        {

            currentTitle =
                ActiveTitle::Other;

        }


        else if(key == 'G')
        {
            // rimane in attesa

            break;
        }


        else
        {
            break;
        }



        selectedDirection = key;


        sequenceState =
            SequenceState::WaitingG;


        break;

    }

}