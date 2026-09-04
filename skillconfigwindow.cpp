#include "skillconfigwindow.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

#include "keyedit.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

SkillConfigWindow::SkillConfigWindow(
    QWidget *parent
    )
    : QDialog(parent)
{
    setWindowTitle(
        "Configurazione Buff Titles"
        );


    setMinimumSize(
        650,
        1000
        );


    buildUi();

    loadConfig();
}


// ============================================================
// UI
// ============================================================

void SkillConfigWindow::buildUi()
{
    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);


    // ========================================================
    // TASTI GENERALI
    // ========================================================

    QGroupBox *keysGroup =
        new QGroupBox(
            "Tasti principali",
            this
            );


    QFormLayout *keysLayout =
        new QFormLayout(
            keysGroup
            );


    stateKeyEdit =
        new KeyEdit(
            keysGroup
            );


    cipollaKeyEdit =
        new KeyEdit(
            keysGroup
            );


    keysLayout->addRow(
        "Tasto stato:",
        stateKeyEdit
        );


    keysLayout->addRow(
        "Cipolla:",
        cipollaKeyEdit
        );


    mainLayout->addWidget(
        keysGroup
        );


    // ========================================================
    // SCALA BUFF TITLES
    // ========================================================

    QGroupBox *scaleGroup =
        new QGroupBox(
            "Dimensione Buff Titles",
            this
            );


    QFormLayout *scaleLayout =
        new QFormLayout(
            scaleGroup
            );


    scaleCombo =
        new QComboBox(
            scaleGroup
            );


    // --------------------------------------------------------
    // VALORI SCALA
    // --------------------------------------------------------

    scaleCombo->addItem(
        "50%",
        0.50
        );


    scaleCombo->addItem(
        "75%",
        0.75
        );


    scaleCombo->addItem(
        "100%",
        1.00
        );


    scaleCombo->addItem(
        "125%",
        1.25
        );


    scaleCombo->addItem(
        "150%",
        1.50
        );


    scaleCombo->addItem(
        "175%",
        1.75
        );


    scaleCombo->addItem(
        "200%",
        2.00
        );


    scaleCombo->addItem(
        "250%",
        2.50
        );


    scaleCombo->addItem(
        "300%",
        3.00
        );


    scaleLayout->addRow(
        "Scala:",
        scaleCombo
        );


    mainLayout->addWidget(
        scaleGroup
        );


    // ========================================================
    // SKILL UP
    // ========================================================

    mainLayout->addWidget(
        createSkillWidget(
            "↑ Skill Up",

            upNameEdit,
            upImageEdit,
            upCooldownSpin,
            upActivationCombo,

            upComboKeysList,
            upAddComboKeyButton,
            upRemoveComboKeyButton
            )
        );


    // ========================================================
    // SKILL LEFT
    // ========================================================

    mainLayout->addWidget(
        createSkillWidget(
            "← Skill Left",

            leftNameEdit,
            leftImageEdit,
            leftCooldownSpin,
            leftActivationCombo,

            leftComboKeysList,
            leftAddComboKeyButton,
            leftRemoveComboKeyButton
            )
        );


    // ========================================================
    // SKILL DOWN
    // ========================================================

    mainLayout->addWidget(
        createSkillWidget(
            "↓ Skill Down",

            downNameEdit,
            downImageEdit,
            downCooldownSpin,
            downActivationCombo,

            downComboKeysList,
            downAddComboKeyButton,
            downRemoveComboKeyButton
            )
        );


    // ========================================================
    // SKILL RIGHT
    // ========================================================

    mainLayout->addWidget(
        createSkillWidget(
            "→ Skill Right",

            rightNameEdit,
            rightImageEdit,
            rightCooldownSpin,
            rightActivationCombo,

            rightComboKeysList,
            rightAddComboKeyButton,
            rightRemoveComboKeyButton
            )
        );


    // ========================================================
    // BUTTONS
    // ========================================================

    QHBoxLayout *buttonsLayout =
        new QHBoxLayout;


    cancelButton =
        new QPushButton(
            "Annulla",
            this
            );


    saveButton =
        new QPushButton(
            "Salva",
            this
            );


    buttonsLayout->addStretch();


    buttonsLayout->addWidget(
        cancelButton
        );


    buttonsLayout->addWidget(
        saveButton
        );


    mainLayout->addLayout(
        buttonsLayout
        );


    // ========================================================
    // SIGNALS
    // ========================================================

    connect(
        cancelButton,
        &QPushButton::clicked,
        this,
        &QDialog::reject
        );


    connect(
        saveButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            readUiToConfig();


            saveConfig();


            emit configurationChanged(
                m_config
                );


            accept();
        }
        );
}


