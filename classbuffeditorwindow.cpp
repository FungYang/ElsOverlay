
#include "classbuffeditorwindow.h"
#include "newbuffdialog.h"

#include <QCloseEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "buffbox.h"
#include "globalkeyboard.h"


#ifdef Q_OS_WIN
#include <windows.h>
#endif


    // ============================================================
    // KEY NAME
    // ============================================================

    static QString keyName(
        int key
        )
{
#ifdef Q_OS_WIN

    switch(key)
    {
    case VK_LCONTROL:
        return "L-Ctrl";

    case VK_RCONTROL:
        return "R-Ctrl";

    case VK_LSHIFT:
        return "L-Shift";

    case VK_RSHIFT:
        return "R-Shift";

    case VK_LMENU:
        return "L-Alt";

    case VK_RMENU:
        return "R-Alt";

    case VK_LWIN:
        return "L-Win";

    case VK_RWIN:
        return "R-Win";

    case VK_SPACE:
        return "Space";

    case VK_RETURN:
        return "Enter";

    case VK_ESCAPE:
        return "Esc";

    case VK_TAB:
        return "Tab";

    case VK_BACK:
        return "Back";

    case VK_UP:
        return "Up";

    case VK_DOWN:
        return "Down";

    case VK_LEFT:
        return "Left";

    case VK_RIGHT:
        return "Right";

    case VK_DELETE:
        return "Del";

    case VK_INSERT:
        return "Ins";

    case VK_HOME:
        return "Home";

    case VK_END:
        return "End";

    case VK_PRIOR:
        return "PgUp";

    case VK_NEXT:
        return "PgDn";

    case VK_F1:
        return "F1";

    case VK_F2:
        return "F2";

    case VK_F3:
        return "F3";

    case VK_F4:
        return "F4";

    case VK_F5:
        return "F5";

    case VK_F6:
        return "F6";

    case VK_F7:
        return "F7";

    case VK_F8:
        return "F8";

    case VK_F9:
        return "F9";

    case VK_F10:
        return "F10";

    case VK_F11:
        return "F11";

    case VK_F12:
        return "F12";

    case VK_F13:
        return "F13";

    case VK_F14:
        return "F14";

    case VK_F15:
        return "F15";

    case VK_F16:
        return "F16";

    case VK_F17:
        return "F17";

    case VK_F18:
        return "F18";

    case VK_F19:
        return "F19";

    case VK_F20:
        return "F20";

    case VK_F21:
        return "F21";

    case VK_F22:
        return "F22";

    case VK_F23:
        return "F23";

    case VK_F24:
        return "F24";

    default:
        break;
    }


    if(
        key >= 'A' &&
        key <= 'Z'
        )
    {
        return QString(
            QChar(key)
            );
    }


    if(
        key >= '0' &&
        key <= '9'
        )
    {
        return QString(
            QChar(key)
            );
    }

#endif

    return QString::number(
        key
        );
}


// ============================================================
// CONSTRUCTOR
// ============================================================

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


    if(m_keyboard)
    {
        connect(
            m_keyboard,
            &GlobalKeyboard::confirmPressed,
            this,
            &ClassBuffEditorWindow::confirmPositions
            );
    }
}


// ============================================================
// CREATE UI
// ============================================================

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


// ============================================================
// LOAD CONFIGURATION
// ============================================================

void ClassBuffEditorWindow::loadConfiguration()
{
    if(!m_manager)
    {
        return;
    }


    m_buffs =
        m_manager->buffs(
            m_configurationId
            );


    createPreviewBoxes();
}


// ============================================================
// REFRESH LIST
// ============================================================

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
                keyName(buff.key),
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
            80
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


// ============================================================
// ADD NEW KEY
// ============================================================

