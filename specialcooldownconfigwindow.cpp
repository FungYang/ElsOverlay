
#include "specialcooldownconfigwindow.h"

#include "keyedit.h"
#include "specialcooldownmanager.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>


SpecialCooldownConfigWindow::SpecialCooldownConfigWindow(
    SpecialCooldownManager *manager,
    QWidget *parent
    )
    : QDialog(parent),
    m_manager(manager)
{
    setWindowTitle(
        "Special Cooldowns"
        );

    resize(
        600,
        400
        );


    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            this
            );


    m_listWidget =
        new QListWidget(
            this
            );

    m_listWidget->setIconSize(
        QSize(
            50,
            50
            )
        );

    m_listWidget->setSpacing(
        4
        );


    mainLayout->addWidget(
        m_listWidget
        );


    QHBoxLayout *buttonLayout =
        new QHBoxLayout();


    m_addButton =
        new QPushButton(
            "Aggiungi",
            this
            );


    m_removeButton =
        new QPushButton(
            "Rimuovi",
            this
            );


    buttonLayout->addWidget(
        m_addButton
        );

    buttonLayout->addWidget(
        m_removeButton
        );

    buttonLayout->addStretch();


    m_saveButton =
        new QPushButton(
            "Salva",
            this
            );


    m_cancelButton =
        new QPushButton(
            "Annulla",
            this
            );


    buttonLayout->addWidget(
        m_saveButton
        );

    buttonLayout->addWidget(
        m_cancelButton
        );


    mainLayout->addLayout(
        buttonLayout
        );


    connect(
        m_addButton,
        &QPushButton::clicked,
        this,
        &SpecialCooldownConfigWindow::addSpecialCooldown
        );


    connect(
        m_removeButton,
        &QPushButton::clicked,
        this,
        &SpecialCooldownConfigWindow::removeSpecialCooldown
        );


    connect(
        m_listWidget,
        &QListWidget::itemDoubleClicked,
        this,
        &SpecialCooldownConfigWindow::editSpecialCooldown
        );


    connect(
        m_saveButton,
        &QPushButton::clicked,
        this,
        &SpecialCooldownConfigWindow::saveConfiguration
        );


    connect(
        m_cancelButton,
        &QPushButton::clicked,
        this,
        &QDialog::reject
        );


    if(m_manager)
    {
        m_configurations =
            m_manager->configurations();
    }


    refreshList();
}


// ============================================================
// ADD
// ============================================================

