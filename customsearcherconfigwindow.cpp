#include "customsearcherconfigwindow.h"

#include "customsearchermanager.h"

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>


CustomSearcherConfigWindow::CustomSearcherConfigWindow(
    CustomSearcherManager *manager,
    QWidget *parent
    )
    : QDialog(parent),
    m_manager(manager)
{
    setWindowTitle(
        QStringLiteral("Custom Searcher")
        );

    setModal(false);

    resize(
        620,
        560
        );


    QVBoxLayout *mainLayout =
        new QVBoxLayout(this);


    // ==================================================
    // TEMPLATE LIST
    // ==================================================

    QLabel *templatesLabel =
        new QLabel(
            QStringLiteral("Templates"),
            this
            );

    mainLayout->addWidget(
        templatesLabel
        );

    m_templateList =
        new QListWidget(this);

    mainLayout->addWidget(
        m_templateList,
        1
        );


    // ==================================================
    // PREVIEW
    // ==================================================

    m_previewLabel =
        new QLabel(this);

    m_previewLabel->setMinimumSize(
        180,
        180
        );

    m_previewLabel->setMaximumHeight(
        220
        );

    m_previewLabel->setAlignment(
        Qt::AlignCenter
        );

    m_previewLabel->setText(
        QStringLiteral("Nessun template")
        );

    m_previewLabel->setStyleSheet(
        QStringLiteral(
            "QLabel {"
            " border: 1px solid #666;"
            " background: #222;"
            "}"
            )
        );

    mainLayout->addWidget(
        m_previewLabel
        );


    // ==================================================
    // COOLDOWN
    // ==================================================

    QHBoxLayout *cooldownLayout =
        new QHBoxLayout();

    QLabel *cooldownLabel =
        new QLabel(
            QStringLiteral("Cooldown (secondi):"),
            this
            );

    m_cooldownSpin =
        new QSpinBox(this);

    m_cooldownSpin->setRange(
        0,
        3600
        );

    m_cooldownSpin->setValue(
        0
        );

    cooldownLayout->addWidget(
        cooldownLabel
        );

    cooldownLayout->addWidget(
        m_cooldownSpin
        );

    cooldownLayout->addStretch();

    mainLayout->addLayout(
        cooldownLayout
        );


    // ==================================================
    // SEARCH ZONE
    // ==================================================

    QHBoxLayout *searchZoneLayout =
        new QHBoxLayout();

    QLabel *searchZoneLabel =
        new QLabel(
            QStringLiteral("Zona ricerca:"),
            this
            );

    m_searchZoneCombo =
        new QComboBox(this);

    m_searchZoneCombo->addItem(
        QStringLiteral("Zona 1"),
        static_cast<int>(
            CustomSearcherManager::SearchZone::Zone1
            )
        );

    m_searchZoneCombo->addItem(
        QStringLiteral("Zona 2"),
        static_cast<int>(
            CustomSearcherManager::SearchZone::Zone2
            )
        );

    searchZoneLayout->addWidget(
        searchZoneLabel
        );

    searchZoneLayout->addWidget(
        m_searchZoneCombo
        );

    searchZoneLayout->addStretch();

    mainLayout->addLayout(
        searchZoneLayout
        );


    // ==================================================
    // CHECKBOX PRESETS
    // ==================================================

    QHBoxLayout *presetLayout =
        new QHBoxLayout();

    QLabel *presetLabel =
        new QLabel(
            QStringLiteral("Preset:"),
            this
            );

    m_presetCombo =
        new QComboBox(this);

    m_savePresetButton =
        new QPushButton(
            QStringLiteral("Salva set"),
            this
            );

    m_deletePresetButton =
        new QPushButton(
            QStringLiteral("Elimina set"),
            this
            );

    m_loadPresetButton =
        new QPushButton(
            QStringLiteral("Carica"),
            this
            );

    presetLayout->addWidget(
        presetLabel
        );

    presetLayout->addWidget(
        m_presetCombo,
        1
        );

    presetLayout->addWidget(
        m_savePresetButton
        );

    presetLayout->addWidget(
        m_deletePresetButton
        );

    presetLayout->addWidget(
        m_loadPresetButton
        );

    mainLayout->addLayout(
        presetLayout
        );


    // ==================================================
    // BUTTONS
    // ==================================================

    QHBoxLayout *buttonLayout =
        new QHBoxLayout();

    m_addButton =
        new QPushButton(
            QStringLiteral("Aggiungi Template"),
            this
            );

    m_removeButton =
        new QPushButton(
            QStringLiteral("Rimuovi"),
            this
            );

    m_cropButton =
        new QPushButton(
            QStringLiteral("Template Crop"),
            this
            );

    m_searchAreaButton =
        new QPushButton(
            QStringLiteral("Search Area"),
            this
            );

    QPushButton *searchArea2Button =
        new QPushButton(
            QStringLiteral("Configura Zona 2"),
            this
            );

    m_saveButton =
        new QPushButton(
            QStringLiteral("Salva"),
            this
            );


    buttonLayout->addWidget(
        m_addButton
        );

    buttonLayout->addWidget(
        m_removeButton
        );

    buttonLayout->addWidget(
        m_cropButton
        );

    buttonLayout->addWidget(
        m_searchAreaButton
        );

    buttonLayout->addWidget(
        searchArea2Button
        );

    buttonLayout->addStretch();

    buttonLayout->addWidget(
        m_saveButton
        );

    mainLayout->addLayout(
        buttonLayout
        );


    // ==================================================
    // CONNECTIONS
    // ==================================================

    connect(
        m_addButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::addTemplate
        );

    connect(
        m_removeButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::removeTemplate
        );

    connect(
        m_cropButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::cropTemplate
        );

    connect(
        m_saveButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::saveConfiguration
        );

    connect(
        m_templateList,
        &QListWidget::currentRowChanged,
        this,
        [this](int)
        {
            onTemplateSelectionChanged();
        }
        );

    connect(
        m_searchAreaButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::configureSearchArea
        );

    connect(
        searchArea2Button,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if (!m_manager)
                return;

            m_manager->configureSearchRegion2();
        }
        );

    connect(
        m_cooldownSpin,
        qOverload<int>(
            &QSpinBox::valueChanged
            ),
        this,
        [this](int)
        {
            updateSelectedTemplate();
        }
        );

    connect(
        m_searchZoneCombo,
        qOverload<int>(
            &QComboBox::currentIndexChanged
            ),
        this,
        [this](int)
        {
            updateSelectedTemplate();
        }
        );

    connect(
        m_templateList,
        &QListWidget::itemChanged,
        this,
        [this](QListWidgetItem *item)
        {
            const int index =
                m_templateList->row(item);

            if (index < 0 ||
                index >= m_templates.size())
            {
                return;
            }

            m_templates[index].searchEnabled =
                item->checkState() == Qt::Checked;
        }
        );

    connect(
        m_savePresetButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::saveCheckboxPreset
        );

    connect(
        m_deletePresetButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::deleteCheckboxPreset
        );

    connect(
        m_loadPresetButton,
        &QPushButton::clicked,
        this,
        &CustomSearcherConfigWindow::loadCheckboxPreset
        );
    connect(
        m_manager,
        &CustomSearcherManager::templateCropReady,
        this,
        [this](const QImage &image)
        {
            if(image.isNull())
                return;

            if(m_cropTemplateIndex < 0 ||
                m_cropTemplateIndex >= m_templates.size())
            {
                return;
            }

            // Il crop modifica SOLO il template di matching.
            // L'immagine overlay (displayImage) rimane invariata.
            m_templates[m_cropTemplateIndex].templateImage =
                image.convertToFormat(
                    QImage::Format_ARGB32
                    );

            const int index =
                m_cropTemplateIndex;

            m_cropTemplateIndex = -1;

            rebuildList();

            if(index >= 0 &&
                index < m_templateList->count())
            {
                m_templateList->setCurrentRow(index);
            }
        }
        );
    connect(
        m_manager,
        &CustomSearcherManager::templateCropCanceled,
        this,
        [this]()
        {
            m_cropTemplateIndex = -1;
        }
        );


    refreshCheckboxPresets();
}


