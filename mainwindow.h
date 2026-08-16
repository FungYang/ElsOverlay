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

    void setupToggleButton(
        QPushButton *button
        );

    void updateToggleText(
        QPushButton *button,
        bool enabled
        );
};

#endif // MAINWINDOW_H