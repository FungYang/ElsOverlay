#include "skilloverlay.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QSettings>
#include <QtGlobal>


SkillOverlay::SkillOverlay(
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    keyboard(keyboard)
{
    // ========================================================
    // CONFIGURAZIONE
    // ========================================================

    loadDefaultConfig();


    // ========================================================
    // SETTINGS
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


    // La scala viene caricata da loadDefaultConfig().
    // Qui usiamo direttamente il valore presente nella config.

    scale =
        config.scale;


    // Protezione da valori non validi.

    if(scale <= 0.0)
    {
        scale =
            1.0;
    }


    // Limite minimo e massimo.

    scale =
        qBound(
            0.50,
            scale,
            3.00
            );


    // Manteniamo config.scale sincronizzato.

    config.scale =
        scale;


    // ========================================================
    // FINESTRA
    // ========================================================

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

    move(
        savedPosition
        );


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
    // SCALA
    // ========================================================

    upSkill->setScale(
        scale
        );


    leftSkill->setScale(
        scale
        );


    downSkill->setScale(
        scale
        );


    rightSkill->setScale(
        scale
        );


    // ========================================================
    // POSIZIONAMENTO E DIMENSIONE
    // ========================================================

    updateOverlayGeometry();


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

    if(keyboard)
    {
        connect(
            keyboard,
            &GlobalKeyboard::keyPressed,
            this,
            [this](int key)
            {
                if(!trackingActive)
                {
                    checkSequences(
                        key
                        );

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


        // ====================================================
        // CTRL SINISTRO
        // ====================================================

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


        // ====================================================
        // CTRL DESTRO / RESET
        // ====================================================

        connect(
            keyboard,
            &GlobalKeyboard::resetPressed,
            this,
            [this]()
            {
                trackingActive =
                    true;


                sequenceState =
                    SequenceState::WaitingStateKey;


                currentDirection =
                    Direction::None;


                resetAllCooldowns();
            },
            Qt::QueuedConnection
            );


        // ====================================================
        // RESET TRASCENDENZA
        // ====================================================

        connect(
            keyboard,
            &GlobalKeyboard::transcendenceResetPressed,
            this,
            [this]()
            {
                trackingActive =
                    true;
            }
            );
    }
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
    // SCALA
    // --------------------------------------------------------

    config.scale =
        settings.value(
                    "Overlay/BuffGroup/Scale",
                    1.0
                    ).toDouble();


    if(config.scale <= 0.0)
    {
        config.scale =
            1.0;
    }


    config.scale =
        qBound(
            0.50,
            config.scale,
            3.00
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
    // COMBO KEYS
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
            keys =
                defaults;
        }
        else
        {
            for(
                int i = 0;
                i < size;
                ++i
                )
            {
                settings.setArrayIndex(
                    i
                    );


                int key =
                    settings.value(
                                "Key",
                                0
                                ).toInt();


                if(key != 0)
                {
                    keys.append(
                        key
                        );
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
// SCALE
// ============================================================

void SkillOverlay::setScale(
    double newScale
    )
{
    if(newScale <= 0.0)
    {
        newScale =
            1.0;
    }


    newScale =
        qBound(
            0.50,
            newScale,
            3.00
            );


    scale =
        newScale;


    // Manteniamo la configurazione sincronizzata.

    config.scale =
        scale;


    // --------------------------------------------------------
    // SKILL BOX
    // --------------------------------------------------------

    if(upSkill)
    {
        upSkill->setScale(
            scale
            );
    }


    if(leftSkill)
    {
        leftSkill->setScale(
            scale
            );
    }


    if(downSkill)
    {
        downSkill->setScale(
            scale
            );
    }


    if(rightSkill)
    {
        rightSkill->setScale(
            scale
            );
    }


    // --------------------------------------------------------
    // GEOMETRIA OVERLAY
    // --------------------------------------------------------

    updateOverlayGeometry();


    // --------------------------------------------------------
    // SALVATAGGIO
    // --------------------------------------------------------

    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    settings.setValue(
        "Overlay/BuffGroup/Scale",
        scale
        );


    settings.sync();
}


double SkillOverlay::getScale() const
{
    return scale;
}


// ============================================================
// UPDATE OVERLAY GEOMETRY
// ============================================================

void SkillOverlay::updateOverlayGeometry()
{
    int overlayWidth =
        qMax(
            1,
            qRound(
                180.0 * scale
                )
            );


    int overlayHeight =
        qMax(
            1,
            qRound(
                165.0 * scale
                )
            );


    setFixedSize(
        overlayWidth,
        overlayHeight
        );


    // --------------------------------------------------------
    // UP
    // --------------------------------------------------------

    if(upSkill)
    {
        upSkill->move(
            qRound(
                60.0 * scale
                ),
            qRound(
                0.0 * scale
                )
            );
    }


    // --------------------------------------------------------
    // LEFT
    // --------------------------------------------------------

    if(leftSkill)
    {
        leftSkill->move(
            qRound(
                20.0 * scale
                ),
            qRound(
                50.0 * scale
                )
            );
    }


    // --------------------------------------------------------
    // DOWN
    // --------------------------------------------------------

    if(downSkill)
    {
        downSkill->move(
            qRound(
                60.0 * scale
                ),
            qRound(
                100.0 * scale
                )
            );
    }


    // --------------------------------------------------------
    // RIGHT
    // --------------------------------------------------------

    if(rightSkill)
    {
        rightSkill->move(
            qRound(
                100.0 * scale
                ),
            qRound(
                50.0 * scale
                )
            );
    }
}


// ============================================================
// APPLY CONFIG
// ============================================================

void SkillOverlay::applyConfig(
    const SkillOverlayConfig &newConfig
    )
{
    // --------------------------------------------------------
    // CONFIGURAZIONE
    // --------------------------------------------------------

    config =
        newConfig;


    // --------------------------------------------------------
    // SCALA
    // --------------------------------------------------------

    setScale(
        config.scale
        );


    // --------------------------------------------------------
    // SKILL BOXES
    // --------------------------------------------------------

    updateSkillBoxes();
}


// ============================================================
// UPDATE SKILL BOXES
// ============================================================

void SkillOverlay::updateSkillBoxes()
{
    if(!upSkill ||
        !leftSkill ||
        !downSkill ||
        !rightSkill)
    {
        return;
    }


    // --------------------------------------------------------
    // UP
    // --------------------------------------------------------

    upSkill->setSkillName(
        config.up.name
        );


    upSkill->setImage(
        config.up.imagePath
        );


    upSkill->setCooldown(
        config.up.cooldown
        );


    // --------------------------------------------------------
    // LEFT
    // --------------------------------------------------------

    leftSkill->setSkillName(
        config.left.name
        );


    leftSkill->setImage(
        config.left.imagePath
        );


    leftSkill->setCooldown(
        config.left.cooldown
        );


    // --------------------------------------------------------
    // DOWN
    // --------------------------------------------------------

    downSkill->setSkillName(
        config.down.name
        );


    downSkill->setImage(
        config.down.imagePath
        );


    downSkill->setCooldown(
        config.down.cooldown
        );


    // --------------------------------------------------------
    // RIGHT
    // --------------------------------------------------------

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
    if(!trackingActive)
    {
        trackingActive =
            true;


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
    }


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


    SkillConfig *skill =
        currentSkill();


    if(!skill)
    {
        return;
    }


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


    if(
        skill->activation !=
        SkillActivation::Combo
        )
    {
        return;
    }


    if(
        !skill->comboKeys.contains(
            key
            )
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
    }


    if(
        sequenceState ==
        SequenceState::WaitingStateKey
        )
    {
        if(
            key ==
            config.stateKey
            )
        {
            sequenceState =
                SequenceState::WaitingDirection;
        }


        return;
    }


    if(
        sequenceState ==
        SequenceState::WaitingDirection
        )
    {
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
        else if(
            key ==
            config.stateKey
            )
        {
            return;
        }
        else
        {
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
            -
            frameGeometry().topLeft();


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
            -
            dragPosition
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
