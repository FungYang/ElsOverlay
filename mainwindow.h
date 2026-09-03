#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

signals:

    void atmaConfigRequested();
    void atmaToggled(bool enabled);

    void classBuffConfigRequested();
    void classBuffToggled(bool enabled);

    void distanceGuidesConfigRequested();
    void distanceGuidesToggled(bool enabled);

    void buffTrackerConfigRequested();

    void buffTitlesConfigRequested();
    void buffTitlesToggled(bool enabled);

    void buffTranscendenceToggled(bool enabled);

    void pauseKeyChanged(int scanCode, bool extended);
    void resetKeyChanged(int scanCode, bool extended);

    void transcendenceConfigRequested();

private:
    QPushButton *atmaConfigButton;
    QPushButton *atmaToggleButton;

    QPushButton *classBuffConfigButton;
    QPushButton *classBuffToggleButton;

    QPushButton *distanceGuidesConfigButton;
    QPushButton *distanceGuidesToggleButton;

    QPushButton *buffTrackerConfigButton;

    QPushButton *buffTitlesConfigButton;
    QPushButton *buffTitlesToggleButton;

    QPushButton *buffTranscendenceToggleButton;

    QPushButton *closeButton;

    QPushButton *pauseKeyButton;
    QPushButton *resetKeyButton;
    QPushButton *transcendenceConfigButton;

    int m_pauseScanCode = 0x01;
    bool m_pauseExtended = false;

    int m_resetScanCode = 0x1D;
    bool m_resetExtended = true;

    void setupToggleButton(QPushButton *button);
    void updateToggleText(QPushButton *button, bool enabled);

    void loadPauseKey();
    void savePauseKey(int scanCode, bool extended);
    void openPauseKeyDialog();
    void updatePauseKeyButtonText();

    void loadResetKey();
    void saveResetKey(int scanCode, bool extended);
    void openResetKeyDialog();
    void updateResetKeyButtonText();
};

#endif