void SpecialCooldownConfigWindow::addSpecialCooldown()
{
    QDialog dialog(
        this
        );


    dialog.setWindowTitle(
        "Nuovo Special Cooldown"
        );


    dialog.resize(
        500,
        400
        );


    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            &dialog
            );


    QFormLayout *formLayout =
        new QFormLayout();


    // ========================================================
    // TASTI ACCETTATI
    // ========================================================

    QVBoxLayout *keysLayout =
        new QVBoxLayout();


    QListWidget *acceptedKeysList =
        new QListWidget(
            &dialog
            );


    acceptedKeysList->setMinimumHeight(
        120
        );


    QHBoxLayout *keyInputLayout =
        new QHBoxLayout();


    KeyEdit *keyEdit =
        new KeyEdit(
            &dialog
            );


    QPushButton *addKeyButton =
        new QPushButton(
            "Aggiungi",
            &dialog
            );


    keyInputLayout->addWidget(
        keyEdit
        );

    keyInputLayout->addWidget(
        addKeyButton
        );


    QPushButton *removeKeyButton =
        new QPushButton(
            "Rimuovi selezionato",
            &dialog
            );


    keysLayout->addWidget(
        acceptedKeysList
        );

    keysLayout->addLayout(
        keyInputLayout
        );

    keysLayout->addWidget(
        removeKeyButton
        );


    formLayout->addRow(
        "Tasti accettati:",
        keysLayout
        );


    // ========================================================
    // IMMAGINE
    // ========================================================

    QHBoxLayout *imageLayout =
        new QHBoxLayout();


    QLineEdit *imageEdit =
        new QLineEdit(
            &dialog
            );


    QPushButton *browseButton =
        new QPushButton(
            "Sfoglia...",
            &dialog
            );


    imageLayout->addWidget(
        imageEdit
        );

    imageLayout->addWidget(
        browseButton
        );


    formLayout->addRow(
        "Immagine:",
        imageLayout
        );


    // ========================================================
    // COOLDOWN
    // ========================================================

    QSpinBox *cooldownSpinBox =
        new QSpinBox(
            &dialog
            );


    cooldownSpinBox->setRange(
        1,
        86400
        );


    cooldownSpinBox->setValue(
        60
        );


    formLayout->addRow(
        "Cooldown:",
        cooldownSpinBox
        );


    mainLayout->addLayout(
        formLayout
        );


    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(
            QDialogButtonBox::Ok |
                QDialogButtonBox::Cancel,
            &dialog
            );


    mainLayout->addWidget(
        buttonBox
        );


    // ========================================================
    // AGGIUNGI TASTO
    // ========================================================

    connect(
        addKeyButton,
        &QPushButton::clicked,
        &dialog,
        [this, keyEdit, acceptedKeysList]()
        {
            const int keyCode =
                keyEdit->keyCode();


            if(keyCode == 0)
            {
                return;
            }


            // ------------------------------------------------
            // Evita duplicati
            // ------------------------------------------------

            for(
                int i = 0;
                i < acceptedKeysList->count();
                ++i
                )
            {
                QListWidgetItem *item =
                    acceptedKeysList->item(
                        i
                        );


                if(
                    item->data(
                            Qt::UserRole
                            ).toInt() == keyCode
                    )
                {
                    return;
                }
            }


            QListWidgetItem *item =
                new QListWidgetItem(
                    keyName(
                        keyCode
                        )
                    );


            item->setData(
                Qt::UserRole,
                keyCode
                );


            acceptedKeysList->addItem(
                item
                );


            keyEdit->setKeyCode(
                0
                );
        }
        );


    // ========================================================
    // RIMUOVI TASTO
    // ========================================================

    connect(
        removeKeyButton,
        &QPushButton::clicked,
        &dialog,
        [acceptedKeysList]()
        {
            const int row =
                acceptedKeysList->currentRow();


            if(row < 0)
            {
                return;
            }


            delete acceptedKeysList->takeItem(
                row
                );
        }
        );


    // ========================================================
    // BROWSE IMMAGINE
    // ========================================================

    connect(
        browseButton,
        &QPushButton::clicked,
        &dialog,
        [imageEdit, &dialog]()
        {
            const QString filePath =
                QFileDialog::getOpenFileName(
                    &dialog,
                    "Seleziona immagine",
                    QString(),
                    "Immagini (*.png *.jpg *.jpeg *.bmp *.webp)"
                    );


            if(!filePath.isEmpty())
            {
                imageEdit->setText(
                    filePath
                    );
            }
        }
        );


    connect(
        buttonBox,
        &QDialogButtonBox::accepted,
        &dialog,
        &QDialog::accept
        );


    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        &dialog,
        &QDialog::reject
        );


    if(
        dialog.exec() !=
        QDialog::Accepted
        )
    {
        return;
    }


    // ========================================================
    // LETTURA DATI
    // ========================================================

    QList<int> acceptedKeys;


    for(
        int i = 0;
        i < acceptedKeysList->count();
        ++i
        )
    {
        acceptedKeys.append(
            acceptedKeysList->item(
                                i
                                )->data(
                    Qt::UserRole
                    ).toInt()
            );
    }


    const QString imagePath =
        imageEdit->text().trimmed();


    const int cooldown =
        cooldownSpinBox->value();


    // ========================================================
    // VALIDAZIONE
    // ========================================================

    if(acceptedKeys.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Configurazione",
            "Devi assegnare almeno un tasto."
            );

        return;
    }


    if(imagePath.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Configurazione",
            "Devi selezionare un'immagine."
            );

        return;
    }


    if(cooldown <= 0)
    {
        QMessageBox::warning(
            this,
            "Configurazione",
            "Il cooldown deve essere maggiore di zero."
            );

        return;
    }


    // ========================================================
    // CREA CONFIGURAZIONE
    // ========================================================

    SpecialCooldownConfiguration configuration;


    configuration.acceptedKeys =
        acceptedKeys;


    configuration.imagePath =
        imagePath;


    configuration.cooldown =
        cooldown;


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


    m_configurations.append(
        configuration
        );


    refreshList();
}