// ======================================================
// REFRESH
// ======================================================

void CustomSearcherConfigWindow::refresh()
{
    m_templates.clear();

    if (!m_manager)
        return;

    const QVector<
        CustomSearcherManager::TemplateConfig
        > configs =
        m_manager->templates();

    m_templates.reserve(
        configs.size()
        );

    for (const auto &config :
         configs)
    {
        EditorTemplate templ;

        templ.id =
            config.id;

        // IMMAGINE OVERLAY
        templ.displayImage =
            config.displayImage;

        // IMMAGINE TEMPLATE/MATCHING
        templ.templateImage =
            config.templateImage;

        templ.cooldownMs =
            config.cooldownMs;

        templ.searchEnabled =
            config.searchEnabled;

        templ.searchZone =
            static_cast<int>(
                config.searchZone
                );

        m_templates.append(
            templ
            );
    }

    rebuildList();
}


// ======================================================
// REBUILD LIST
// ======================================================

void CustomSearcherConfigWindow::rebuildList()
{
    const int previousRow =
        m_templateList->currentRow();

    m_templateList->blockSignals(true);

    m_templateList->clear();

    for (const EditorTemplate &templ :
         m_templates)
    {
        const bool configured =
            !templ.templateImage.isNull();

        const QString status =
            configured
                ? QStringLiteral("✓ Configurato")
                : QStringLiteral("⚠ Da croppare");

        const QString zoneText =
            templ.searchZone == 1
                ? QStringLiteral("Zona 2")
                : QStringLiteral("Zona 1");

        const QString text =
            QStringLiteral(
                "Template %1 — %2 s — %3 — %4"
                )
                .arg(templ.id)
                .arg(templ.cooldownMs / 1000)
                .arg(zoneText)
                .arg(status);

        auto *item =
            new QListWidgetItem(
                QIcon(
                    QPixmap::fromImage(
                        templ.displayImage
                        )
                    ),
                text
                );

        item->setFlags(
            item->flags() |
            Qt::ItemIsUserCheckable
            );

        item->setCheckState(
            templ.searchEnabled
                ? Qt::Checked
                : Qt::Unchecked
            );

        m_templateList->addItem(
            item
            );
    }

    m_templateList->blockSignals(false);

    if (previousRow >= 0 &&
        previousRow < m_templateList->count())
    {
        m_templateList->setCurrentRow(
            previousRow
            );
    }
    else if (!m_templates.isEmpty())
    {
        m_templateList->setCurrentRow(0);
    }
}