// ============================================================
// CREATE SKILL WIDGET
// ============================================================

QWidget *SkillConfigWindow::createSkillWidget(
    const QString &title,

    QLineEdit *&nameEdit,
    QLineEdit *&imageEdit,
    QSpinBox *&cooldownSpin,
    QComboBox *&activationCombo,

    QListWidget *&comboKeysList,
    QPushButton *&addComboKeyButton,
    QPushButton *&removeComboKeyButton
    )
{
    QGroupBox *group =
        new QGroupBox(
            title,
            this
            );


    QFormLayout *layout =
        new QFormLayout(
            group
            );


    // ========================================================
    // NOME
    // ========================================================

    nameEdit =
        new QLineEdit(
            group
            );


    layout->addRow(
        "Nome:",
        nameEdit
        );


    // ========================================================
    // IMMAGINE
    // ========================================================

    imageEdit =
        new QLineEdit(
            group
            );


    QPushButton *browseButton =
        new QPushButton(
            "Sfoglia",
            group
            );


    QHBoxLayout *imageLayout =
        new QHBoxLayout;


    imageLayout->addWidget(
        imageEdit
        );


    imageLayout->addWidget(
        browseButton
        );


    layout->addRow(
        "Immagine:",
        imageLayout
        );


    connect(
        browseButton,
        &QPushButton::clicked,
        this,
        [this, imageEdit]()
        {
            QString path =
                QFileDialog::getOpenFileName(
                    this,
                    "Seleziona immagine",
                    QCoreApplication::applicationDirPath(),
                    "Immagini (*.png *.jpg *.jpeg *.bmp)"
                    );


            if(!path.isEmpty())
            {
                imageEdit->setText(
                    path
                    );
            }
        }
        );


    // ========================================================
    // COOLDOWN
    // ========================================================

    cooldownSpin =
        new QSpinBox(
            group
            );


    cooldownSpin->setRange(
        0,
        9999
        );


    cooldownSpin->setSuffix(
        " sec"
        );


    layout->addRow(
        "Cooldown:",
        cooldownSpin
        );


    // ========================================================
    // ATTIVAZIONE
    // ========================================================

    activationCombo =
        new QComboBox(
            group
            );


    // --------------------------------------------------------
    // CTRL / CIPOLLA
    // --------------------------------------------------------

    activationCombo->addItem(
        "CTRL / Cipolla",
        static_cast<int>(
            SkillActivation::CtrlCipolla
            )
        );


    // --------------------------------------------------------
    // COMBO
    // --------------------------------------------------------

    activationCombo->addItem(
        "Combo",
        static_cast<int>(
            SkillActivation::Combo
            )
        );


    // --------------------------------------------------------
    // ARTIFACT
    // --------------------------------------------------------

    activationCombo->addItem(
        "Artifact",
        static_cast<int>(
            SkillActivation::Artifact
            )
        );


    layout->addRow(
        "Attivazione:",
        activationCombo
        );


    // ========================================================
    // COMBO KEYS
    // ========================================================

    comboKeysList =
        new QListWidget(
            group
            );


    comboKeysList->setMaximumHeight(
        80
        );


    addComboKeyButton =
        new QPushButton(
            "Aggiungi",
            group
            );


    removeComboKeyButton =
        new QPushButton(
            "Rimuovi",
            group
            );


    QHBoxLayout *comboButtons =
        new QHBoxLayout;


    comboButtons->addWidget(
        addComboKeyButton
        );


    comboButtons->addWidget(
        removeComboKeyButton
        );


    QVBoxLayout *comboLayout =
        new QVBoxLayout;


    comboLayout->addWidget(
        comboKeysList
        );


    comboLayout->addLayout(
        comboButtons
        );


    layout->addRow(
        "Tasti Combo:",
        comboLayout
        );


    // ========================================================
    // SIGNALS COMBO
    // ========================================================

    connect(
        addComboKeyButton,
        &QPushButton::clicked,
        this,
        [this, comboKeysList]()
        {
            addComboKey(
                comboKeysList
                );
        }
        );


    connect(
        removeComboKeyButton,
        &QPushButton::clicked,
        this,
        [this, comboKeysList]()
        {
            removeComboKey(
                comboKeysList
                );
        }
        );


    return group;
}


// ============================================================
// LOAD CONFIG
// ============================================================

