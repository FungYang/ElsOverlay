#include "distanceguidegroupconfigwindow.h"

#include "distanceguideconfiguration.h"
#include "distanceguidegroup.h"
#include "globalkeyboard.h"

#include <QCloseEvent>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>


DistanceGuideGroupConfigWindow::DistanceGuideGroupConfigWindow(
    DistanceGuideManager *manager,
    const QString &groupId,
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    m_manager(manager),
    m_keyboard(keyboard),
    m_groupId(groupId)
{
    setWindowTitle(
        "Configure Group"
        );


    setFixedSize(
        600,
        500
        );


    createUi();

    refresh();
}


// ==================================================
// UI
// ==================================================

void DistanceGuideGroupConfigWindow::createUi()
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


    // ==================================================
    // GROUP TITLE
    // ==================================================

    m_groupLabel =
        new QLabel(
            this
            );


    QFont font =
        m_groupLabel->font();


    font.setBold(
        true
        );


    font.setPointSize(
        16
        );


    m_groupLabel->setFont(
        font
        );


    m_groupLabel->setAlignment(
        Qt::AlignCenter
        );


    mainLayout->addWidget(
        m_groupLabel
        );


    // ==================================================
    // LIST
    // ==================================================

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


    // ==================================================
    // BUTTONS
    // ==================================================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();


    m_addLineButton =
        new QPushButton(
            "Aggiungi linea",
            this
            );


    m_addRectangleButton =
        new QPushButton(
            "Aggiungi rettangolo",
            this
            );


    m_addCircleButton =
        new QPushButton(
            "Aggiungi cerchio",
            this
            );


    m_closeButton =
        new QPushButton(
            "Chiudi",
            this
            );


    m_addLineButton->setMinimumHeight(
        40
        );


    m_addRectangleButton->setMinimumHeight(
        40
        );


    m_addCircleButton->setMinimumHeight(
        40
        );


    m_closeButton->setMinimumHeight(
        40
        );


    buttonLayout->addWidget(
        m_addLineButton
        );


    buttonLayout->addWidget(
        m_addRectangleButton
        );


    buttonLayout->addWidget(
        m_addCircleButton
        );


    buttonLayout->addStretch();


    buttonLayout->addWidget(
        m_closeButton
        );


    mainLayout->addLayout(
        buttonLayout
        );


    // ==================================================
    // CONNECTIONS
    // ==================================================

    connect(
        m_addLineButton,
        &QPushButton::clicked,
        this,
        &DistanceGuideGroupConfigWindow::addLineToGroup
        );


    connect(
        m_addRectangleButton,
        &QPushButton::clicked,
        this,
        &DistanceGuideGroupConfigWindow::addRectangleToGroup
        );


    connect(
        m_addCircleButton,
        &QPushButton::clicked,
        this,
        &DistanceGuideGroupConfigWindow::addCircleToGroup
        );


    connect(
        m_closeButton,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );


}


// ==================================================
// REFRESH
// ==================================================

void DistanceGuideGroupConfigWindow::refresh()
{
    populateList();
}


// ==================================================
// POPULATE
// ==================================================

