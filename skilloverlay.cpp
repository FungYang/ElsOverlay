#include "skilloverlay.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QSettings>


SkillOverlay::SkillOverlay(
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    keyboard(keyboard)
{
    setFixedSize(
        180,
        165
        );


    setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Tool |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    // ========================================================
    // POSIZIONE
    // ========================================================

    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    QPoint savedPosition =
        settings.value(
                    "Overlay/BuffGroup/position",
                    QPoint(100, 100)
                    ).toPoint();


    move(
        savedPosition
        );


    // ========================================================
    // CONFIGURAZIONE
    // ========================================================

    loadDefaultConfig();


    // ========================================================
    // SKILL
    // ========================================================

    upSkill =
        new SkillBox(
            config.up.imagePath,
            config.up.name,
            config.up.cooldown,
            this
            );


    leftSkill =
        new SkillBox(
            config.left.imagePath,
            config.left.name,
            config.left.cooldown,
            this
            );


    downSkill =
        new SkillBox(
            config.down.imagePath,
            config.down.name,
            config.down.cooldown,
            this
            );


    rightSkill =
        new SkillBox(
            config.right.imagePath,
            config.right.name,
            config.right.cooldown,
            this
            );


    // ========================================================
    // POSIZIONAMENTO
    // ========================================================

    upSkill->move(
        60,
        0
        );


    leftSkill->move(
        20,
        50
        );


    downSkill->move(
        60,
        100
        );


    rightSkill->move(
        100,
        50
        );


    // ========================================================
    // TIMER
    // ========================================================

    timer =
        new QTimer(this);


    connect(
        timer,
        &QTimer::timeout,
        this,
        [this]()
        {
            upSkill->tick();

            leftSkill->tick();

            downSkill->tick();

            rightSkill->tick();
        }
        );


    timer->start(
        1000
        );


    // ========================================================
    // KEYBOARD
    // ========================================================

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


            // ------------------------------------------------
            // CIPOLLA
            // ------------------------------------------------

            if(
                key == config.cipollaKey &&
                sequenceState ==
                    SequenceState::WaitingStateKey
                )
            {
                activateCipollaSkill();
            }


            // ------------------------------------------------
            // COMBO
            // ------------------------------------------------

            activateComboSkill(
                key
                );


            // ------------------------------------------------
            // SEQUENZA STATO
            // ------------------------------------------------

            checkSequences(
                key
                );
        },
        Qt::QueuedConnection
        );


    // ========================================================
    // CTRL SINISTRO
    // ========================================================

    connect(
        keyboard,
        &GlobalKeyboard::ctrlPressed,
        this,
        [this]()
        {
            activateCtrlSkill();
        },
        Qt::QueuedConnection
        );


    // ========================================================
    // CTRL DESTRO / RESET
    // ========================================================

    connect(
        keyboard,
        &GlobalKeyboard::resetPressed,
        this,
        [this]()
        {
            trackingActive = true;


            sequenceState =
                SequenceState::WaitingStateKey;


            currentDirection =
                Direction::None;


            resetAllCooldowns();

        },
        Qt::QueuedConnection
        );


    // ========================================================
    // RESET TRASCENDENZA
    // ========================================================

    connect(
        keyboard,
        &GlobalKeyboard::transcendenceResetPressed,
        this,
        [this]()
        {
            trackingActive = true;
        }
        );
}


// ============================================================
// DEFAULT CONFIG
// ============================================================

