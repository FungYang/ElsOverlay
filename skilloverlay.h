#ifndef SKILLOVERLAY_H
#define SKILLOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QSettings>
#include <QPoint>
#include <QRect>

#include "skillbox.h"
#include "globalkeyboard.h"
#include "skillconfig.h"


class OverlayRoot;

    class SkillOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit SkillOverlay(
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );

    ~SkillOverlay() override;

    // Registra i 4 box in OverlayRoot (click-through globale, raise,
    // nascondi/mostra tutto). Ogni box e' una finestra indipendente.
    void registerBoxes(
        OverlayRoot *root
        );


    void resetAllCooldowns();


    void checkSequences(
        int key
        );


    void setScale(
        double scale
        );


    double getScale() const;
    int cipollaKey() const;
    void startArtifactSkills();


public slots:

    void applyConfig(
        const SkillOverlayConfig &config
        );
    void pauseAtmaGate();

    void resumeAtmaGate();

    // Riporta i 4 box nella disposizione a croce originale
    // (cancella le posizioni salvate per ogni box).
    void resetPositions();

    void setTransparency(
        int value
        );


protected:

    // Questa finestra e' solo il COORDINATORE dei 4 box (1x1 px,
    // trasparente). Mostrarla/nasconderla mostra/nasconde i box.
    void showEvent(
        QShowEvent *event
        ) override;

    void hideEvent(
        QHideEvent *event
        ) override;


private:
    bool cooldownsPaused = false;

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
    // POSIZIONE DEI BOX (indipendente per ciascun box)
    //
    // 0 = Up, 1 = Left, 2 = Down, 3 = Right.
    // Se un box non e' mai stato spostato non ha posizione salvata
    // e sta nella disposizione a croce attorno alla finestra madre.
    // --------------------------------------------------------

    QPoint m_boxPosition[4];
    bool m_boxHasPosition[4] = { false, false, false, false };

    SkillBox *boxAt(int index) const;
    QPoint defaultBoxPosition(int index) const;
    void initBoxes();
    void applyBoxPositions();
    void saveBoxPosition(int index, const QPoint &position);


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

    void updateSelectedSkillVisual();

    // Applica a ogni SkillBox lo stato "titolo attivo" della config
    // (box nascosto + cooldown/glow disattivati).
    void applyEnabledState();

    Direction directionFromConfig(int value) const;
};

#endif
