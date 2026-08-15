#ifndef NEWBUFFDIALOG_H
#define NEWBUFFDIALOG_H

#include <QDialog>
#include <QChar>

class QLineEdit;
class QPushButton;
class QSpinBox;

class NewBuffDialog : public QDialog
{
    Q_OBJECT

public:

    explicit NewBuffDialog(
        QWidget *parent = nullptr
        );


    QChar key() const;

    int cooldown() const;


private:

    QLineEdit *keyEdit = nullptr;

    QSpinBox *cooldownSpinBox = nullptr;

    QPushButton *saveButton = nullptr;

    QPushButton *cancelButton = nullptr;


    void createUi();

private slots:

    void save();

};

#endif