void SkillOverlay::loadDefaultConfig()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    // --------------------------------------------------------
    // TASTI PRINCIPALI
    // --------------------------------------------------------

    config.stateKey =
        settings.value(
                    "BuffTitles/StateKey",
                    'G'
                    ).toInt();


    config.cipollaKey =
        settings.value(
                    "BuffTitles/CipollaKey",
                    '6'
                    ).toInt();


    // --------------------------------------------------------
    // UP
    // --------------------------------------------------------

    config.up.name =
        settings.value(
                    "BuffTitles/Up/Name",
                    "Concerto"
                    ).toString();


    config.up.imagePath =
        settings.value(
                    "BuffTitles/Up/Image",
                    "images/concerto.png"
                    ).toString();


    config.up.cooldown =
        settings.value(
                    "BuffTitles/Up/Cooldown",
                    60
                    ).toInt();


    config.up.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Up/Activation",
                        static_cast<int>(
                            SkillActivation::CtrlCipolla
                            )
                        ).toInt()
            );


    // --------------------------------------------------------
    // LEFT
    // --------------------------------------------------------

    config.left.name =
        settings.value(
                    "BuffTitles/Left/Name",
                    "Night Parade"
                    ).toString();


    config.left.imagePath =
        settings.value(
                    "BuffTitles/Left/Image",
                    "images/nightparade.png"
                    ).toString();


    config.left.cooldown =
        settings.value(
                    "BuffTitles/Left/Cooldown",
                    25
                    ).toInt();


    config.left.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Left/Activation",
                        static_cast<int>(
                            SkillActivation::Combo
                            )
                        ).toInt()
            );


    // --------------------------------------------------------
    // DOWN
    // --------------------------------------------------------

    config.down.name =
        settings.value(
                    "BuffTitles/Down/Name",
                    "Setting Sun"
                    ).toString();


    config.down.imagePath =
        settings.value(
                    "BuffTitles/Down/Image",
                    "images/settingsun.png"
                    ).toString();


    config.down.cooldown =
        settings.value(
                    "BuffTitles/Down/Cooldown",
                    30
                    ).toInt();


    config.down.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Down/Activation",
                        static_cast<int>(
                            SkillActivation::CtrlCipolla
                            )
                        ).toInt()
            );


    // --------------------------------------------------------
    // RIGHT
    // --------------------------------------------------------

    config.right.name =
        settings.value(
                    "BuffTitles/Right/Name",
                    "Other"
                    ).toString();


    config.right.imagePath =
        settings.value(
                    "BuffTitles/Right/Image",
                    "images/artifact.png"
                    ).toString();


    config.right.cooldown =
        settings.value(
                    "BuffTitles/Right/Cooldown",
                    0
                    ).toInt();


    config.right.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Right/Activation",
                        static_cast<int>(
                            SkillActivation::Combo
                            )
                        ).toInt()
            );


    // --------------------------------------------------------
    // COMBO KEYS PER SKILL
    // --------------------------------------------------------

    auto loadComboKeys =
        [&settings](
            const QString &prefix,
            QList<int> &keys,
            const QList<int> &defaults
            )
    {
        keys.clear();


        int size =
            settings.beginReadArray(
                prefix + "/ComboKeys"
                );


        if(size == 0)
        {
            keys = defaults;
        }
        else
        {
            for(int i = 0;
                 i < size;
                 ++i)
            {
                settings.setArrayIndex(i);


                int key =
                    settings.value(
                                "Key",
                                0
                                ).toInt();


                if(key != 0)
                {
                    keys.append(key);
                }
            }
        }


        settings.endArray();
    };


    loadComboKeys(
        "BuffTitles/Up",
        config.up.comboKeys,
        {}
        );


    loadComboKeys(
        "BuffTitles/Left",
        config.left.comboKeys,
        { 'F', 'T' }
        );


    loadComboKeys(
        "BuffTitles/Down",
        config.down.comboKeys,
        {}
        );


    loadComboKeys(
        "BuffTitles/Right",
        config.right.comboKeys,
        {}
        );
}


// ============================================================
// APPLY CONFIG
// ============================================================

void SkillOverlay::applyConfig(
    const SkillOverlayConfig &newConfig
    )
{
    config =
        newConfig;


    updateSkillBoxes();
}


// ============================================================
// UPDATE SKILL BOXES
// ============================================================