void DistanceGuideGroupConfigWindow::populateList()
{
    if(!m_list)
        return;


    m_list->clear();


    if(!m_manager)
        return;


    if(!m_manager->containsGroup(
            m_groupId
            ))
    {
        m_groupLabel->setText(
            "Gruppo non trovato"
            );

        return;
    }


    const DistanceGuideGroup group =
        m_manager->group(
            m_groupId
            );


    m_groupLabel->setText(
        "GRUPPO: " +
        group.name
        );


    const QList<DistanceGuideConfiguration>
        guides =
        m_manager->groupGuides(
            m_groupId
            );


    bool hasGuides =
        false;


    for(const DistanceGuideConfiguration &guide :
         guides)
    {



        hasGuides =
            true;


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


        // ==================================================
        // NAME
        // ==================================================

        QLabel *nameLabel =
            new QLabel(
                guide.name,
                row
                );


        nameLabel->setMinimumWidth(
            180
            );


        layout->addWidget(
            nameLabel
            );


        // ==================================================
        // TYPE
        // ==================================================

        QString typeName;


        switch(guide.type)
        {
        case DistanceGuideType::VerticalLine:
            typeName =
                "Linea";
            break;


        case DistanceGuideType::Rectangle:
            typeName =
                "Rettangolo";
            break;


        case DistanceGuideType::Circle:
            typeName =
                "Cerchio";
            break;
        }


        QLabel *typeLabel =
            new QLabel(
                typeName,
                row
                );


        typeLabel->setMinimumWidth(
            90
            );


        layout->addWidget(
            typeLabel
            );


        layout->addStretch();


        // ==================================================
        // CONFIGURE
        // ==================================================

        QPushButton *configureButton =
            new QPushButton(
                "Configura",
                row
                );


        layout->addWidget(
            configureButton
            );


        connect(
            configureButton,
            &QPushButton::clicked,
            this,
            [this,
             guideId = guide.id]()
            {
                emit configureGuideRequested(
                    guideId
                    );
            }
            );


        // ==================================================
        // REMOVE
        // ==================================================

        QPushButton *removeButton =
            new QPushButton(
                "Rimuovi",
                row
                );


        layout->addWidget(
            removeButton
            );


        connect(
            removeButton,
            &QPushButton::clicked,
            this,
            [this,
             guideId = guide.id]()
            {
                removeGuideFromGroup(
                    guideId
                    );
            }
            );


        item->setSizeHint(
            row->sizeHint()
            );


        m_list->setItemWidget(
            item,
            row
            );
    }


    if(!hasGuides)
    {
        QListWidgetItem *emptyItem =
            new QListWidgetItem(
                "Nessuna guida appartenente a questo gruppo.",
                m_list
                );


        emptyItem->setFlags(
            Qt::NoItemFlags
            );
    }
}



// ==================================================
// ADD LINE TO GROUP
// ==================================================

void DistanceGuideGroupConfigWindow::addLineToGroup()
{
    if(!m_manager)
        return;

    if(!m_manager->containsGroup(m_groupId))
        return;

    bool ok = false;

    const QString name =
        QInputDialog::getText(
            this,
            "Aggiungi linea",
            "Nome:",
            QLineEdit::Normal,
            "Nuova linea",
            &ok
            );

    if(!ok)
        return;

    if(name.trimmed().isEmpty())
        return;

    if(!m_manager->addGroupGuide(
            m_groupId,
            name,
            DistanceGuideType::VerticalLine,
            Qt::white
            ))
    {
        return;
    }

    refresh();
}

// ==================================================
// ADD RECTANGLE TO GROUP
// ==================================================

void DistanceGuideGroupConfigWindow::addRectangleToGroup()
{
    if(!m_manager)
        return;

    if(!m_manager->containsGroup(m_groupId))
        return;

    bool ok = false;

    const QString name =
        QInputDialog::getText(
            this,
            "Aggiungi rettangolo",
            "Nome:",
            QLineEdit::Normal,
            "Nuovo rettangolo",
            &ok
            );

    if(!ok)
        return;

    if(name.trimmed().isEmpty())
        return;

    if(!m_manager->addGroupGuide(
            m_groupId,
            name,
            DistanceGuideType::Rectangle,
            Qt::white
            ))
    {
        return;
    }

    refresh();
}

// ==================================================
// ADD CIRCLE TO GROUP
// ==================================================

void DistanceGuideGroupConfigWindow::addCircleToGroup()
{
    if(!m_manager)
        return;

    if(!m_manager->containsGroup(m_groupId))
        return;

    bool ok = false;

    const QString name =
        QInputDialog::getText(
            this,
            "Aggiungi cerchio",
            "Nome:",
            QLineEdit::Normal,
            "Nuovo cerchio",
            &ok
            );

    if(!ok)
        return;

    if(name.trimmed().isEmpty())
        return;

    if(!m_manager->addGroupGuide(
            m_groupId,
            name,
            DistanceGuideType::Circle,
            Qt::white
            ))
    {
        return;
    }

    refresh();
}

// ==================================================
// REMOVE GUIDE
// ==================================================

void DistanceGuideGroupConfigWindow::removeGuideFromGroup(
    const QString &guideId
    )
{
    if(!m_manager)
        return;


    m_manager->removeGroupGuide(
        m_groupId,
        guideId
        );


    refresh();
}


// ==================================================
// CLOSE
// ==================================================

void DistanceGuideGroupConfigWindow::closeEvent(
    QCloseEvent *event
    )
{
    event->accept();
}