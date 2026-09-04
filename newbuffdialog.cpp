#include "newbuffdialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>


NewBuffDialog::NewBuffDialog(
    QWidget *parent
    )
    : QDialog(parent)
{
    setWindowTitle(
        "New Key"
        );


    setFixedSize(
        350,
        250
        );


    createUi();
}



void NewBuffDialog::createUi()
{
    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            this
            );


    mainLayout->setContentsMargins(
        20,
        20,
        20,
        20
        );


    mainLayout->setSpacing(
        10
        );



    // =========================
    // KEY
    // =========================

    QLabel *keyLabel =
        new QLabel(
            "Key:",
            this
            );


    keyEdit =
        new KeyEdit(
            this
            );


    keyEdit->setPlaceholderText(
        "Premi un tasto..."
        );


    mainLayout->addWidget(
        keyLabel
        );


    mainLayout->addWidget(
        keyEdit
        );



    // =========================
    // COOLDOWN
    // =========================

    QLabel *cooldownLabel =
        new QLabel(
            "Cooldown (secondi):",
            this
            );


    cooldownSpinBox =
        new QSpinBox(
            this
            );


    cooldownSpinBox->setMinimum(
        1
        );


    cooldownSpinBox->setMaximum(
        3600
        );


    cooldownSpinBox->setValue(
        1
        );


    mainLayout->addWidget(
        cooldownLabel
        );


    mainLayout->addWidget(
        cooldownSpinBox
        );



    // =========================
    // BUTTONS
    // =========================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();


    saveButton =
        new QPushButton(
            "Salva",
            this
            );


    cancelButton =
        new QPushButton(
            "Annulla",
            this
            );


    buttonLayout->addWidget(
        saveButton
        );


    buttonLayout->addWidget(
        cancelButton
        );


    mainLayout->addLayout(
        buttonLayout
        );



    // =========================
    // CONNECTIONS
    // =========================

    connect(
        saveButton,
        &QPushButton::clicked,
        this,
        &NewBuffDialog::save
        );


    connect(
        cancelButton,
        &QPushButton::clicked,
        this,
        &QDialog::reject
        );
}



void NewBuffDialog::save()
{
    if(keyEdit->keyCode() == 0)
    {
        QMessageBox::warning(
            this,
            "Errore",
            "Inserisci un tasto."
            );

        return;
    }


    if(cooldownSpinBox->value() <= 0)
    {
        QMessageBox::warning(
            this,
            "Errore",
            "Inserisci un cooldown valido."
            );

        return;
    }


    accept();
}



int NewBuffDialog::keyCode() const
{
    return keyEdit->keyCode();
}



int NewBuffDialog::cooldown() const
{
    return cooldownSpinBox->value();
}