// ======================================================
// SELECTED INDEX
// ======================================================

int CustomSearcherConfigWindow::selectedTemplateIndex() const
{
    const int row =
        m_templateList->currentRow();

    if (row < 0 ||
        row >= m_templates.size())
    {
        return -1;
    }

    return row;
}


// ======================================================
// LOAD SELECTED TEMPLATE
// ======================================================

void CustomSearcherConfigWindow::loadSelectedTemplate()
{
    const int index =
        selectedTemplateIndex();

    if (index < 0)
    {
        m_previewLabel->setPixmap(
            QPixmap()
            );

        m_previewLabel->setText(
            QStringLiteral(
                "Nessun template"
                )
            );

        return;
    }

    const EditorTemplate &templ =
        m_templates[index];


    // --------------------------------------------------
    // COOLDOWN
    // --------------------------------------------------

    m_cooldownSpin->blockSignals(true);

    m_cooldownSpin->setValue(
        templ.cooldownMs / 1000
        );

    m_cooldownSpin->blockSignals(false);


    // --------------------------------------------------
    // SEARCH ZONE
    // --------------------------------------------------

    m_searchZoneCombo->blockSignals(true);

    const int zoneIndex =
        m_searchZoneCombo->findData(
            templ.searchZone
            );

    if (zoneIndex >= 0)
    {
        m_searchZoneCombo->setCurrentIndex(
            zoneIndex
            );
    }
    else
    {
        m_searchZoneCombo->setCurrentIndex(0);
    }

    m_searchZoneCombo->blockSignals(false);


    // --------------------------------------------------
    // PREVIEW
    //
    // Mostriamo DISPLAY IMAGE:
    // questa è l'immagine associata all'overlay.
    //
    // NON usiamo templateImage qui.
    // --------------------------------------------------

    if (templ.displayImage.isNull())
    {
        m_previewLabel->setPixmap(
            QPixmap()
            );

        m_previewLabel->setText(
            QStringLiteral(
                "Nessuna immagine overlay"
                )
            );

        return;
    }

    QPixmap pixmap =
        QPixmap::fromImage(
            templ.displayImage
            );

    m_previewLabel->setPixmap(
        pixmap.scaled(
            180,
            180,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            )
        );

    m_previewLabel->setText(
        QString()
        );
}


