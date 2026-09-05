#ifndef SPECIALCOOLDOWNCONFIGWINDOW_H
#define SPECIALCOOLDOWNCONFIGWINDOW_H

#include <QDialog>
#include <QList>
#include <QString>

#include "specialcooldownconfiguration.h"

    class QListWidget;
class QListWidgetItem;
class QPushButton;

class SpecialCooldownManager;

class SpecialCooldownConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SpecialCooldownConfigWindow(
        SpecialCooldownManager *manager,
        QWidget *parent = nullptr
        );

signals:
    void configurationSaved();

private slots:
    void addSpecialCooldown();

    void removeSpecialCooldown();

    void editSpecialCooldown(
        QListWidgetItem *item
        );

    void saveConfiguration();

private:
    QListWidget *m_listWidget = nullptr;

    QPushButton *m_addButton = nullptr;
    QPushButton *m_removeButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_cancelButton = nullptr;

    SpecialCooldownManager *m_manager = nullptr;

    QList<SpecialCooldownConfiguration> m_configurations;

    void refreshList();

    QString keyName(
        int key
        ) const;

    QString displayName(
        const SpecialCooldownConfiguration &configuration
        ) const;
};

#endif
