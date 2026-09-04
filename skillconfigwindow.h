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

    KeyEdit *stateKeyEdit = nullptr;

    KeyEdit *cipollaKeyEdit = nullptr;


    // --------------------------------------------------------
    // SCALA
    // --------------------------------------------------------

    QComboBox *scaleCombo = nullptr;


    // --------------------------------------------------------
    // SKILL UP
    // --------------------------------------------------------

    QLineEdit *upNameEdit = nullptr;

    QLineEdit *upImageEdit = nullptr;

    QSpinBox *upCooldownSpin = nullptr;

    QComboBox *upActivationCombo = nullptr;

    QListWidget *upComboKeysList = nullptr;

    QPushButton *upAddComboKeyButton = nullptr;

    QPushButton *upRemoveComboKeyButton = nullptr;


    // --------------------------------------------------------
    // SKILL LEFT
    // --------------------------------------------------------

    QLineEdit *leftNameEdit = nullptr;

    QLineEdit *leftImageEdit = nullptr;

    QSpinBox *leftCooldownSpin = nullptr;

    QComboBox *leftActivationCombo = nullptr;

    QListWidget *leftComboKeysList = nullptr;

    QPushButton *leftAddComboKeyButton = nullptr;

    QPushButton *leftRemoveComboKeyButton = nullptr;


    // --------------------------------------------------------
    // SKILL DOWN
    // --------------------------------------------------------

    QLineEdit *downNameEdit = nullptr;

    QLineEdit *downImageEdit = nullptr;

    QSpinBox *downCooldownSpin = nullptr;

    QComboBox *downActivationCombo = nullptr;

    QListWidget *downComboKeysList = nullptr;

    QPushButton *downAddComboKeyButton = nullptr;

    QPushButton *downRemoveComboKeyButton = nullptr;


    // --------------------------------------------------------
    // SKILL RIGHT
    // --------------------------------------------------------

    QLineEdit *rightNameEdit = nullptr;

    QLineEdit *rightImageEdit = nullptr;

    QSpinBox *rightCooldownSpin = nullptr;

    QComboBox *rightActivationCombo = nullptr;

    QListWidget *rightComboKeysList = nullptr;

    QPushButton *rightAddComboKeyButton = nullptr;

    QPushButton *rightRemoveComboKeyButton = nullptr;


    // --------------------------------------------------------
    // SKILL ARTIFACT
    // --------------------------------------------------------

    QLineEdit *artifactNameEdit = nullptr;

    QLineEdit *artifactImageEdit = nullptr;

    QSpinBox *artifactCooldownSpin = nullptr;


    // --------------------------------------------------------
    // BUTTONS
    // --------------------------------------------------------

    QPushButton *saveButton = nullptr;

    QPushButton *cancelButton = nullptr;


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