// ============================================================
// REMOVE
// ============================================================

void SpecialCooldownConfigWindow::removeSpecialCooldown()
{
    const int row =
        m_listWidget->currentRow();


    if(row < 0)
    {
        return;
    }


    const QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            "Rimuovi Special Cooldown",
            "Vuoi davvero rimuovere lo Special Cooldown selezionato?"
            );


    if(
        result !=
        QMessageBox::Yes
        )
    {
        return;
    }


    m_configurations.removeAt(
        row
        );


    refreshList();
}


// ============================================================
// EDIT
// ============================================================

void SpecialCooldownConfigWindow::editSpecialCooldown(
    QListWidgetItem *item
    )
{
    if(!item)
    {
        return;
    }


    const int row =
        m_listWidget->row(
            item
            );


    if(
        row < 0 ||
        row >= m_configurations.size()
        )
    {
        return;
    }


    SpecialCooldownConfiguration configuration =
        m_configurations.at(
            row
            );


    QDialog dialog(
        this
        );


    dialog.setWindowTitle(
        "Modifica Special Cooldown"
        );


    dialog.resize(
        500,
        400
        );


    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            &dialog
            );


    QFormLayout *formLayout =
        new QFormLayout();


    // ========================================================
    // TASTI ACCETTATI
    // ========================================================

    QVBoxLayout *keysLayout =
        new QVBoxLayout();


    QListWidget *acceptedKeysList =
        new QListWidget(
            &dialog
            );


    acceptedKeysList->setMinimumHeight(
        120
        );


    // --------------------------------------------------------
    // Carica tasti esistenti
    // --------------------------------------------------------

    for(
        int key :
        configuration.acceptedKeys
        )
    {
        QListWidgetItem *keyItem =
            new QListWidgetItem(
                keyName(
                    key
                    )
                );


        keyItem->setData(
            Qt::UserRole,
            key
            );


        acceptedKeysList->addItem(
            keyItem
            );
    }


    QHBoxLayout *keyInputLayout =
        new QHBoxLayout();


    KeyEdit *keyEdit =
        new KeyEdit(
            &dialog
            );


    QPushButton *addKeyButton =
        new QPushButton(
            "Aggiungi",
            &dialog
            );


    keyInputLayout->addWidget(
        keyEdit
        );

    keyInputLayout->addWidget(
        addKeyButton
        );


    QPushButton *removeKeyButton =
        new QPushButton(
            "Rimuovi selezionato",
            &dialog
            );


    keysLayout->addWidget(
        acceptedKeysList
        );

    keysLayout->addLayout(
        keyInputLayout
        );

    keysLayout->addWidget(
        removeKeyButton
        );


    formLayout->addRow(
        "Tasti accettati:",
        keysLayout
        );


    // ========================================================
    // IMMAGINE
    // ========================================================

    QHBoxLayout *imageLayout =
        new QHBoxLayout();


    QLineEdit *imageEdit =
        new QLineEdit(
            &dialog
            );


    imageEdit->setText(
        configuration.imagePath
        );


    QPushButton *browseButton =
        new QPushButton(
            "Sfoglia...",
            &dialog
            );


    imageLayout->addWidget(
        imageEdit
        );

    imageLayout->addWidget(
        browseButton
        );


    formLayout->addRow(
        "Immagine:",
        imageLayout
        );


    // ========================================================
    // COOLDOWN
    // ========================================================

    QSpinBox *cooldownSpinBox =
        new QSpinBox(
            &dialog
            );


    cooldownSpinBox->setRange(
        1,
        86400
        );


    cooldownSpinBox->setValue(
        configuration.cooldown
        );


    formLayout->addRow(
        "Cooldown:",
        cooldownSpinBox
        );


    mainLayout->addLayout(
        formLayout
        );


    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(
            QDialogButtonBox::Ok |
                QDialogButtonBox::Cancel,
            &dialog
            );


    mainLayout->addWidget(
        buttonBox
        );


    // ========================================================
    // AGGIUNGI TASTO
    // ========================================================

    connect(
        addKeyButton,
        &QPushButton::clicked,
        &dialog,
        [this, keyEdit, acceptedKeysList]()
        {
            const int keyCode =
                keyEdit->keyCode();


            if(keyCode == 0)
            {
                return;
            }


            // ------------------------------------------------
            // Evita duplicati
            // ------------------------------------------------

            for(
                int i = 0;
                i < acceptedKeysList->count();
                ++i
                )
            {
                QListWidgetItem *item =
                    acceptedKeysList->item(
                        i
                        );


                if(
                    item->data(
                            Qt::UserRole
                            ).toInt() == keyCode
                    )
                {
                    return;
                }
            }


            QListWidgetItem *item =
                new QListWidgetItem(
                    keyName(
                        keyCode
                        )
                    );


            item->setData(
                Qt::UserRole,
                keyCode
                );


            acceptedKeysList->addItem(
                item
                );


            keyEdit->setKeyCode(
                0
                );
        }
        );


    // ========================================================
    // RIMUOVI TASTO
    // ========================================================

    connect(
        removeKeyButton,
        &QPushButton::clicked,
        &dialog,
        [acceptedKeysList]()
        {
            const int row =
                acceptedKeysList->currentRow();


            if(row < 0)
            {
                return;
            }


            delete acceptedKeysList->takeItem(
                row
                );
        }
        );


    // ========================================================
    // BROWSE IMMAGINE
    // ========================================================

    connect(
        browseButton,
        &QPushButton::clicked,
        &dialog,
        [imageEdit, &dialog]()
        {
            const QString filePath =
                QFileDialog::getOpenFileName(
                    &dialog,
                    "Seleziona immagine",
                    QString(),
                    "Immagini (*.png *.jpg *.jpeg *.bmp *.webp)"
                    );


            if(!filePath.isEmpty())
            {
                imageEdit->setText(
                    filePath
                    );
            }
        }
        );


    connect(
        buttonBox,
        &QDialogButtonBox::accepted,
        &dialog,
        &QDialog::accept
        );


    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        &dialog,
        &QDialog::reject
        );


    if(
        dialog.exec() !=
        QDialog::Accepted
        )
    {
        return;
    }


    // ========================================================
    // LETTURA DATI
    // ========================================================

    QList<int> acceptedKeys;


    for(
        int i = 0;
        i < acceptedKeysList->count();
        ++i
        )
    {
        acceptedKeys.append(
            acceptedKeysList->item(
                                i
                                )->data(
                    Qt::UserRole
                    ).toInt()
            );
    }


    const QString imagePath =
        imageEdit->text().trimmed();


    const int cooldown =
        cooldownSpinBox->value();


    // ========================================================
    // VALIDAZIONE
    // ========================================================

    if(acceptedKeys.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Configurazione",
            "Devi assegnare almeno un tasto."
            );

        return;
    }


    if(imagePath.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Configurazione",
            "Devi selezionare un'immagine."
            );

        return;
    }


    if(cooldown <= 0)
    {
        QMessageBox::warning(
            this,
            "Configurazione",
            "Il cooldown deve essere maggiore di zero."
            );

        return;
    }


    // ========================================================
    // AGGIORNA CONFIGURAZIONE
    // ========================================================

    configuration.acceptedKeys =
        acceptedKeys;


    configuration.imagePath =
        imagePath;


    configuration.cooldown =
        cooldown;


    m_configurations[row] =
        configuration;


    refreshList();
}


