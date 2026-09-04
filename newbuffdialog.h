#ifndef NEWBUFFDIALOG_H
#define NEWBUFFDIALOG_H

#include <QDialog>
#include "keyedit.h"

class QSpinBox;
class QPushButton;

class NewBuffDialog : public QDialog
{
    Q_OBJECT

public:

    explicit NewBuffDialog(
        QWidget *parent = nullptr
        );


    int keyCode() const;

    int cooldown() const;


private:

    KeyEdit *keyEdit = nullptr;

    QSpinBox *cooldownSpinBox = nullptr;

    QPushButton *saveButton = nullptr;

    QPushButton *cancelButton = nullptr;


    void createUi();


private slots:

    void save();

};

#endif