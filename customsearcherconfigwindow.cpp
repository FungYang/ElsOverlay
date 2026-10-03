#include "customsearcherconfigwindow.h"

#include "customsearchermanager.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>


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
        500
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

    mainLayout->addLayout(
        presetLayout
        );

    m_loadPresetButton =
        new QPushButton(
            QStringLiteral("Carica"),
            this
            );

    presetLayout->addWidget(
        m_loadPresetButton
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

    m_saveButton =
        new QPushButton(
            QStringLiteral("Salva"),
            this
            );

    m_searchAreaButton =
        new QPushButton(
            tr("Search Area"),
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

    refreshCheckboxPresets();
}


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

        templ.displayImage =
            config.displayImage;

        templ.templateImage =
            config.templateImage;

        templ.cooldownMs =
            config.cooldownMs;

        templ.searchEnabled =
            config.searchEnabled;

        m_templates.append(
            templ
            );
    }

    rebuildList();
}


void CustomSearcherConfigWindow::rebuildList()
{
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

        const QString text =
            QStringLiteral(
                "Template %1 — %2 s — %3"
                )
                .arg(templ.id)
                .arg(templ.cooldownMs / 1000)
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

        m_templateList->addItem(item);
    }
}


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

    m_cooldownSpin->blockSignals(true);

    m_cooldownSpin->setValue(
        templ.cooldownMs / 1000
        );

    m_cooldownSpin->blockSignals(false);


    if (templ.displayImage.isNull())
    {
        m_previewLabel->setPixmap(
            QPixmap()
            );

        m_previewLabel->setText(
            QStringLiteral(
                "Nessuna immagine"
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


void CustomSearcherConfigWindow::updateSelectedTemplate()
{
    const int index =
        selectedTemplateIndex();

    if (index < 0)
        return;

    m_templates[index].cooldownMs =
        m_cooldownSpin->value() * 1000;

    QListWidgetItem *item =
        m_templateList->item(index);

    if (!item)
        return;

    item->setText(
        QStringLiteral(
            "Template %1 — %2 s"
            )
            .arg(
                m_templates[index].id
                )
            .arg(
                m_templates[index].cooldownMs / 1000
                )
        );
}


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
        return;


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

    templ.displayImage =
        displayImage;

    templ.templateImage =
        QImage();

    templ.cooldownMs =
        0;

    templ.searchEnabled = true;

    m_templates.append(
        templ
        );

    rebuildList();

    m_templateList->setCurrentRow(
        m_templates.size() - 1
        );
}


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


void CustomSearcherConfigWindow::cropTemplate()
{
    const int index =
        selectedTemplateIndex();

    if (index < 0 ||
        index >= m_templates.size())
    {
        return;
    }

    if (!m_manager)
        return;

    updateSelectedTemplate();

    m_cropTemplateIndex = index;

    connect(
        m_manager,
        &CustomSearcherManager::templateCropReady,
        this,
        [this](const QImage &image)
        {
            const int index =
                m_cropTemplateIndex;

            m_cropTemplateIndex = -1;

            if (index < 0 ||
                index >= m_templates.size())
            {
                return;
            }

            m_templates[index].templateImage =
                image.convertToFormat(
                    QImage::Format_ARGB32
                    );

            rebuildList();

            m_templateList->setCurrentRow(
                index
                );

            show();
            raise();
            activateWindow();
            setFocus();
        },
        Qt::SingleShotConnection
        );

    connect(
        m_manager,
        &CustomSearcherManager::templateCropCanceled,
        this,
        [this]()
        {
            m_cropTemplateIndex = -1;

            show();
            raise();
            activateWindow();
            setFocus();
        },
        Qt::SingleShotConnection
        );

    m_manager->captureTemplateCrop();
}


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

        config.displayImage =
            templ.displayImage;

        config.templateImage =
            templ.templateImage;

        config.cooldownMs =
            templ.cooldownMs;

        config.searchEnabled =
            templ.searchEnabled;

        configs.append(
            config
            );
    }

    m_manager->setTemplates(
        configs
        );

    close();
}


void CustomSearcherConfigWindow::onTemplateSelectionChanged()
{
    loadSelectedTemplate();
}


void CustomSearcherConfigWindow::configureSearchArea()
{
    if (!m_manager)
        return;

    m_manager->configureSearchRegion();
}
void CustomSearcherConfigWindow::saveCheckboxPreset()
{
    if(!m_presetCombo)
        return;

    const QString name =
        QInputDialog::getText(
            this,
            QStringLiteral("Salva preset"),
            QStringLiteral("Nome del preset:")
            ).trimmed();

    if(name.isEmpty())
        return;

    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher/CheckboxPresets")
        );

    settings.beginGroup(name);

    settings.remove(
        QString()
        );

    for(const EditorTemplate &templ : m_templates)
    {
        settings.setValue(
            QString::number(templ.id),
            templ.searchEnabled
            );
    }

    settings.endGroup();
    settings.endGroup();

    settings.sync();

    refreshCheckboxPresets();

    const int index =
        m_presetCombo->findText(name);

    if(index >= 0)
    {
        m_presetCombo->setCurrentIndex(
            index
            );
    }
}

void CustomSearcherConfigWindow::loadCheckboxPreset()
{
    if(!m_presetCombo)
        return;

    const QString name =
        m_presetCombo->currentText();

    if(name.isEmpty())
        return;

    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher/CheckboxPresets")
        );

    if(!settings.childGroups().contains(name))
    {
        settings.endGroup();
        return;
    }

    settings.beginGroup(name);

    /*
     * Il preset viene applicato per ID,
     * non per posizione nella lista.
     */
    for(int i = 0;
         i < m_templates.size();
         ++i)
    {
        const int id =
            m_templates[i].id;

        const QString key =
            QString::number(id);

        if(!settings.contains(key))
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

    if(previousRow >= 0 &&
        previousRow < m_templateList->count())
    {
        m_templateList->setCurrentRow(
            previousRow
            );
    }
}

void CustomSearcherConfigWindow::deleteCheckboxPreset()
{
    if(!m_presetCombo)
        return;

    const QString name =
        m_presetCombo->currentText();

    if(name.isEmpty())
        return;

    const QMessageBox::StandardButton result =
        QMessageBox::question(
            this,
            QStringLiteral("Elimina preset"),
            QStringLiteral(
                "Eliminare il preset \"%1\"?"
                ).arg(name)
            );

    if(result != QMessageBox::Yes)
        return;

    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher/CheckboxPresets")
        );

    settings.remove(
        name
        );

    settings.endGroup();

    settings.sync();

    refreshCheckboxPresets();
}

void CustomSearcherConfigWindow::refreshCheckboxPresets()
{
    if(!m_presetCombo)
        return;

    const QString currentName =
        m_presetCombo->currentText();

    QSettings settings;

    settings.beginGroup(
        QStringLiteral("CustomSearcher/CheckboxPresets")
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
        QStringLiteral("Seleziona preset...")
        );

    for(const QString &preset : presets)
    {
        m_presetCombo->addItem(
            preset
            );
    }

    const int index =
        m_presetCombo->findText(
            currentName
            );

    if(index >= 0)
    {
        m_presetCombo->setCurrentIndex(
            index
            );
    }
    else
    {
        m_presetCombo->setCurrentIndex(0);
    }

    m_presetCombo->blockSignals(false);
}