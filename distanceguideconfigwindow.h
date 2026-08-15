#ifndef DISTANCEGUIDECONFIGWINDOW_H
#define DISTANCEGUIDECONFIGWINDOW_H

#include <QWidget>

#include "distanceguidemanager.h"
#include "distanceguiderectangle.h"
#include "distanceguidecircle.h"
#include "globalkeyboard.h"


class QListWidget;
class QPushButton;
class QSlider;
class QLabel;
class DistanceGuideLine;
class GlobalKeyboard;


class DistanceGuideConfigWindow : public QWidget{
    Q_OBJECT

public:

    explicit DistanceGuideConfigWindow(
        DistanceGuideManager *manager,
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );

    void confirmPositions();


    void refresh();


signals:

    void configureRequested(
        const QString &guideId
        );

protected:

    void closeEvent(
        QCloseEvent *event
        ) override;


private:

    void createUi();
    void populateList();
    void addGuide();
    void clearConfigurationObject();
    void fixCharacterCenter();

    void confirmCharacterCenter();

    void clearCharacterCenterLine();

    DistanceGuideLine *m_characterCenterLine =
        nullptr;


    DistanceGuideManager *m_manager;

    DistanceGuideLine *m_configurationLine = nullptr;
    DistanceGuideRectangle *m_configurationRectangle = nullptr;
    DistanceGuideCircle *m_configurationCircle = nullptr;

    GlobalKeyboard *m_keyboard = nullptr;

    QString m_configurationGuideId;

    QListWidget *m_list;

    QPushButton *m_addButton;
    QPushButton *m_closeButton;

    QPushButton *m_addRectangleButton =
        nullptr;

    QPushButton *m_addCircleButton =
        nullptr;

    QPushButton *m_addGroupButton =
        nullptr;

    QPushButton *m_fixCharacterButton =
        nullptr;

    QSlider *m_opacitySlider =
        nullptr;

    QLabel *m_opacityValueLabel =
        nullptr;
};


#endif