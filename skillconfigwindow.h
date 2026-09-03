#ifndef SKILLCONFIGWINDOW_H
#define SKILLCONFIGWINDOW_H

#include <QDialog>
#include <QString>

#include "skillconfig.h"

class QComboBox;
class QLineEdit;
class KeyEdit;
class QPushButton;
class QSpinBox;
class QListWidget;
class QWidget;


class SkillConfigWindow : public QDialog
{
    Q_OBJECT

public:

    explicit SkillConfigWindow(
        QWidget *parent = nullptr
        );

    SkillOverlayConfig config() const;


signals:

    void configurationChanged(
        const SkillOverlayConfig &config
        );


private:

    SkillOverlayConfig m_config;


    // --------------------------------------------------------
    // TASTI GENERALI
    // --------------------------------------------------------

    KeyEdit *stateKeyEdit;
    KeyEdit *cipollaKeyEdit;


    // --------------------------------------------------------
    // SKILL UP
    // --------------------------------------------------------

    QLineEdit *upNameEdit;
    QLineEdit *upImageEdit;
    QSpinBox *upCooldownSpin;
    QComboBox *upActivationCombo;
    QListWidget *upComboKeysList;
    QPushButton *upAddComboKeyButton;
    QPushButton *upRemoveComboKeyButton;


    // --------------------------------------------------------
    // SKILL LEFT
    // --------------------------------------------------------

    QLineEdit *leftNameEdit;
    QLineEdit *leftImageEdit;
    QSpinBox *leftCooldownSpin;
    QComboBox *leftActivationCombo;
    QListWidget *leftComboKeysList;
    QPushButton *leftAddComboKeyButton;
    QPushButton *leftRemoveComboKeyButton;


    // --------------------------------------------------------
    // SKILL DOWN
    // --------------------------------------------------------

    QLineEdit *downNameEdit;
    QLineEdit *downImageEdit;
    QSpinBox *downCooldownSpin;
    QComboBox *downActivationCombo;
    QListWidget *downComboKeysList;
    QPushButton *downAddComboKeyButton;
    QPushButton *downRemoveComboKeyButton;


    // --------------------------------------------------------
    // SKILL RIGHT
    // --------------------------------------------------------

    QLineEdit *rightNameEdit;
    QLineEdit *rightImageEdit;
    QSpinBox *rightCooldownSpin;
    QComboBox *rightActivationCombo;
    QListWidget *rightComboKeysList;
    QPushButton *rightAddComboKeyButton;
    QPushButton *rightRemoveComboKeyButton;


    // --------------------------------------------------------
    // SKILL ARTIFACT
    // --------------------------------------------------------

    QLineEdit *artifactNameEdit;
    QLineEdit *artifactImageEdit;
    QSpinBox *artifactCooldownSpin;


    // --------------------------------------------------------
    // BUTTONS
    // --------------------------------------------------------

    QPushButton *saveButton;
    QPushButton *cancelButton;


    // --------------------------------------------------------
    // UI
    // --------------------------------------------------------

    void buildUi();


    QWidget *createSkillWidget(
        const QString &title,

        QLineEdit *&nameEdit,
        QLineEdit *&imageEdit,
        QSpinBox *&cooldownSpin,
        QComboBox *&activationCombo,

        QListWidget *&comboKeysList,
        QPushButton *&addComboKeyButton,
        QPushButton *&removeComboKeyButton
        );


    QWidget *createArtifactWidget(
        const QString &title,

        QLineEdit *&nameEdit,
        QLineEdit *&imageEdit,
        QSpinBox *&cooldownSpin
        );


    // --------------------------------------------------------
    // CONFIG
    // --------------------------------------------------------

    void loadConfig();

    void saveConfig();


    void loadSkill(
        const SkillConfig &config,

        QLineEdit *nameEdit,
        QLineEdit *imageEdit,
        QSpinBox *cooldownSpin,
        QComboBox *activationCombo,

        QListWidget *comboKeysList
        );


    SkillConfig readSkill(
        QLineEdit *nameEdit,
        QLineEdit *imageEdit,
        QSpinBox *cooldownSpin,
        QComboBox *activationCombo,

        QListWidget *comboKeysList
        ) const;


    void loadArtifact(
        const SkillConfig &config
        );


    SkillConfig readArtifact() const;


    void readUiToConfig();


    // --------------------------------------------------------
    // COMBO
    // --------------------------------------------------------

    void addComboKey(
        QListWidget *list
        );


    void removeComboKey(
        QListWidget *list
        );


    void loadComboKeys(
        const QList<int> &keys,
        QListWidget *list
        );
};

#endif