// ============================================================
// SAVE
// ============================================================

void SpecialCooldownConfigWindow::saveConfiguration()
{
    if(!m_manager)
    {
        QMessageBox::warning(
            this,
            "Errore",
            "SpecialCooldownManager non disponibile."
            );

        return;
    }


    m_manager->setConfigurations(
        m_configurations
        );


    m_manager->save();


    emit configurationSaved();


    accept();
}


// ============================================================
// REFRESH LIST
// ============================================================

void SpecialCooldownConfigWindow::refreshList()
{
    m_listWidget->clear();


    for(
        const SpecialCooldownConfiguration &configuration :
        m_configurations
        )
    {
        QListWidgetItem *item =
            new QListWidgetItem();


        QPixmap pixmap(
            configuration.imagePath
            );


        if(!pixmap.isNull())
        {
            QPixmap thumbnail =
                pixmap.scaled(
                    50,
                    50,
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation
                    );


            item->setIcon(
                QIcon(
                    thumbnail
                    )
                );
        }


        item->setText(
            displayName(
                configuration
                )
            );


        item->setSizeHint(
            QSize(
                0,
                58
                )
            );


        m_listWidget->addItem(
            item
            );
    }
}


// ============================================================
// DISPLAY NAME
// ============================================================

QString SpecialCooldownConfigWindow::displayName(
    const SpecialCooldownConfiguration &configuration
    ) const
{
    const QFileInfo fileInfo(
        configuration.imagePath
        );


    const QString fileName =
        fileInfo.fileName();


    QStringList keyNames;


    for(
        int key :
        configuration.acceptedKeys
        )
    {
        keyNames.append(
            keyName(
                key
                )
            );
    }


    const QString keysText =
        keyNames.join(
            ", "
            );


    return QString(
               "%1    |    %2    |    %3 s"
               )
        .arg(
            keysText
            )
        .arg(
            fileName
            )
        .arg(
            configuration.cooldown
            );
}


