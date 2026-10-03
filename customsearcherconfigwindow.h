#pragma once

#include <QDialog>
#include <QImage>
#include <QRect>
#include <QVector>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSpinBox;
class QLabel;
class QComboBox;

class CustomSearcherManager;

class CustomSearcherConfigWindow : public QDialog
{
    Q_OBJECT

public:

    explicit CustomSearcherConfigWindow(
        CustomSearcherManager *manager,
        QWidget *parent = nullptr
        );

    ~CustomSearcherConfigWindow() override = default;

    void refresh();
    void saveCheckboxPreset();
    void loadCheckboxPreset();
    void deleteCheckboxPreset();
    void refreshCheckboxPresets();

private slots:

    void addTemplate();
    void removeTemplate();
    void cropTemplate();
    void saveConfiguration();

    void onTemplateSelectionChanged();
    void configureSearchArea();

private:

    struct EditorTemplate
    {
        int id;
        QImage displayImage;
        QImage templateImage;
        int cooldownMs;
        bool searchEnabled = true;
    };

    void rebuildList();
    void loadSelectedTemplate();
    void updateSelectedTemplate();

    int selectedTemplateIndex() const;

private:

    int m_cropTemplateIndex = -1;
    CustomSearcherManager *m_manager = nullptr;

    QVector<EditorTemplate> m_templates;

    QListWidget *m_templateList = nullptr;

    QLabel *m_previewLabel = nullptr;

    QSpinBox *m_cooldownSpin = nullptr;

    QPushButton *m_addButton = nullptr;
    QPushButton *m_removeButton = nullptr;
    QPushButton *m_cropButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_searchAreaButton = nullptr;
    QComboBox *m_presetCombo = nullptr;

    QPushButton *m_savePresetButton = nullptr;
    QPushButton *m_deletePresetButton = nullptr;
    QPushButton *m_loadPresetButton = nullptr;
};