void SkillOverlay::updateSkillBoxes()
{
    upSkill->setSkillName(
        config.up.name
        );


    upSkill->setImage(
        config.up.imagePath
        );


    upSkill->setCooldown(
        config.up.cooldown
        );


    leftSkill->setSkillName(
        config.left.name
        );


    leftSkill->setImage(
        config.left.imagePath
        );


    leftSkill->setCooldown(
        config.left.cooldown
        );


    downSkill->setSkillName(
        config.down.name
        );


    downSkill->setImage(
        config.down.imagePath
        );


    downSkill->setCooldown(
        config.down.cooldown
        );


    rightSkill->setSkillName(
        config.right.name
        );


    rightSkill->setImage(
        config.right.imagePath
        );


    rightSkill->setCooldown(
        config.right.cooldown
        );
}


// ============================================================
// CURRENT SKILL
// ============================================================

SkillConfig *SkillOverlay::currentSkill()
{
    switch(currentDirection)
    {
    case Direction::Up:
        return &config.up;


    case Direction::Left:
        return &config.left;


    case Direction::Down:
        return &config.down;


    case Direction::Right:
        return &config.right;


    case Direction::None:
        break;
    }


    return nullptr;
}


// ============================================================
// CTRL SINISTRO
// ============================================================

void SkillOverlay::activateCtrlSkill()
{
    // --------------------------------------------------------
    // Primo CTRL: avvia il tracking
    // --------------------------------------------------------

    if(!trackingActive)
    {
        trackingActive = true;

        // Primo CTRL:
        // attiva Concerto
        // upSkill->startCooldown();

        // Attiva anche l'Artifact, ovunque sia configurato
        if(config.up.activation == SkillActivation::Artifact)
            upSkill->startCooldown();

        if(config.left.activation == SkillActivation::Artifact)
            leftSkill->startCooldown();

        if(config.down.activation == SkillActivation::Artifact)
            downSkill->startCooldown();

        if(config.right.activation == SkillActivation::Artifact)
            rightSkill->startCooldown();

        //return;
    }


    // --------------------------------------------------------
    // CTRL DURANTE IL TRACKING
    // --------------------------------------------------------
    //
    // ARTIFACT:
    // è globale durante il tracking.
    //
    // Se QUALSIASI dei 4 box è configurato come Artifact,
    // CTRL lo attiva indipendentemente dalla direzione corrente.
    // --------------------------------------------------------

    if(
        config.up.activation ==
        SkillActivation::Artifact
        )
    {
        upSkill->startCooldown();
    }


    if(
        config.left.activation ==
        SkillActivation::Artifact
        )
    {
        leftSkill->startCooldown();
    }


    if(
        config.down.activation ==
        SkillActivation::Artifact
        )
    {
        downSkill->startCooldown();
    }


    if(
        config.right.activation ==
        SkillActivation::Artifact
        )
    {
        rightSkill->startCooldown();
    }


    // --------------------------------------------------------
    // SKILL DELLA DIREZIONE CORRENTE
    // --------------------------------------------------------

    SkillConfig *skill =
        currentSkill();


    if(!skill)
    {
        return;
    }


    // --------------------------------------------------------
    // CTRL / CIPOLLA
    // --------------------------------------------------------

    if(
        skill->activation ==
        SkillActivation::CtrlCipolla
        )
    {
        switch(currentDirection)
        {
        case Direction::Up:
            upSkill->startCooldown();
            break;


        case Direction::Left:
            leftSkill->startCooldown();
            break;


        case Direction::Down:
            downSkill->startCooldown();
            break;


        case Direction::Right:
            rightSkill->startCooldown();
            break;


        case Direction::None:
            break;
        }
    }
}


// ============================================================
// CIPOLLA
// ============================================================