// ======================================================
// UPDATE SELECTED TEMPLATE
// ======================================================

void CustomSearcherConfigWindow::updateSelectedTemplate()
{
    const int index =
        selectedTemplateIndex();

    if (index < 0)
        return;


    // --------------------------------------------------
    // COOLDOWN
    // --------------------------------------------------

    m_templates[index].cooldownMs =
        m_cooldownSpin->value() * 1000;


    // --------------------------------------------------
    // SEARCH ZONE
    // --------------------------------------------------

    const QVariant zoneData =
        m_searchZoneCombo->currentData();

    if (zoneData.isValid())
    {
        m_templates[index].searchZone =
            zoneData.toInt();
    }


    // --------------------------------------------------
    // LIST ITEM
    // --------------------------------------------------

    QListWidgetItem *item =
        m_templateList->item(index);

    if (!item)
        return;

    const bool configured =
        !m_templates[index].templateImage.isNull();

    const QString status =
        configured
            ? QStringLiteral("✓ Configurato")
            : QStringLiteral("⚠ Da croppare");

    const QString zoneText =
        m_templates[index].searchZone == 1
            ? QStringLiteral("Zona 2")
            : QStringLiteral("Zona 1");

    item->setText(
        QStringLiteral(
            "Template %1 — %2 s — %3 — %4"
            )
            .arg(
                m_templates[index].id
                )
            .arg(
                m_templates[index].cooldownMs / 1000
                )
            .arg(
                zoneText
                )
            .arg(
                status
                )
        );
}


// ======================================================
// ADD TEMPLATE
//
// IMPORTANTE:
// displayImage = immagine OVERLAY
// templateImage = vuota, verrà creata dal CROP
// ======================================================

void CustomSearcherConfigWindow::addTemplate()
{
    const QString fileName =
        QFileDialog::getOpenFileName(
            this,
            QStringLiteral(
                "Seleziona immagine display"
                ),
            QString(),
            QStringLiteral(
                "Immagini (*.png *.jpg *.jpeg *.bmp *.webp)"
                )
            );

    if (fileName.isEmpty())
        return;


    QImage displayImage;

    if (!displayImage.load(fileName))
    {
        QMessageBox::warning(
            this,
            QStringLiteral("Errore"),
            QStringLiteral(
                "Impossibile caricare l'immagine selezionata."
                )
            );

        return;
    }


    int newId = 0;

    for (const EditorTemplate &templ :
         m_templates)
    {
        newId =
            qMax(
                newId,
                templ.id + 1
                );
    }


    EditorTemplate templ;

    templ.id =
        newId;


    // --------------------------------------------------
    // OVERLAY IMAGE
    // --------------------------------------------------

    templ.displayImage =
        displayImage;


    // --------------------------------------------------
    // TEMPLATE IMAGE
    //
    // Resta vuota.
    // Verrà impostata da "Template Crop".
    // --------------------------------------------------

    templ.templateImage =
        QImage();


    templ.cooldownMs =
        0;

    templ.searchEnabled =
        true;


    // Zona 1 come default
    templ.searchZone =
        static_cast<int>(
            CustomSearcherManager::SearchZone::Zone1
            );


    m_templates.append(
        templ
        );


    rebuildList();

    m_templateList->setCurrentRow(
        m_templates.size() - 1
        );
}


