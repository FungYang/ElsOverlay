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


    void setScale(
        double scale
        );


    double getScale() const;


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

    SkillBox *upSkill = nullptr;

    SkillBox *leftSkill = nullptr;

    SkillBox *downSkill = nullptr;

    SkillBox *rightSkill = nullptr;


    // --------------------------------------------------------
    // TIMER
    // --------------------------------------------------------

    QTimer *timer = nullptr;


    // --------------------------------------------------------
    // DRAG
    // --------------------------------------------------------

    QPoint dragPosition;


    // --------------------------------------------------------
    // KEYBOARD
    // --------------------------------------------------------

    GlobalKeyboard *keyboard = nullptr;


    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    SkillOverlayConfig config;


    // --------------------------------------------------------
    // SCALE
    // --------------------------------------------------------

    double scale = 1.0;


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


    void updateOverlayGeometry();
};

#endif
