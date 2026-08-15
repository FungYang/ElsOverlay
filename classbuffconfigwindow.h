#ifndef CLASSBUFFCONFIGWINDOW_H
#define CLASSBUFFCONFIGWINDOW_H

#include <QWidget>
#include "classconfigurationmanager.h"

class QListWidget;
class QPushButton;
class ClassConfigurationManager;
class GlobalKeyboard;


class ClassBuffConfigWindow : public QWidget
{
    Q_OBJECT

public:

    explicit ClassBuffConfigWindow(
        ClassConfigurationManager *manager,
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );


    void refresh();


signals:

    void configureRequested(
        const QString &configurationId
        );


    void addConfigurationRequested();


private:

    ClassConfigurationManager *m_manager = nullptr;

    QListWidget *m_list = nullptr;

    QPushButton *m_addButton = nullptr;

    QPushButton *m_closeButton = nullptr;

    GlobalKeyboard *m_keyboard = nullptr;



    void createUi();

    void populateList();

    void selectActiveConfiguration(
        const QString &id
        );

};

#endif