// ======================================================
// REMOVE TEMPLATE
// ======================================================

void CustomSearcherConfigWindow::removeTemplate()
{
    const int index =
        selectedTemplateIndex();

    if (index < 0)
        return;


    m_templates.removeAt(
        index
        );

    rebuildList();


    if (!m_templates.isEmpty())
    {
        const int newIndex =
            qMin(
                index,
                m_templates.size() - 1
                );

        m_templateList->setCurrentRow(
            newIndex
            );
    }
}


// ======================================================
// CROP TEMPLATE
//
// Questo modifica SOLO templateImage.
// NON tocca displayImage.
// ======================================================


void CustomSearcherConfigWindow::cropTemplate()
{
    const int index = selectedTemplateIndex();

    if(index < 0)
        return;

    if(m_templates[index].displayImage.isNull())
        return;

    updateSelectedTemplate();

    m_cropTemplateIndex = index;

    const auto zone =
        m_templates[index].searchZone == 1
            ? CustomSearcherManager::SearchZone::Zone2
            : CustomSearcherManager::SearchZone::Zone1;

    m_manager->captureTemplateCrop(zone);
}


// ======================================================
// SAVE CONFIGURATION
// ======================================================

void CustomSearcherConfigWindow::saveConfiguration()
{
    updateSelectedTemplate();

    if (!m_manager)
        return;


    QVector<
        CustomSearcherManager::TemplateConfig
        > configs;

    configs.reserve(
        m_templates.size()
        );


    for (const EditorTemplate &templ :
         m_templates)
    {
        CustomSearcherManager::TemplateConfig config;


        config.id =
            templ.id;


        // --------------------------------------------------
        // OVERLAY
        // --------------------------------------------------

        config.displayImage =
            templ.displayImage;


        // --------------------------------------------------
        // TEMPLATE / MATCHING
        // --------------------------------------------------

        config.templateImage =
            templ.templateImage;


        config.cooldownMs =
            templ.cooldownMs;


        config.searchEnabled =
            templ.searchEnabled;


        config.searchZone =
            templ.searchZone == 1
                ? CustomSearcherManager::SearchZone::Zone2
                : CustomSearcherManager::SearchZone::Zone1;


        configs.append(
            config
            );
    }


    m_manager->setTemplates(
        configs
        );

    close();
}


// ======================================================
// SELECTION CHANGED
// ======================================================

void CustomSearcherConfigWindow::onTemplateSelectionChanged()
{
    loadSelectedTemplate();
}


// ======================================================
// SEARCH AREA ZONE 1
// ======================================================

void CustomSearcherConfigWindow::configureSearchArea()
{
    if (!m_manager)
        return;

    m_manager->configureSearchRegion();
}


// ======================================================
// SAVE CHECKBOX PRESET
// ======================================================

