#include "mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>
#include <QWidget>


MainWindow::MainWindow(
    QWidget *parent
    )
    : QMainWindow(parent)
{
    setWindowTitle(
        "ElsOverlay"
        );


    setFixedSize(
        420,
        420
        );


    QWidget *central =
        new QWidget(this);


    setCentralWidget(
        central
        );


    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            central
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
    // TITOLO
    // =========================

    QLabel *title =
        new QLabel(
            "ELS OVERLAY",
            central
            );


    QFont titleFont;

    titleFont.setPointSize(
        18
        );

    titleFont.setBold(
        true
        );


    title->setFont(
        titleFont
        );


    title->setAlignment(
        Qt::AlignCenter
        );


    mainLayout->addWidget(
        title
        );



    // =========================
    // ATMA
    // =========================

    QGroupBox *atmaGroup =
        new QGroupBox(
            "Atma",
            central
            );


    QHBoxLayout *atmaLayout =
        new QHBoxLayout(
            atmaGroup
            );


    atmaConfigButton =
        new QPushButton(
            "Configura",
            atmaGroup
            );


    atmaToggleButton =
        new QPushButton(
            "OFF",
            atmaGroup
            );


    setupToggleButton(
        atmaToggleButton
        );


    atmaLayout->addWidget(
        atmaConfigButton
        );


    atmaLayout->addStretch();


    atmaLayout->addWidget(
        atmaToggleButton
        );


    mainLayout->addWidget(
        atmaGroup
        );



    // =========================
    // CLASS BUFF
    // =========================

    QGroupBox *classBuffGroup =
        new QGroupBox(
            "Class Buff",
            central
            );


    QHBoxLayout *classBuffLayout =
        new QHBoxLayout(
            classBuffGroup
            );


    classBuffConfigButton =
        new QPushButton(
            "Configura",
            classBuffGroup
            );


    classBuffToggleButton =
        new QPushButton(
            "OFF",
            classBuffGroup
            );


    setupToggleButton(
        classBuffToggleButton
        );


    classBuffLayout->addWidget(
        classBuffConfigButton
        );


    classBuffLayout->addStretch();


    classBuffLayout->addWidget(
        classBuffToggleButton
        );


    mainLayout->addWidget(
        classBuffGroup
        );



    // =========================
    // DISTANCE GUIDES
    // =========================

    QGroupBox *distanceGroup =
        new QGroupBox(
            "Distance Guides",
            central
            );


    QHBoxLayout *distanceLayout =
        new QHBoxLayout(
            distanceGroup
            );


    distanceGuidesConfigButton =
        new QPushButton(
            "Configura",
            distanceGroup
            );


    distanceGuidesToggleButton =
        new QPushButton(
            "OFF",
            distanceGroup
            );


    setupToggleButton(
        distanceGuidesToggleButton
        );


    distanceLayout->addWidget(
        distanceGuidesConfigButton
        );


    distanceLayout->addStretch();


    distanceLayout->addWidget(
        distanceGuidesToggleButton
        );


    mainLayout->addWidget(
        distanceGroup
        );



    // =========================
    // BUFF TRACKER
    // =========================

    QGroupBox *trackerGroup =
        new QGroupBox(
            "Buff Tracker",
            central
            );


    QHBoxLayout *trackerLayout =
        new QHBoxLayout(
            trackerGroup
            );


    buffTrackerConfigButton =
        new QPushButton(
            "Configura",
            trackerGroup
            );




    trackerLayout->addWidget(
        buffTrackerConfigButton
        );




    mainLayout->addWidget(
        trackerGroup
        );



    // =========================
    // SPAZIO
    // =========================

    mainLayout->addStretch();



    // =========================
    // CHIUDI
    // =========================

    closeButton =
        new QPushButton(
            "Chiudi",
            central
            );


    mainLayout->addWidget(
        closeButton
        );



    // =========================
    // CONNECTIONS ATMA
    // =========================

    connect(
        atmaConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::atmaConfigRequested
        );


    connect(
        atmaToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                atmaToggleButton,
                enabled
                );

            emit atmaToggled(
                enabled
                );
        }
        );



    // =========================
    // CONNECTIONS CLASS BUFF
    // =========================

    connect(
        classBuffConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::classBuffConfigRequested
        );


    connect(
        classBuffToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                classBuffToggleButton,
                enabled
                );

            emit classBuffToggled(
                enabled
                );
        }
        );



    // =========================
    // CONNECTIONS DISTANCE
    // =========================

    connect(
        distanceGuidesConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::distanceGuidesConfigRequested
        );


    connect(
        distanceGuidesToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                distanceGuidesToggleButton,
                enabled
                );

            emit distanceGuidesToggled(
                enabled
                );
        }
        );



    // =========================
    // CONNECTIONS BUFF TRACKER
    // =========================

    connect(
        buffTrackerConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::buffTrackerConfigRequested
        );





    // =========================
    // CHIUDI
    // =========================

    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &QMainWindow::close
        );
}



void MainWindow::setupToggleButton(
    QPushButton *button
    )
{
    if(!button)
        return;


    button->setCheckable(
        true
        );


    button->setChecked(
        false
        );


    button->setMinimumWidth(
        70
        );


    updateToggleText(
        button,
        false
        );
}



void MainWindow::updateToggleText(
    QPushButton *button,
    bool enabled
    )
{
    if(!button)
        return;


    button->setText(
        enabled
            ? "ON"
            : "OFF"
        );
}