// ============================================================
// KEY NAME
// ============================================================

QString SpecialCooldownConfigWindow::keyName(
    int key
    ) const
{
    // ========================================================
    // LETTERE
    // ========================================================

    if(
        key >= 'A' &&
        key <= 'Z'
        )
    {
        return QString(
            QChar(
                key
                )
            );
    }


    // ========================================================
    // NUMERI
    // ========================================================

    if(
        key >= '0' &&
        key <= '9'
        )
    {
        return QString(
            QChar(
                key
                )
            );
    }


    // ========================================================
    // TASTI SPECIALI
    // ========================================================

    switch(key)
    {
    case 162:
        return "Left Ctrl";

    case 163:
        return "Right Ctrl";

    case 160:
        return "Left Shift";

    case 161:
        return "Right Shift";

    case 164:
        return "Left Alt";

    case 165:
        return "Right Alt";

    case 91:
        return "Left Win";

    case 92:
        return "Right Win";

    case 32:
        return "Space";

    case 9:
        return "Tab";

    case 13:
        return "Enter";

    case 27:
        return "Esc";

    case 37:
        return "Left";

    case 38:
        return "Up";

    case 39:
        return "Right";

    case 40:
        return "Down";


        // --------------------------------------------------------
        // F1 - F12
        // --------------------------------------------------------

    case 112:
        return "F1";

    case 113:
        return "F2";

    case 114:
        return "F3";

    case 115:
        return "F4";

    case 116:
        return "F5";

    case 117:
        return "F6";

    case 118:
        return "F7";

    case 119:
        return "F8";

    case 120:
        return "F9";

    case 121:
        return "F10";

    case 122:
        return "F11";

    case 123:
        return "F12";


        // --------------------------------------------------------
        // CONTROLLO
        // --------------------------------------------------------

    case 8:
        return "Backspace";

    case 20:
        return "Caps Lock";

    case 144:
        return "Num Lock";

    case 145:
        return "Scroll Lock";

    case 19:
        return "Pause";

    case 33:
        return "Page Up";

    case 34:
        return "Page Down";

    case 35:
        return "End";

    case 36:
        return "Home";

    case 45:
        return "Insert";

    case 46:
        return "Delete";

    case 44:
        return "Print Screen";


    default:
        return QString(
                   "VK %1"
                   ).arg(
                key
                );
    }
}
