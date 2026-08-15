#ifndef CLASSBUFFEDITORWINDOW_H
#define CLASSBUFFEDITORWINDOW_H

#include <QWidget>
#include <QList>
#include <QString>

#include "classconfigurationmanager.h"


class QListWidget;
class QPushButton;
class BuffBox;
class GlobalKeyboard;


class ClassBuffEditorWindow : public QWidget
{
    Q_OBJECT

public:

    explicit ClassBuffEditorWindow(
        ClassConfigurationManager *manager,
        const QString &configurationId,
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );

    void clearPreviewBoxes();
    void createPreviewBoxes();


signals:

    void configurationSaved();
    void editorClosed();

private slots:

    void confirmPositions();


protected:

    void closeEvent(
        QCloseEvent *event
        ) override;


private:

    ClassConfigurationManager *m_manager = nullptr;

    QString m_configurationId;

    QList<BuffConfiguration> m_buffs;


    QListWidget *buffList = nullptr;

    QPushButton *newKeyButton = nullptr;

    QPushButton *saveButton = nullptr;

    QPushButton *closeButton = nullptr;

    QList<BuffBox*> m_previewBoxes;

    GlobalKeyboard *m_keyboard = nullptr;




    void createUi();

    void loadConfiguration();

    void refreshList();

    void addNewKey();

    void deleteBuff(
        int index
        );

    void saveConfiguration();

};

#endif // CLASSBUFFEDITORWINDOW_H