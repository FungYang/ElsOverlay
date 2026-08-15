#include "classbuffconfigwindow.h"
#include "classselector.h"
#include "classconfigurationmanager.h"
#include "classbuffeditorwindow.h"
#include "globalkeyboard.h"


#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QMessageBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QDir>
#include <QCoreApplication>


ClassBuffConfigWindow::ClassBuffConfigWindow(
    ClassConfigurationManager *manager,
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    m_manager(manager),
    m_keyboard(keyboard)
{

    setWindowTitle(
        "Class Buff Configuration"
        );


    setFixedSize(
        600,
        600
        );


    createUi();


    ClassSelector *selector =
        new ClassSelector();


    selector->hide();



    connect(
        m_addButton,
        &QPushButton::clicked,
        this,
        [selector]()
        {
            selector->open();
        }
        );



    connect(
        selector,
        &ClassSelector::classSelected,
        this,
        [this](
            const QString &id,
            const QString &imagePath
            )
        {

            if(!m_manager)
                return;


            if(!m_manager->addConfiguration(
                    id,
                    imagePath
                    ))
            {
                return;
            }


            refresh();


            show();

            raise();

            activateWindow();

        }
        );



    connect(
        m_closeButton,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );

    connect(
        this,
        &ClassBuffConfigWindow::configureRequested,
        this,
        [this](const QString &configurationId)
        {
            if(!m_manager)
                return;


            ClassBuffEditorWindow *editor =
                new ClassBuffEditorWindow(
                    m_manager,
                    configurationId,
                    m_keyboard
                    );

            editor->setAttribute(
                Qt::WA_DeleteOnClose
                );

            connect(
                editor,
                &ClassBuffEditorWindow::editorClosed,
                this,
                [this]()
                {
                    refresh();
                }
                );

            editor->show();
            editor->raise();
            editor->activateWindow();


        }
        );


    refresh();

}



void ClassBuffConfigWindow::createUi()
{

    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            this
            );


    mainLayout->setContentsMargins(
        15,
        15,
        15,
        15
        );


    mainLayout->setSpacing(
        10
        );



    QLabel *title =
        new QLabel(
            "CLASS BUFF CONFIGURATION",
            this
            );


    QFont font =
        title->font();


    font.setBold(
        true
        );


    font.setPointSize(
        16
        );


    title->setFont(
        font
        );


    title->setAlignment(
        Qt::AlignCenter
        );


    mainLayout->addWidget(
        title
        );



    m_list =
        new QListWidget(
            this
            );


    m_list->setSpacing(
        5
        );


    mainLayout->addWidget(
        m_list
        );



    m_addButton =
        new QPushButton(
            "Add Configuration",
            this
            );


    m_addButton->setMinimumHeight(
        40
        );


    mainLayout->addWidget(
        m_addButton
        );



    m_closeButton =
        new QPushButton(
            "Chiudi",
            this
            );


    mainLayout->addWidget(
        m_closeButton
        );





    connect(
        m_closeButton,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );

}



void ClassBuffConfigWindow::refresh()
{

    populateList();

}



void ClassBuffConfigWindow::populateList()
{

    m_list->clear();


    if(!m_manager)
        return;


    const QList<ClassConfiguration>
        configurations =
        m_manager->configurations();


    const QString activeId =
        m_manager->activeConfigurationId();



    for(const ClassConfiguration &configuration :
         configurations)
    {

        QListWidgetItem *item =
            new QListWidgetItem(
                m_list
                );


        QWidget *row =
            new QWidget(
                m_list
                );


        QHBoxLayout *layout =
            new QHBoxLayout(
                row
                );


        layout->setContentsMargins(
            5,
            5,
            5,
            5
            );



        QLabel *imageLabel =
            new QLabel(
                row
                );


        imageLabel->setFixedSize(
            70,
            70
            );


        imageLabel->setAlignment(
            Qt::AlignCenter
            );


        QPixmap image(
            configuration.imagePath
            );


        if(!image.isNull())
        {

            imageLabel->setPixmap(
                image.scaled(
                    64,
                    64,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    )
                );

        }


        layout->addWidget(
            imageLabel
            );



        QLabel *nameLabel =
            new QLabel(
                configuration.id,
                row
                );


        nameLabel->setMinimumWidth(
            120
            );


        layout->addWidget(
            nameLabel
            );



        QRadioButton *activeButton =
            new QRadioButton(
                "Attiva",
                row
                );


        activeButton->setChecked(
            configuration.id == activeId
            );


        layout->addWidget(
            activeButton
            );



        layout->addStretch();



        QPushButton *configureButton =
            new QPushButton(
                "Configura",
                row
                );


        layout->addWidget(
            configureButton
            );



        QPushButton *deleteButton =
            new QPushButton(
                "Delete",
                row
                );


        layout->addWidget(
            deleteButton
            );



        item->setSizeHint(
            row->sizeHint()
            );


        m_list->setItemWidget(
            item,
            row
            );



        connect(
            activeButton,
            &QRadioButton::toggled,
            this,
            [this, configurationId = configuration.id](bool checked)
            {

                if(!checked)
                    return;


                if(!m_manager)
                    return;


                m_manager->setActiveConfiguration(
                    configurationId
                    );


                populateList();

            }
            );



        connect(
            configureButton,
            &QPushButton::clicked,
            this,
            [this, configurationId = configuration.id]()
            {

                emit configureRequested(
                    configurationId
                    );

            }
            );



        connect(
            deleteButton,
            &QPushButton::clicked,
            this,
            [this, configurationId = configuration.id]()
            {

                if(!m_manager)
                    return;


                const auto result =
                    QMessageBox::question(
                        this,
                        "Delete Configuration",
                        "Really hehe? \"" +
                            configurationId +
                            "\"?"
                        );


                if(result != QMessageBox::Yes)
                    return;


                m_manager->removeConfiguration(
                    configurationId
                    );


                populateList();

            }
            );

    }

}