void CustomSearcherConfigWindow::saveCheckboxPreset()
{
    if (!m_presetCombo)
        return;


    const QString name =
        QInputDialog::getText(
            this,
            QStringLiteral("Salva preset"),
            QStringLiteral("Nome del preset:")
            ).trimmed();


    if (name.isEmpty())
        return;


    QSettings settings;

    settings.beginGroup(
        QStringLiteral(
            "CustomSearcher/CheckboxPresets"
            )
        );

    settings.beginGroup(
        name
        );

    settings.remove(
        QString()
        );


    for (const EditorTemplate &templ :
         m_templates)
    {
        settings.setValue(
            QString::number(
                templ.id
                ),
            templ.searchEnabled
            );
    }


    settings.endGroup();
    settings.endGroup();

    settings.sync();


    refreshCheckboxPresets();


    const int index =
        m_presetCombo->findText(
            name
            );

    if (index >= 0)
    {
        m_presetCombo->setCurrentIndex(
            index
            );
    }
}


// ======================================================
// LOAD CHECKBOX PRESET
// ======================================================

void CustomSearcherConfigWindow::loadCheckboxPreset()
{
    if (!m_presetCombo)
        return;


    const QString name =
        m_presetCombo->currentText();


    if (name.isEmpty() ||
        name == QStringLiteral("Seleziona preset..."))
    {
        return;
    }


    QSettings settings;

    settings.beginGroup(
        QStringLiteral(
            "CustomSearcher/CheckboxPresets"
            )
        );


    if (!settings.childGroups().contains(name))
    {
        settings.endGroup();
        return;
    }


    settings.beginGroup(
        name
        );


    /*
     * Il preset viene applicato per ID,
     * non per posizione nella lista.
     */

    for (int i = 0;
         i < m_templates.size();
         ++i)
    {
        const int id =
            m_templates[i].id;

        const QString key =
            QString::number(id);


        if (!settings.contains(key))
            continue;


        m_templates[i].searchEnabled =
            settings.value(
                        key,
                        true
                        ).toBool();
    }


    settings.endGroup();
    settings.endGroup();


    const int previousRow =
        m_templateList->currentRow();


    rebuildList();


    if (previousRow >= 0 &&
        previousRow < m_templateList->count())
    {
        m_templateList->setCurrentRow(
            previousRow
            );
    }
}


// ======================================================
// DELETE CHECKBOX PRESET
// ======================================================

void CustomSearcherConfigWindow::deleteCheckboxPreset()
{
    if (!m_presetCombo)
        return;


    const QString name =
        m_presetCombo->currentText();


    if (name.isEmpty() ||
        name == QStringLiteral("Seleziona preset..."))
    {
        return;
    }


    const QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            QStringLiteral("Elimina preset"),
            QStringLiteral(
                "Eliminare il preset \"%1\"?"
                ).arg(name)
            );


    if (result != QMessageBox::Yes)
        return;


    QSettings settings;

    settings.beginGroup(
        QStringLiteral(
            "CustomSearcher/CheckboxPresets"
            )
        );


    settings.remove(
        name
        );

    settings.endGroup();

    settings.sync();


    refreshCheckboxPresets();
}


// ======================================================
// REFRESH CHECKBOX PRESETS
// ======================================================

void CustomSearcherConfigWindow::refreshCheckboxPresets()
{
    if (!m_presetCombo)
        return;


    const QString currentName =
        m_presetCombo->currentText();


    QSettings settings;

    settings.beginGroup(
        QStringLiteral(
            "CustomSearcher/CheckboxPresets"
            )
        );


    QStringList presets =
        settings.childGroups();


    settings.endGroup();


    presets.sort(
        Qt::CaseInsensitive
        );


    m_presetCombo->blockSignals(true);

    m_presetCombo->clear();


    m_presetCombo->addItem(
        QStringLiteral(
            "Seleziona preset..."
            )
        );


    for (const QString &preset :
         presets)
    {
        m_presetCombo->addItem(
            preset
            );
    }


    const int index =
        m_presetCombo->findText(
            currentName
            );


    if (index >= 0)
    {
        m_presetCombo->setCurrentIndex(
            index
            );
    }
    else
    {
        m_presetCombo->setCurrentIndex(
            0
            );
    }


    m_presetCombo->blockSignals(false);
}