void SkillConfigWindow::loadConfig()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    // ========================================================
    // TASTI GENERALI
    // ========================================================

    m_config.stateKey =
        settings.value(
                    "BuffTitles/StateKey",
                    'G'
                    ).toInt();


    m_config.cipollaKey =
        settings.value(
                    "BuffTitles/CipollaKey",
                    '6'
                    ).toInt();


    // ========================================================
    // SCALA BUFF TITLES
    // ========================================================

    m_config.scale =
        settings.value(
                    "Overlay/BuffGroup/Scale",
                    1.0
                    ).toDouble();


    // --------------------------------------------------------
    // LIMITI
    // --------------------------------------------------------

    if(m_config.scale < 0.50)
    {
        m_config.scale = 0.50;
    }


    if(m_config.scale > 3.00)
    {
        m_config.scale = 3.00;
    }


    // ========================================================
    // UP
    // ========================================================

    m_config.up.name =
        settings.value(
                    "BuffTitles/Up/Name",
                    "Concerto"
                    ).toString();


    m_config.up.imagePath =
        settings.value(
                    "BuffTitles/Up/Image",
                    "images/concerto.png"
                    ).toString();


    m_config.up.cooldown =
        settings.value(
                    "BuffTitles/Up/Cooldown",
                    60
                    ).toInt();


    m_config.up.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Up/Activation",
                        static_cast<int>(
                            SkillActivation::CtrlCipolla
                            )
                        ).toInt()
            );


    // ========================================================
    // LEFT
    // ========================================================

    m_config.left.name =
        settings.value(
                    "BuffTitles/Left/Name",
                    "Night Parade"
                    ).toString();


    m_config.left.imagePath =
        settings.value(
                    "BuffTitles/Left/Image",
                    "images/nightparade.png"
                    ).toString();


    m_config.left.cooldown =
        settings.value(
                    "BuffTitles/Left/Cooldown",
                    25
                    ).toInt();


    m_config.left.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Left/Activation",
                        static_cast<int>(
                            SkillActivation::Combo
                            )
                        ).toInt()
            );


    // ========================================================
    // DOWN
    // ========================================================

    m_config.down.name =
        settings.value(
                    "BuffTitles/Down/Name",
                    "Setting Sun"
                    ).toString();


    m_config.down.imagePath =
        settings.value(
                    "BuffTitles/Down/Image",
                    "images/settingsun.png"
                    ).toString();


    m_config.down.cooldown =
        settings.value(
                    "BuffTitles/Down/Cooldown",
                    30
                    ).toInt();


    m_config.down.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Down/Activation",
                        static_cast<int>(
                            SkillActivation::CtrlCipolla
                            )
                        ).toInt()
            );


    // ========================================================
    // RIGHT
    // ========================================================

    m_config.right.name =
        settings.value(
                    "BuffTitles/Right/Name",
                    "Other"
                    ).toString();


    m_config.right.imagePath =
        settings.value(
                    "BuffTitles/Right/Image",
                    "images/artifact.png"
                    ).toString();


    m_config.right.cooldown =
        settings.value(
                    "BuffTitles/Right/Cooldown",
                    20
                    ).toInt();


    m_config.right.activation =
        static_cast<SkillActivation>(
            settings.value(
                        "BuffTitles/Right/Activation",
                        static_cast<int>(
                            SkillActivation::Artifact
                            )
                        ).toInt()
            );


    // ========================================================
    // COMBO KEYS
    // ========================================================

    auto loadSkillCombo =
        [&settings](
            const QString &prefix,
            QList<int> &keys,
            const QList<int> &defaults
            )
    {
        keys.clear();


        int size =
            settings.beginReadArray(
                prefix + "/ComboKeys"
                );


        if(size == 0)
        {
            keys = defaults;
        }
        else
        {
            for(int i = 0;
                 i < size;
                 ++i)
            {
                settings.setArrayIndex(
                    i
                    );


                int key =
                    settings.value(
                                "Key",
                                0
                                ).toInt();


                if(key != 0)
                {
                    keys.append(
                        key
                        );
                }
            }
        }


        settings.endArray();
    };


    loadSkillCombo(
        "BuffTitles/Up",
        m_config.up.comboKeys,
        {}
        );


    loadSkillCombo(
        "BuffTitles/Left",
        m_config.left.comboKeys,
        {
            'F',
            'T'
        }
        );


    loadSkillCombo(
        "BuffTitles/Down",
        m_config.down.comboKeys,
        {}
        );


    loadSkillCombo(
        "BuffTitles/Right",
        m_config.right.comboKeys,
        {}
        );


    // ========================================================
    // UI
    // ========================================================

    stateKeyEdit->setKeyCode(
        m_config.stateKey
        );


    cipollaKeyEdit->setKeyCode(
        m_config.cipollaKey
        );


    // ========================================================
    // CARICA SCALA NEL COMBOBOX
    // ========================================================

    int scaleIndex =
        scaleCombo->findData(
            m_config.scale
            );


    if(scaleIndex >= 0)
    {
        scaleCombo->setCurrentIndex(
            scaleIndex
            );
    }
    else
    {
        scaleCombo->setCurrentIndex(
            scaleCombo->findData(
                1.0
                )
            );
    }


    // ========================================================
    // LOAD SKILL
    // ========================================================

    loadSkill(
        m_config.up,
        upNameEdit,
        upImageEdit,
        upCooldownSpin,
        upActivationCombo,
        upComboKeysList
        );


    loadSkill(
        m_config.left,
        leftNameEdit,
        leftImageEdit,
        leftCooldownSpin,
        leftActivationCombo,
        leftComboKeysList
        );


    loadSkill(
        m_config.down,
        downNameEdit,
        downImageEdit,
        downCooldownSpin,
        downActivationCombo,
        downComboKeysList
        );


    loadSkill(
        m_config.right,
        rightNameEdit,
        rightImageEdit,
        rightCooldownSpin,
        rightActivationCombo,
        rightComboKeysList
        );
}


