#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void buffTitlesToggled(bool enabled);
    void buffTranscendenceToggled(bool enabled);
    void pauseKeyChanged(int vkCode);

private:
    QPushButton *atmaConfigButton;
    QPushButton *atmaToggleButton;
    QPushButton *classBuffConfigButton;
    QPushButton *classBuffToggleButton;
    QPushButton *distanceGuidesConfigButton;
    QPushButton *distanceGuidesToggleButton;
    QPushButton *buffTrackerConfigButton;
    QPushButton *closeButton;
    QPushButton *buffTitlesToggleButton;
    QPushButton *buffTranscendenceToggleButton;
    QPushButton *pauseKeyButton;

    int m_pauseKey = 0x20; // VK_SPACE

    int pauseKey() const;
    void setupToggleButton(QPushButton *button);
    void updateToggleText(QPushButton *button, bool enabled);
    void loadPauseKey();
    void savePauseKey(int vkCode);
    void openPauseKeyDialog();
    void updatePauseKeyButtonText();
};

#endif // MAINWINDOW_H