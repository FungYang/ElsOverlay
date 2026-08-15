#ifndef DISTANCEGUIDEGROUPCONFIGWINDOW_H
#define DISTANCEGUIDEGROUPCONFIGWINDOW_H

#include <QWidget>

#include "distanceguidemanager.h"

class QListWidget;
class QPushButton;
class QLabel;
class QListWidgetItem;
class GlobalKeyboard;


class DistanceGuideGroupConfigWindow :
                                       public QWidget
{
    Q_OBJECT

public:

    explicit DistanceGuideGroupConfigWindow(
        DistanceGuideManager *manager,
        const QString &groupId,
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );


signals:

    void configureGuideRequested(
        const QString &guideId
        );


protected:

    void closeEvent(
        QCloseEvent *event
        ) override;


private:

    void createUi();

    void refresh();

    void populateList();

    void addLineToGroup();

    void addRectangleToGroup();

    void addCircleToGroup();

    void removeGuideFromGroup(
        const QString &guideId
        );


    DistanceGuideManager *m_manager =
        nullptr;


    GlobalKeyboard *m_keyboard =
        nullptr;


    QString m_groupId;


    QListWidget *m_list =
        nullptr;

    QPushButton *m_closeButton =
        nullptr;


    QLabel *m_groupLabel =
        nullptr;


    QPushButton *m_addLineButton =
        nullptr;

    QPushButton *m_addRectangleButton =
        nullptr;

    QPushButton *m_addCircleButton =
        nullptr;
};


#endif