void ClassBuffEditorWindow::addNewKey()
{
    NewBuffDialog dialog(
        this
        );


    if(
        dialog.exec() !=
        QDialog::Accepted
        )
    {
        return;
    }


    int keyCode =
        dialog.keyCode();


    // ========================================================
    // CHECK KEY
    // ========================================================

    if(keyCode == 0)
    {
        QMessageBox::warning(
            this,
            "Errore",
            "Tasto non valido."
            );

        return;
    }


    // ========================================================
    // CHECK DUPLICATE
    // ========================================================

    for(const BuffConfiguration &buff :
         m_buffs)
    {
        if(buff.key == keyCode)
        {
            QMessageBox::warning(
                this,
                "Errore",
                "Questo tasto è già configurato."
                );

            return;
        }
    }


    // ========================================================
    // CREATE CONFIGURATION
    // ========================================================

    BuffConfiguration configuration;


    configuration.key =
        keyCode;


    configuration.cooldown =
        dialog.cooldown();


    configuration.position =
        QPoint(
            100,
            100
            );


    configuration.size =
        QSize(
            42,
            42
            );


    m_buffs.append(
        configuration
        );


    // ========================================================
    // UPDATE UI
    // ========================================================

    refreshList();


    createPreviewBoxes();
}


// ============================================================
// DELETE BUFF
// ============================================================

void ClassBuffEditorWindow::deleteBuff(
    int index
    )
{
    if(
        index < 0 ||
        index >= m_buffs.size()
        )
    {
        return;
    }


    m_buffs.removeAt(
        index
        );


    refreshList();


    createPreviewBoxes();
}


// ============================================================
// SAVE CONFIGURATION
// ============================================================

void ClassBuffEditorWindow::saveConfiguration()
{
    if(!m_manager)
    {
        return;
    }


    /*
     * Salviamo l'intera lista.
     *
     * La posizione e la dimensione sono già
     * contenute in m_buffs.
     */

    m_manager->setBuffs(
        m_configurationId,
        m_buffs
        );


    emit configurationSaved();


    close();
}


// ============================================================
// CLOSE EVENT
// ============================================================

void ClassBuffEditorWindow::closeEvent(
    QCloseEvent *event
    )
{
    clearPreviewBoxes();


    emit editorClosed();


    event->accept();
}


// ============================================================
// CLEAR PREVIEW BOXES
// ============================================================

void ClassBuffEditorWindow::clearPreviewBoxes()
{
    qDeleteAll(
        m_previewBoxes
        );


    m_previewBoxes.clear();
}


// ============================================================
// CREATE PREVIEW BOXES
// ============================================================

void ClassBuffEditorWindow::createPreviewBoxes()
{
    clearPreviewBoxes();


    for(const BuffConfiguration &buff :
         m_buffs)
    {
        BuffBox *box =
            new BuffBox(
                buff.key,
                buff.cooldown,
                nullptr
                );


        // ====================================================
        // APPLICA DIMENSIONE SALVATA
        // ====================================================

        box->resize(
            buff.size
            );


        // ====================================================
        // APPLICA POSIZIONE SALVATA
        // ====================================================

        box->move(
            buff.position
            );


        box->setConfigurationMode(
            true
            );


        box->show();


        m_previewBoxes.append(
            box
            );
    }
}


// ============================================================
// CONFIRM POSITIONS
// ============================================================

void ClassBuffEditorWindow::confirmPositions()
{
    if(!m_manager)
    {
        return;
    }


    // ========================================================
    // SALVA POSIZIONE E DIMENSIONE
    // ========================================================

    for(int i = 0;
         i < m_previewBoxes.size() &&
         i < m_buffs.size();
         ++i)
    {
        m_buffs[i].position =
            m_previewBoxes[i]->pos();


        m_buffs[i].size =
            m_previewBoxes[i]->size();
    }


    // ========================================================
    // SALVA CONFIGURAZIONE
    // ========================================================

    m_manager->setBuffs(
        m_configurationId,
        m_buffs
        );


    emit configurationSaved();


    clearPreviewBoxes();
}
