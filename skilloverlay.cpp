#include "skilloverlay.h"

#include "imagepathutil.h"
#include "overlayroot.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QHideEvent>
#include <QScreen>
#include <QSettings>
#include <QShowEvent>
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
    // qDebug()
    //     << "DefaultDirection:"
    //     << config.defaultDirection;
    currentDirection =
        directionFromConfig(
            config.defaultDirection
            );


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

    // La finestra madre non disegna nulla ed e' grande 1x1 px: i box
    // sono finestre indipendenti. Non deve intercettare il mouse.
    setAttribute(
        Qt::WA_TransparentForMouseEvents
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
            nullptr
            );


    leftSkill =
        new SkillBox(
            config.left.imagePath,
            config.left.name,
            config.left.cooldown,
            nullptr
            );


    downSkill =
        new SkillBox(
            config.down.imagePath,
            config.down.name,
            config.down.cooldown,
            nullptr
            );


    rightSkill =
        new SkillBox(
            config.right.imagePath,
            config.right.name,
            config.right.cooldown,
            nullptr
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

    initBoxes();

    updateOverlayGeometry();



    // ========================================================
    // TITOLI ATTIVI / DISATTIVATI
    // ========================================================

    applyEnabledState();


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

            if(cooldownsPaused)
            {
                return;
            }

            upSkill->tick();

            leftSkill->tick();

            downSkill->tick();

            rightSkill->tick();
        }
        );


    timer->start(
        100
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
            // ------------------------------------------------
            // CIPOLLA
            // ------------------------------------------------

            if(key == config.cipollaKey)
            {
                // Come Ctrl: se il tracking non era attivo lo avvio
                trackingActive = true;

                // Skillbox di tipo Artifact: partono sempre
                startArtifactSkills();

                // Skill selezionata di tipo CtrlCipolla
                if(sequenceState == SequenceState::WaitingStateKey)
                {
                    activateCipollaSkill();
                }
            }

            if(!trackingActive)
            {
                checkSequences(key);
                return;
            }

            // ------------------------------------------------
            // COMBO
            // ------------------------------------------------

            activateComboSkill(key);

            // ------------------------------------------------
            // SEQUENZA STATO
            // ------------------------------------------------

            checkSequences(key);
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
                trackingActive = true;
                sequenceState = SequenceState::WaitingStateKey;

                currentDirection =
                    directionFromConfig(
                        config.defaultDirection
                        );

                updateSelectedSkillVisual();

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

        updateSelectedSkillVisual();
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
    config.defaultDirection =
        settings.value(
                    "BuffTitles/DefaultDirection",
                    0
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
        // Nell'ini e' relativo all'exe (o assoluto per file esterni/vecchie
        // versioni). In memoria e' sempre assoluto.
        ImagePath::resolve(
            settings.value(
                        "BuffTitles/Up/Image",
                        "images/concerto.png"
                        ).toString()
            );


    config.up.cooldown =
        settings.value(
                    "BuffTitles/Up/Cooldown",
                    60
                    ).toDouble();


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
        // Nell'ini e' relativo all'exe (o assoluto per file esterni/vecchie
        // versioni). In memoria e' sempre assoluto.
        ImagePath::resolve(
            settings.value(
                        "BuffTitles/Left/Image",
                        "images/nightparade.png"
                        ).toString()
            );


    config.left.cooldown =
        settings.value(
                    "BuffTitles/Left/Cooldown",
                    25
                    ).toDouble();


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
        // Nell'ini e' relativo all'exe (o assoluto per file esterni/vecchie
        // versioni). In memoria e' sempre assoluto.
        ImagePath::resolve(
            settings.value(
                        "BuffTitles/Down/Image",
                        "images/settingsun.png"
                        ).toString()
            );


    config.down.cooldown =
        settings.value(
                    "BuffTitles/Down/Cooldown",
                    30
                    ).toDouble();


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
        // Nell'ini e' relativo all'exe (o assoluto per file esterni/vecchie
        // versioni). In memoria e' sempre assoluto.
        ImagePath::resolve(
            settings.value(
                        "BuffTitles/Right/Image",
                        "images/artifact.png"
                        ).toString()
            );


    config.right.cooldown =
        settings.value(
                    "BuffTitles/Right/Cooldown",
                    0
                    ).toDouble();


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
    // TITOLI ATTIVI (default: tutti attivi)
    // --------------------------------------------------------

    config.up.enabled =
        settings.value(
                    "BuffTitles/Up/Enabled",
                    true
                    ).toBool();

    config.left.enabled =
        settings.value(
                    "BuffTitles/Left/Enabled",
                    true
                    ).toBool();

    config.down.enabled =
        settings.value(
                    "BuffTitles/Down/Enabled",
                    true
                    ).toBool();

    config.right.enabled =
        settings.value(
                    "BuffTitles/Right/Enabled",
                    true
                    ).toBool();


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

// ============================================================
// TRASPARENZA
// ============================================================

void SkillOverlay::setTransparency(
    int value
    )
{
    upSkill->setTransparency(
        value
        );


    leftSkill->setTransparency(
        value
        );


    downSkill->setTransparency(
        value
        );


    rightSkill->setTransparency(
        value
        );
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
    // La finestra madre e' solo il coordinatore: 1x1 px.
    setFixedSize(
        1,
        1
        );


    // I box si posizionano da soli (posizione salvata, oppure
    // disposizione a croce scalata attorno alla finestra madre).
    applyBoxPositions();
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


    // --------------------------------------------------------
    // TITOLI ATTIVI / DISATTIVATI
    // --------------------------------------------------------

    applyEnabledState();


    // Il glow dipende dal titolo selezionato E dal fatto che sia
    // attivo: va ricalcolato quando cambia lo stato di un titolo.
    updateSelectedSkillVisual();
}


// ============================================================
// APPLY ENABLED STATE
// ============================================================

void SkillOverlay::applyEnabledState()
{
    const bool enabled[4] =
    {
        config.up.enabled,
        config.left.enabled,
        config.down.enabled,
        config.right.enabled
    };

    for(int i = 0; i < 4; ++i)
    {
        SkillBox *box =
            boxAt(i);

        if(!box)
            continue;

        box->setTitleEnabled(
            enabled[i]
            );

        // Un box attivo compare solo se l'overlay e' visibile:
        // Buff Titles ON e overlay non nascosti con il tasto globale.
        // (Prima era automatico perche' i box erano figli.)
        if(enabled[i] && isVisible())
        {
            box->show();
        }
    }
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

        updateSelectedSkillVisual();
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
// DISTRUTTORE
// ============================================================

SkillOverlay::~SkillOverlay()
{
    // I box sono finestre indipendenti (senza parent): vanno
    // eliminati a mano, altrimenti resterebbero a schermo.
    delete upSkill;
    delete leftSkill;
    delete downSkill;
    delete rightSkill;

    upSkill = nullptr;
    leftSkill = nullptr;
    downSkill = nullptr;
    rightSkill = nullptr;
}


// ============================================================
// REGISTRA I BOX IN OVERLAYROOT
// ============================================================

void SkillOverlay::registerBoxes(
    OverlayRoot *root
    )
{
    if(!root)
        return;

    for(int i = 0; i < 4; ++i)
    {
        root->registerOverlay(
            boxAt(i)
            );
    }
}


// ============================================================
// VISIBILITA' A CASCATA
//
// OverlayRoot e main.cpp mostrano/nascondono questa finestra come
// prima; i box (finestre indipendenti) la seguono.
// ============================================================

void SkillOverlay::showEvent(
    QShowEvent *event
    )
{
    QWidget::showEvent(event);

    for(int i = 0; i < 4; ++i)
    {
        SkillBox *box =
            boxAt(i);

        if(box && box->isTitleEnabled())
        {
            box->show();
        }
    }
}


void SkillOverlay::hideEvent(
    QHideEvent *event
    )
{
    QWidget::hideEvent(event);

    for(int i = 0; i < 4; ++i)
    {
        if(SkillBox *box = boxAt(i))
        {
            box->hide();
        }
    }
}


// ============================================================
// POSIZIONI DEI BOX
// ============================================================

namespace
{

// Disposizione a croce originale (a scala 1.0), relativa
// all'angolo in alto a sinistra della finestra madre.
const QPoint BOX_DEFAULT_OFFSET[4] =
{
    QPoint(60,   0),    // Up
    QPoint(20,  50),    // Left
    QPoint(60, 100),    // Down
    QPoint(100, 50)     // Right
};

const char *BOX_SETTINGS_KEY[4] =
{
    "Overlay/BuffGroup/position_up",
    "Overlay/BuffGroup/position_left",
    "Overlay/BuffGroup/position_down",
    "Overlay/BuffGroup/position_right"
};

QString boxSettingsPath()
{
    return QCoreApplication::applicationDirPath() +
           "/ElsOverlay.ini";
}

// Un box e' "recuperabile" se almeno una parte sensata e' sopra
// uno schermo (altrimenti, dopo aver cambiato monitor, finirebbe
// fuori dallo schermo senza possibilita' di riprenderlo).
bool isReachable(
    const QRect &rect
    )
{
    const auto screens =
        QGuiApplication::screens();

    for(const QScreen *screen : screens)
    {
        const QRect visible =
            screen->availableGeometry().intersected(
                rect
                );

        if(visible.width() >= 20 &&
            visible.height() >= 20)
        {
            return true;
        }
    }

    return false;
}

}


SkillBox *SkillOverlay::boxAt(
    int index
    ) const
{
    switch(index)
    {
    case 0: return upSkill;
    case 1: return leftSkill;
    case 2: return downSkill;
    case 3: return rightSkill;
    }

    return nullptr;
}


QPoint SkillOverlay::defaultBoxPosition(
    int index
    ) const
{
    const QPoint offset =
        BOX_DEFAULT_OFFSET[index];

    return pos() +
           QPoint(
               qRound(offset.x() * scale),
               qRound(offset.y() * scale)
               );
}


void SkillOverlay::initBoxes()
{
    QSettings settings(
        boxSettingsPath(),
        QSettings::IniFormat
        );

    for(int i = 0; i < 4; ++i)
    {
        SkillBox *box =
            boxAt(i);

        if(!box)
            continue;

        // A fine trascinamento ogni box salva la SUA posizione.
        connect(
            box,
            &SkillBox::dragFinished,
            this,
            [this, i](const QPoint &position)
            {
                saveBoxPosition(
                    i,
                    position
                    );
            }
            );

        // Posizione salvata (se c'e' ed e' ancora raggiungibile).
        m_boxHasPosition[i] = false;

        if(settings.contains(BOX_SETTINGS_KEY[i]))
        {
            const QPoint saved =
                settings.value(
                            BOX_SETTINGS_KEY[i]
                            ).toPoint();

            if(isReachable(QRect(saved, box->size())))
            {
                m_boxPosition[i] = saved;
                m_boxHasPosition[i] = true;
            }
        }
    }
}


void SkillOverlay::applyBoxPositions()
{
    for(int i = 0; i < 4; ++i)
    {
        SkillBox *box =
            boxAt(i);

        if(!box)
            continue;

        box->move(
            m_boxHasPosition[i]
                ? m_boxPosition[i]
                : defaultBoxPosition(i)
            );
    }
}


void SkillOverlay::saveBoxPosition(
    int index,
    const QPoint &position
    )
{
    m_boxPosition[index] = position;
    m_boxHasPosition[index] = true;

    QSettings settings(
        boxSettingsPath(),
        QSettings::IniFormat
        );

    settings.setValue(
        BOX_SETTINGS_KEY[index],
        position
        );

    settings.sync();
}


void SkillOverlay::resetPositions()
{
    QSettings settings(
        boxSettingsPath(),
        QSettings::IniFormat
        );

    for(int i = 0; i < 4; ++i)
    {
        settings.remove(
            BOX_SETTINGS_KEY[i]
            );

        m_boxHasPosition[i] = false;
    }

    settings.sync();

    applyBoxPositions();
}


// ============================================================
// ATMA GATE
// ============================================================

void SkillOverlay::pauseAtmaGate()
{
    cooldownsPaused = true;
    upSkill->pauseCooldown();
    leftSkill->pauseCooldown();
    downSkill->pauseCooldown();
    rightSkill->pauseCooldown();
}


void SkillOverlay::resumeAtmaGate()
{
    cooldownsPaused = false;
    upSkill->resumeCooldown();
    leftSkill->resumeCooldown();
    downSkill->resumeCooldown();
    rightSkill->resumeCooldown();
}

int SkillOverlay::cipollaKey() const
{
    return config.cipollaKey;
}

SkillOverlay::Direction SkillOverlay::directionFromConfig(int value) const
{
    switch(value)
    {
    case 1:
        return Direction::Up;

    case 2:
        return Direction::Left;

    case 3:
        return Direction::Down;

    case 4:
        return Direction::Right;

    default:
        return Direction::None;
    }
}

void SkillOverlay::updateSelectedSkillVisual()
{
    upSkill->setSelected(
        currentDirection == Direction::Up
        );

    leftSkill->setSelected(
        currentDirection == Direction::Left
        );

    downSkill->setSelected(
        currentDirection == Direction::Down
        );

    rightSkill->setSelected(
        currentDirection == Direction::Right
        );
}
void SkillOverlay::startArtifactSkills()
{
    if(config.up.activation == SkillActivation::Artifact)
        upSkill->startCooldown();

    if(config.left.activation == SkillActivation::Artifact)
        leftSkill->startCooldown();

    if(config.down.activation == SkillActivation::Artifact)
        downSkill->startCooldown();

    if(config.right.activation == SkillActivation::Artifact)
        rightSkill->startCooldown();
}