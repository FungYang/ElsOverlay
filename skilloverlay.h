#ifndef SKILLOVERLAY_H
#define SKILLOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QSettings>
#include <QPoint>

#include "skillbox.h"
#include "globalkeyboard.h"
#include "skillconfig.h"


class SkillOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit SkillOverlay(
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );


    void resetAllCooldowns();


    void checkSequences(
        int key
        );


public slots:

    void applyConfig(
        const SkillOverlayConfig &config
        );


protected:

    void mousePressEvent(
        QMouseEvent *event
        ) override;


    void mouseMoveEvent(
        QMouseEvent *event
        ) override;


private:

    // --------------------------------------------------------
    // SKILL BOX
    // --------------------------------------------------------

    SkillBox *upSkill;
    SkillBox *leftSkill;
    SkillBox *downSkill;
    SkillBox *rightSkill;



    // --------------------------------------------------------
    // TIMER
    // --------------------------------------------------------

    QTimer *timer;


    // --------------------------------------------------------
    // DRAG
    // --------------------------------------------------------

    QPoint dragPosition;


    // --------------------------------------------------------
    // KEYBOARD
    // --------------------------------------------------------

    GlobalKeyboard *keyboard;


    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    SkillOverlayConfig config;


    // --------------------------------------------------------
    // STATE
    // --------------------------------------------------------

    bool trackingActive = true;


    enum class SequenceState
    {
        WaitingStateKey,
        WaitingDirection
    };


    SequenceState sequenceState =
        SequenceState::WaitingStateKey;


    enum class Direction
    {
        None,
        Up,
        Left,
        Down,
        Right
    };


    Direction currentDirection =
        Direction::None;


    // --------------------------------------------------------
    // METHODS
    // --------------------------------------------------------

    SkillConfig *currentSkill();


    void activateCtrlSkill();


    void activateCipollaSkill();


    void activateComboSkill(
        int key
        );


    void updateSkillBoxes();


    void loadDefaultConfig();
};

#endif