void SkillOverlay::activateCipollaSkill()
{
    SkillConfig *skill =
        currentSkill();


    if(!skill)
    {
        return;
    }


    if(
        skill->activation !=
        SkillActivation::CtrlCipolla
        )
    {
        return;
    }


    switch(currentDirection)
    {
    case Direction::Up:
        upSkill->startCooldown();
        break;


    case Direction::Left:
        leftSkill->startCooldown();
        break;


    case Direction::Down:
        downSkill->startCooldown();
        break;


    case Direction::Right:
        rightSkill->startCooldown();
        break;


    case Direction::None:
        break;
    }
}


// ============================================================
// COMBO
// ============================================================

void SkillOverlay::activateComboSkill(
    int key
    )
{
    SkillConfig *skill =
        currentSkill();


    if(!skill)
    {
        return;
    }


    // --------------------------------------------------------
    // COMBO RIMANE LEGATO ALLA DIREZIONE CORRENTE
    // --------------------------------------------------------

    if(
        skill->activation !=
        SkillActivation::Combo
        )
    {
        return;
    }


    if(
        !skill->comboKeys.contains(key)
        )
    {
        return;
    }


    switch(currentDirection)
    {
    case Direction::Up:
        upSkill->startCooldown();
        break;


    case Direction::Left:
        leftSkill->startCooldown();
        break;


    case Direction::Down:
        downSkill->startCooldown();
        break;


    case Direction::Right:
        rightSkill->startCooldown();
        break;


    case Direction::None:
        break;
    }
}


// ============================================================
// SEQUENCES
// ============================================================

void SkillOverlay::checkSequences(
    int key
    )
{
    if(!trackingActive)
    {
        sequenceState =
            SequenceState::WaitingStateKey;

        //return;
    }


    // ========================================================
    // ATTESA TASTO STATO
    // ========================================================

    if(
        sequenceState ==
        SequenceState::WaitingStateKey
        )
    {
        if(key == config.stateKey)
        {
            sequenceState =
                SequenceState::WaitingDirection;
        }


        return;
    }


    // ========================================================
    // ATTESA DIREZIONE
    // ========================================================

    if(
        sequenceState ==
        SequenceState::WaitingDirection
        )
    {
        // Le frecce rimangono FISSE.

        if(key == VK_UP)
        {
            currentDirection =
                Direction::Up;
        }
        else if(key == VK_LEFT)
        {
            currentDirection =
                Direction::Left;
        }
        else if(key == VK_DOWN)
        {
            currentDirection =
                Direction::Down;
        }
        else if(key == VK_RIGHT)
        {
            currentDirection =
                Direction::Right;
        }
        else if(key == config.stateKey)
        {
            // Rimane in attesa della direzione.
            return;
        }
        else
        {
            // Sequenza interrotta.
            sequenceState =
                SequenceState::WaitingStateKey;

            return;
        }


        sequenceState =
            SequenceState::WaitingStateKey;
    }
}


// ============================================================
// RESET
// ============================================================

void SkillOverlay::resetAllCooldowns()
{
    upSkill->resetCooldown();


    leftSkill->resetCooldown();


    downSkill->resetCooldown();


    rightSkill->resetCooldown();
}


// ============================================================
// MOUSE PRESS
// ============================================================

void SkillOverlay::mousePressEvent(
    QMouseEvent *event
    )
{
    if(
        event->button() ==
        Qt::LeftButton
        )
    {
        dragPosition =
            event->globalPosition().toPoint()
            - frameGeometry().topLeft();


        event->accept();
    }
}


// ============================================================
// MOUSE MOVE
// ============================================================

void SkillOverlay::mouseMoveEvent(
    QMouseEvent *event
    )
{
    if(
        event->buttons() &
        Qt::LeftButton
        )
    {
        move(
            event->globalPosition().toPoint()
            - dragPosition
            );


        QSettings settings(
            QCoreApplication::applicationDirPath() +
                "/ElsOverlay.ini",
            QSettings::IniFormat
            );


        settings.setValue(
            "Overlay/BuffGroup/position",
            pos()
            );


        settings.sync();
    }
}