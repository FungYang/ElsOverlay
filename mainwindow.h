#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>


class QPushButton;
class QSlider;
class QCheckBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(
        QWidget *parent = nullptr
        );

    ~MainWindow() override = default;


    // ==================================================
    // TOGGLE PERSISTENCE
    // ==================================================

    void loadToggleStates();
    void saveToggleStates();
    int transparencyValue() const;
    void setResonanceGateChecked(bool on);
    bool resonanceGatePauseCooldownEnabled() const
    {
        return m_resonanceGatePauseCooldown;
    }

    // Esito di "Porta tutto in primo piano" (da OverlayRoot):
    // ok = nessuno copre gli overlay; altrimenti detail dice cosa li copre.
    // Lo mostra per qualche secondo sul pulsante.
    void showBringToFrontResult(
        bool ok,
        const QString &detail
        );


signals:

    void customSearcherConfigRequested();
    void customSearcherToggled(bool enabled);

    void screenMirrorConfigRequested();
    void screenMirrorToggled(bool enabled);

    void atmaConfigRequested();
    void atmaZonesConfigRequested();
    void atmaToggled(
        bool enabled
        );


    void classBuffConfigRequested();
    void classBuffToggled(
        bool enabled
        );


    void distanceGuidesConfigRequested();
    void distanceGuidesToggled(
        bool enabled
        );




    void buffTitlesConfigRequested();
    void buffTitlesToggled(
        bool enabled
        );


    void buffTranscendenceToggled(
        bool enabled
        );


    void specialCooldownConfigRequested();
    void specialCooldownsToggled(
        bool enabled
        );


    void pauseKeyChanged(
        int scanCode,
        bool extended
        );

    void resetKeyChanged(
        int scanCode,
        bool extended
        );


    void transcendenceConfigRequested();


    void overlayClickabilityToggled(
        bool enabled
        );

    // Pulsante "Porta in primo piano": rimette tutti gli overlay
    // sopra le altre finestre.
    void bringOverlaysToFrontRequested();
    void resonanceGateConfigRequested();

    void resonanceGateToggled(
        bool enabled
        );
    void transparencyChanged(
        int value
        );
    void resonanceGatePauseCooldownChanged(bool enabled);




private:

    // Pulsante "Porta tutto in primo piano" e relativo esito.
    QPushButton *m_bringToFrontButton = nullptr;
    quint32 m_bringToFrontResultGeneration = 0;


    // ==================================================
    // ATMA
    // ==================================================

    QPushButton *atmaConfigButton;
    QPushButton *atmaZonesConfigButton;
    QPushButton *atmaToggleButton;


    // ==================================================
    // CLASS BUFF
    // ==================================================

    QPushButton *classBuffConfigButton;
    QPushButton *classBuffToggleButton;


    // ==================================================
    // DISTANCE GUIDES
    // ==================================================

    QPushButton *distanceGuidesConfigButton;
    QPushButton *distanceGuidesToggleButton;


    // ==================================================
    // BUFF TITLES
    // ==================================================

    QPushButton *buffTitlesConfigButton;
    QPushButton *buffTitlesToggleButton;


    // ==================================================
    // TRANSCENDENCE
    // ==================================================

    QPushButton *buffTranscendenceToggleButton;


    // ==================================================
    // SPECIAL COOLDOWNS
    // ==================================================

    QPushButton *specialCooldownConfigButton;
    QPushButton *specialCooldownToggleButton;


    // ==================================================
    // GENERAL
    // ==================================================

    QPushButton *closeButton;


    // ==================================================
    // PAUSE / RESET
    // ==================================================

    QPushButton *pauseKeyButton;
    QPushButton *resetKeyButton;


    // ==================================================
    // TRANSCENDENCE CONFIG
    // ==================================================

    QPushButton *transcendenceConfigButton;


    // ==================================================
    // OVERLAY CLICKABILITY
    // ==================================================

    QPushButton *overlayClickabilityToggleButton;
    // ==================================================
    // RESONANCE GATE
    // ==================================================

    QPushButton *resonanceGateConfigButton;
    QPushButton *resonanceGateToggleButton;

    // ==================================================
    // TRASPARENZA
    // ==================================================

    QSlider *transparencySlider;

    QPushButton *customSearcherConfigButton;
    QPushButton *customSearcherToggleButton;

    QPushButton *screenMirrorConfigButton;
    QPushButton *screenMirrorToggleButton;

    QCheckBox *resonanceGatePauseCooldownCheckBox = nullptr;
    bool m_resonanceGatePauseCooldown = true;



    // ==================================================
    // PAUSE KEY
    // ==================================================

    int m_pauseScanCode = 0x01;
    bool m_pauseExtended = false;


    // ==================================================
    // RESET KEY
    // ==================================================

    int m_resetScanCode = 0x1D;
    bool m_resetExtended = true;


    // ==================================================
    // TOGGLE UI
    // ==================================================

    void setupToggleButton(
        QPushButton *button
        );

    void updateToggleText(
        QPushButton *button,
        bool enabled
        );


    // ==================================================
    // PAUSE KEY
    // ==================================================

    void loadPauseKey();

    void savePauseKey(
        int scanCode,
        bool extended
        );

    void openPauseKeyDialog();

    void updatePauseKeyButtonText();


    // ==================================================
    // RESET KEY
    // ==================================================

    void loadResetKey();

    void saveResetKey(
        int scanCode,
        bool extended
        );

    void openResetKeyDialog();

    void updateResetKeyButtonText();


};

#endif