// ============================================================
// LOAD SKILL
// ============================================================

void SkillConfigWindow::loadSkill(
    const SkillConfig &config,

    QLineEdit *nameEdit,
    QLineEdit *imageEdit,
    QSpinBox *cooldownSpin,
    QComboBox *activationCombo,

    QListWidget *comboKeysList
    )
{
    nameEdit->setText(
        config.name
        );


    imageEdit->setText(
        config.imagePath
        );


    cooldownSpin->setValue(
        config.cooldown
        );


    int index =
        activationCombo->findData(
            static_cast<int>(
                config.activation
                )
            );


    if(index >= 0)
    {
        activationCombo->setCurrentIndex(
            index
            );
    }
    else
    {
        activationCombo->setCurrentIndex(
            0
            );
    }


    loadComboKeys(
        config.comboKeys,
        comboKeysList
        );
}


// ============================================================
// LOAD COMBO KEYS
// ============================================================

void SkillConfigWindow::loadComboKeys(
    const QList<int> &keys,
    QListWidget *list
    )
{
    list->clear();


    for(int key : keys)
    {
        if(key <= 0)
        {
            continue;
        }


        list->addItem(
            QString(
                QChar(key)
                )
            );
    }
}


// ============================================================
// READ UI
// ============================================================

void SkillConfigWindow::readUiToConfig()
{
    // ========================================================
    // TASTI GENERALI
    // ========================================================

    if(stateKeyEdit->keyCode() != 0)
    {
        m_config.stateKey =
            stateKeyEdit->keyCode();
    }


    if(cipollaKeyEdit->keyCode() != 0)
    {
        m_config.cipollaKey =
            cipollaKeyEdit->keyCode();
    }


    // ========================================================
    // SCALA BUFF TITLES
    // ========================================================

    if(scaleCombo)
    {
        m_config.scale =
            scaleCombo->currentData()
                .toDouble();
    }


    // --------------------------------------------------------
    // LIMITI
    // --------------------------------------------------------

    if(m_config.scale < 0.50)
    {
        m_config.scale = 0.50;
    }


    if(m_config.scale > 3.00)
    {
        m_config.scale = 3.00;
    }


    // ========================================================
    // SKILL UP
    // ========================================================

    m_config.up =
        readSkill(
            upNameEdit,
            upImageEdit,
            upCooldownSpin,
            upActivationCombo,
            upComboKeysList
            );


    // ========================================================
    // SKILL LEFT
    // ========================================================

    m_config.left =
        readSkill(
            leftNameEdit,
            leftImageEdit,
            leftCooldownSpin,
            leftActivationCombo,
            leftComboKeysList
            );


    // ========================================================
    // SKILL DOWN
    // ========================================================

    m_config.down =
        readSkill(
            downNameEdit,
            downImageEdit,
            downCooldownSpin,
            downActivationCombo,
            downComboKeysList
            );


    // ========================================================
    // SKILL RIGHT
    // ========================================================

    m_config.right =
        readSkill(
            rightNameEdit,
            rightImageEdit,
            rightCooldownSpin,
            rightActivationCombo,
            rightComboKeysList
            );
}


// ============================================================
// READ SKILL
// ============================================================

