#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>


class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(
        QWidget *parent = nullptr
        );

    ~MainWindow() override = default;


    // ==================================================
    // TOGGLE PERSISTENCE
    // ==================================================

    void loadToggleStates();
    void saveToggleStates();


signals:

    void atmaConfigRequested();
    void atmaToggled(
        bool enabled
        );


    void classBuffConfigRequested();
    void classBuffToggled(
        bool enabled
        );


    void distanceGuidesConfigRequested();
    void distanceGuidesToggled(
        bool enabled
        );


    void buffTrackerConfigRequested();


    void buffTitlesConfigRequested();
    void buffTitlesToggled(
        bool enabled
        );


    void buffTranscendenceToggled(
        bool enabled
        );


    void specialCooldownConfigRequested();
    void specialCooldownsToggled(
        bool enabled
        );


    void pauseKeyChanged(
        int scanCode,
        bool extended
        );

    void resetKeyChanged(
        int scanCode,
        bool extended
        );


    void transcendenceConfigRequested();


    void overlayClickabilityToggled(
        bool enabled
        );




private:

    // ==================================================
    // ATMA
    // ==================================================

    QPushButton *atmaConfigButton;
    QPushButton *atmaToggleButton;


    // ==================================================
    // CLASS BUFF
    // ==================================================

    QPushButton *classBuffConfigButton;
    QPushButton *classBuffToggleButton;


    // ==================================================
    // DISTANCE GUIDES
    // ==================================================

    QPushButton *distanceGuidesConfigButton;
    QPushButton *distanceGuidesToggleButton;


    // ==================================================
    // BUFF TRACKER
    // ==================================================

    QPushButton *buffTrackerConfigButton;


    // ==================================================
    // BUFF TITLES
    // ==================================================

    QPushButton *buffTitlesConfigButton;
    QPushButton *buffTitlesToggleButton;


    // ==================================================
    // TRANSCENDENCE
    // ==================================================

    QPushButton *buffTranscendenceToggleButton;


    // ==================================================
    // SPECIAL COOLDOWNS
    // ==================================================

    QPushButton *specialCooldownConfigButton;
    QPushButton *specialCooldownToggleButton;


    // ==================================================
    // GENERAL
    // ==================================================

    QPushButton *closeButton;


    // ==================================================
    // PAUSE / RESET
    // ==================================================

    QPushButton *pauseKeyButton;
    QPushButton *resetKeyButton;


    // ==================================================
    // TRANSCENDENCE CONFIG
    // ==================================================

    QPushButton *transcendenceConfigButton;


    // ==================================================
    // OVERLAY CLICKABILITY
    // ==================================================

    QPushButton *overlayClickabilityToggleButton;



    // ==================================================
    // PAUSE KEY
    // ==================================================

    int m_pauseScanCode = 0x01;
    bool m_pauseExtended = false;


    // ==================================================
    // RESET KEY
    // ==================================================

    int m_resetScanCode = 0x1D;
    bool m_resetExtended = true;


    // ==================================================
    // TOGGLE UI
    // ==================================================

    void setupToggleButton(
        QPushButton *button
        );

    void updateToggleText(
        QPushButton *button,
        bool enabled
        );


    // ==================================================
    // PAUSE KEY
    // ==================================================

    void loadPauseKey();

    void savePauseKey(
        int scanCode,
        bool extended
        );

    void openPauseKeyDialog();

    void updatePauseKeyButtonText();


    // ==================================================
    // RESET KEY
    // ==================================================

    void loadResetKey();

    void saveResetKey(
        int scanCode,
        bool extended
        );

    void openResetKeyDialog();

    void updateResetKeyButtonText();


};

#endif
