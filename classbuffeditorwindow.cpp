#include "classbuffeditorwindow.h"
#include "newbuffdialog.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "buffbox.h"
#include "globalkeyboard.h"


ClassBuffEditorWindow::ClassBuffEditorWindow(
    ClassConfigurationManager *manager,
    const QString &configurationId,
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    m_manager(manager),
    m_configurationId(configurationId),
    m_keyboard(keyboard)
{

    setWindowTitle(
        "Buff Configuration - " +
        configurationId
        );


    setFixedSize(
        500,
        500
        );


    createUi();

    loadConfiguration();



    refreshList();

    connect(
        m_keyboard,
        &GlobalKeyboard::confirmPressed,
        this,
        &ClassBuffEditorWindow::confirmPositions
        );

}

void ClassBuffEditorWindow::createUi()
{

    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            this
            );


    QLabel *title =
        new QLabel(
            m_configurationId,
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



    buffList =
        new QListWidget(
            this
            );


    mainLayout->addWidget(
        buffList
        );



    newKeyButton =
        new QPushButton(
            "New Key",
            this
            );


    newKeyButton->setMinimumHeight(
        40
        );


    mainLayout->addWidget(
        newKeyButton
        );



    QHBoxLayout *bottomLayout =
        new QHBoxLayout();


    saveButton =
        new QPushButton(
            "Salva",
            this
            );


    closeButton =
        new QPushButton(
            "Chiudi",
            this
            );


    bottomLayout->addWidget(
        saveButton
        );


    bottomLayout->addWidget(
        closeButton
        );


    mainLayout->addLayout(
        bottomLayout
        );



    connect(
        newKeyButton,
        &QPushButton::clicked,
        this,
        &ClassBuffEditorWindow::addNewKey
        );


    connect(
        saveButton,
        &QPushButton::clicked,
        this,
        &ClassBuffEditorWindow::saveConfiguration
        );


    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );

}

void ClassBuffEditorWindow::loadConfiguration()
{

    if(!m_manager)
        return;


    m_buffs =
        m_manager->buffs(
            m_configurationId
            );

}

void ClassBuffEditorWindow::refreshList()
{

    buffList->clear();


    for(int i = 0;
         i < m_buffs.size();
         ++i)
    {

        const BuffConfiguration &buff =
            m_buffs.at(i);


        QString cooldownText =
            QString::number(
                buff.cooldown
                ) +
            "s";


        QWidget *row =
            new QWidget();


        QHBoxLayout *layout =
            new QHBoxLayout(
                row
                );


        QLabel *keyLabel =
            new QLabel(
                QString(buff.key),
                row
                );


        QLabel *cooldownLabel =
            new QLabel(
                cooldownText,
                row
                );


        QPushButton *deleteButton =
            new QPushButton(
                "Delete",
                row
                );


        keyLabel->setMinimumWidth(
            40
            );


        cooldownLabel->setMinimumWidth(
            180
            );


        layout->addWidget(
            keyLabel
            );


        layout->addWidget(
            cooldownLabel
            );


        layout->addStretch();


        layout->addWidget(
            deleteButton
            );


        QListWidgetItem *item =
            new QListWidgetItem(
                buffList
                );


        item->setSizeHint(
            row->sizeHint()
            );


        buffList->setItemWidget(
            item,
            row
            );


        connect(
            deleteButton,
            &QPushButton::clicked,
            this,
            [this, i]()
            {
                deleteBuff(i);
            }
            );

    }

}
void ClassBuffEditorWindow::addNewKey()
{
    NewBuffDialog dialog(this);


    if(dialog.exec() != QDialog::Accepted)
        return;


    const QChar key =
        dialog.key();


    for(const BuffConfiguration &buff :
         m_buffs)
    {
        if(buff.key == key)
        {
            QMessageBox::warning(
                this,
                "Errore",
                "Questo tasto è già configurato."
                );

            return;
        }
    }


    BuffConfiguration configuration;


    configuration.key =
        key;


    configuration.cooldown =
        dialog.cooldown();


    configuration.position =
        QPoint(
            100,
            100
            );


    m_buffs.append(
        configuration
        );

    refreshList();

    createPreviewBoxes();


    refreshList();
}

void ClassBuffEditorWindow::deleteBuff(
    int index
    )
{

    if(index < 0 ||
        index >= m_buffs.size())
    {
        return;
    }


    m_buffs.removeAt(
        index
        );


    refreshList();

}

void ClassBuffEditorWindow::saveConfiguration()
{

    if(!m_manager)
        return;


    m_manager->setBuffs(
        m_configurationId,
        m_buffs
        );


    emit configurationSaved();


    close();

}

void ClassBuffEditorWindow::closeEvent(
    QCloseEvent *event
    )
{
    clearPreviewBoxes();

    emit configurationSaved();
    emit editorClosed();

    event->accept();
}

void ClassBuffEditorWindow::clearPreviewBoxes()
{
    qDeleteAll(m_previewBoxes);
    m_previewBoxes.clear();
}

void ClassBuffEditorWindow::createPreviewBoxes()
{
    clearPreviewBoxes();

    for(const BuffConfiguration &buff : m_buffs)
    {
        BuffBox *box =
            new BuffBox(
                buff.key,
                buff.cooldown,
                nullptr
                );

        box->move(
            buff.position
            );

        box->setConfigurationMode(true);

        box->show();

        m_previewBoxes.append(
            box
            );
    }
}


void ClassBuffEditorWindow::confirmPositions()
{
    if(!m_manager)
        return;


    for(int i = 0;
         i < m_previewBoxes.size() &&
         i < m_buffs.size();
         ++i)
    {
        m_buffs[i].position =
            m_previewBoxes[i]->pos();
    }


    m_manager->setBuffs(
        m_configurationId,
        m_buffs
        );


    clearPreviewBoxes();
}