SkillConfig SkillConfigWindow::readSkill(
    QLineEdit *nameEdit,
    QLineEdit *imageEdit,
    QSpinBox *cooldownSpin,
    QComboBox *activationCombo,

    QListWidget *comboKeysList
    ) const
{
    SkillConfig config;


    // ========================================================
    // DATI BASE
    // ========================================================

    config.name =
        nameEdit->text();


    config.imagePath =
        imageEdit->text();


    config.cooldown =
        cooldownSpin->value();


    config.activation =
        static_cast<SkillActivation>(
            activationCombo->currentData()
                .toInt()
            );


    // ========================================================
    // ARTIFACT
    // ========================================================
    //
    // Se questa direzione è configurata come Artifact,
    // i ComboKeys non hanno alcun significato.
    //

    if(config.activation ==
        SkillActivation::Artifact)
    {
        config.comboKeys.clear();


        return config;
    }


    // ========================================================
    // COMBO KEYS
    // ========================================================

    for(int i = 0;
         i < comboKeysList->count();
         ++i)
    {
        QListWidgetItem *item =
            comboKeysList->item(i);


        if(!item)
        {
            continue;
        }


        QString text =
            item->text()
                .trimmed();


        if(text.isEmpty())
        {
            continue;
        }


        int key =
            text.at(0)
                .toUpper()
                .unicode();


        if(!config.comboKeys.contains(key))
        {
            config.comboKeys.append(
                key
                );
        }
    }


    return config;
}


// ============================================================
// SAVE CONFIG
// ============================================================

void SkillConfigWindow::saveConfig()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    // ========================================================
    // TASTI GENERALI
    // ========================================================

    settings.setValue(
        "BuffTitles/StateKey",
        m_config.stateKey
        );


    settings.setValue(
        "BuffTitles/CipollaKey",
        m_config.cipollaKey
        );


    // ========================================================
    // SCALA BUFF TITLES
    // ========================================================

    settings.setValue(
        "Overlay/BuffGroup/Scale",
        m_config.scale
        );


    // ========================================================
    // SALVA SKILL
    // ========================================================

    auto saveSkill =
        [&settings](
            const QString &prefix,
            const SkillConfig &config
            )
    {
        settings.setValue(
            prefix + "/Name",
            config.name
            );


        settings.setValue(
            prefix + "/Image",
            config.imagePath
            );


        settings.setValue(
            prefix + "/Cooldown",
            config.cooldown
            );


        settings.setValue(
            prefix + "/Activation",
            static_cast<int>(
                config.activation
                )
            );


        // ----------------------------------------------------
        // COMBO KEYS
        // ----------------------------------------------------

        settings.beginWriteArray(
            prefix + "/ComboKeys"
            );


        for(int i = 0;
             i < config.comboKeys.size();
             ++i)
        {
            settings.setArrayIndex(
                i
                );


            settings.setValue(
                "Key",
                config.comboKeys.at(i)
                );
        }


        settings.endArray();
    };


    // ========================================================
    // UP
    // ========================================================

    saveSkill(
        "BuffTitles/Up",
        m_config.up
        );


    // ========================================================
    // LEFT
    // ========================================================

    saveSkill(
        "BuffTitles/Left",
        m_config.left
        );


    // ========================================================
    // DOWN
    // ========================================================

    saveSkill(
        "BuffTitles/Down",
        m_config.down
        );


    // ========================================================
    // RIGHT
    // ========================================================

    saveSkill(
        "BuffTitles/Right",
        m_config.right
        );


    settings.sync();
}


// ============================================================
// ADD COMBO KEY
// ============================================================

void SkillConfigWindow::addComboKey(
    QListWidget *list
    )
{
    bool ok = false;


    QString key =
        QInputDialog::getText(
            this,
            "Aggiungi tasto",
            "Inserisci il tasto:",
            QLineEdit::Normal,
            "",
            &ok
            );


    if(!ok)
    {
        return;
    }


    QString normalized =
        key.trimmed()
            .left(1)
            .toUpper();


    if(normalized.isEmpty())
    {
        return;
    }


    // ========================================================
    // EVITA DUPLICATI
    // ========================================================

    for(int i = 0;
         i < list->count();
         ++i)
    {
        if(list->item(i)->text() ==
            normalized)
        {
            return;
        }
    }


    list->addItem(
        normalized
        );
}


// ============================================================
// REMOVE COMBO KEY
// ============================================================

void SkillConfigWindow::removeComboKey(
    QListWidget *list
    )
{
    QListWidgetItem *item =
        list->currentItem();


    if(!item)
    {
        return;
    }


    delete list->takeItem(
        list->row(item)
        );
}


// ============================================================
// GET CONFIG
// ============================================================

SkillOverlayConfig SkillConfigWindow::config() const
{
    return m_config;
}
