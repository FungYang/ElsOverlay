#include "mainwindow.h"

#define NOMINMAX
#include <windows.h>

#include <QCoreApplication>
#include <QDialog>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

    namespace
{

    QString scanCodeToKeyName(
        int scanCode,
        bool extended
        )
    {
        LONG lParam =
            static_cast<LONG>(
                static_cast<UINT>(scanCode) << 16
                );

        if(extended)
        {
            lParam |= (1 << 24);
        }

        wchar_t buffer[64] = { 0 };

        if(GetKeyNameTextW(
                lParam,
                buffer,
                64
                ) > 0)
        {
            return QString::fromWCharArray(buffer);
        }

        return QString("ScanCode 0x%1")
            .arg(
                scanCode,
                2,
                16,
                QChar('0')
                )
            .toUpper();
    }


    // ============================================================
    // DIALOG CONFIGURAZIONE TASTO
    // ============================================================

    class KeyDialog : public QDialog
    {
    public:

        explicit KeyDialog(
            int currentScanCode,
            bool currentExtended,
            QWidget *parent = nullptr
            )
            : QDialog(parent),
            m_scanCode(currentScanCode),
            m_extended(currentExtended)
        {
            setWindowTitle(
                "Configura Tasto"
                );

            setFixedSize(
                260,
                120
                );


            QVBoxLayout *layout =
                new QVBoxLayout(this);


            m_infoLabel =
                new QLabel(
                    "Premi un tasto...",
                    this
                    );

            m_infoLabel->setAlignment(
                Qt::AlignCenter
                );


            m_saveButton =
                new QPushButton(
                    "Salva",
                    this
                    );

            m_saveButton->setEnabled(
                false
                );


            layout->addWidget(
                m_infoLabel
                );

            layout->addStretch();

            layout->addWidget(
                m_saveButton
                );


            connect(
                m_saveButton,
                &QPushButton::clicked,
                this,
                &QDialog::accept
                );
        }


        int scanCode() const
        {
            return m_scanCode;
        }


        bool extended() const
        {
            return m_extended;
        }


    protected:

        void keyPressEvent(
            QKeyEvent *event
            ) override
        {
            int scanCode =
                event->nativeScanCode();


            if(scanCode == 0)
            {
                return;
            }


            bool extended =
                false;


            /*
         * Qt/Windows può restituire lo scan code
         * extended con il prefisso 0xE000.
         *
         * Esempio:
         *
         * CTRL DESTRO
         *
         * nativeScanCode = 0xE01D
         *
         * diventa:
         *
         * scanCode = 0x1D
         * extended = true
         */

            if((scanCode & 0xE000) == 0xE000)
            {
                scanCode &= 0xFF;
                extended = true;
            }


            m_scanCode =
                scanCode;

            m_extended =
                extended;


            m_infoLabel->setText(
                "Tasto: " +
                scanCodeToKeyName(
                    m_scanCode,
                    m_extended
                    )
                );


            m_saveButton->setEnabled(
                true
                );


            // qDebug()
            //     << "TASTO CONFIG:"
            //     << "VK =" << event->nativeVirtualKey()
            //     << "ScanCode =" << Qt::hex
            //     << event->nativeScanCode()
            //     << "->" << m_scanCode
            //     << "Extended =" << m_extended;
        }


    private:

        int m_scanCode;
        bool m_extended;

        QLabel *m_infoLabel;
        QPushButton *m_saveButton;
    };

}


// ============================================================
// MAINWINDOW
// ============================================================

MainWindow::MainWindow(
    QWidget *parent
    )
    : QMainWindow(parent)
{
    setWindowTitle(
        "ElsOverlay"
        );

    setFixedSize(
        420,
        600
        );


    QWidget *central =
        new QWidget(this);

    setCentralWidget(
        central
        );


    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            central
            );


    mainLayout->setContentsMargins(
        20,
        20,
        20,
        20
        );

    mainLayout->setSpacing(
        10
        );


    // ========================================================
    // TITOLO
    // ========================================================

    QLabel *title =
        new QLabel(
            "ELS OVERLAY",
            central
            );


    QFont titleFont;

    titleFont.setPointSize(
        18
        );

    titleFont.setBold(
        true
        );


    title->setFont(
        titleFont
        );

    title->setAlignment(
        Qt::AlignCenter
        );


    mainLayout->addWidget(
        title
        );


    // ========================================================
    // ATMA
    // ========================================================

    QGroupBox *atmaGroup =
        new QGroupBox(
            "Atma",
            central
            );


    QHBoxLayout *atmaLayout =
        new QHBoxLayout(
            atmaGroup
            );


    atmaConfigButton =
        new QPushButton(
            "Configura",
            atmaGroup
            );


    atmaToggleButton =
        new QPushButton(
            "OFF",
            atmaGroup
            );


    setupToggleButton(
        atmaToggleButton
        );


    atmaLayout->addWidget(
        atmaConfigButton
        );

    atmaLayout->addStretch();

    atmaLayout->addWidget(
        atmaToggleButton
        );


    mainLayout->addWidget(
        atmaGroup
        );


    // ========================================================
    // CLASS BUFF
    // ========================================================

    QGroupBox *classBuffGroup =
        new QGroupBox(
            "Class Buff",
            central
            );


    QHBoxLayout *classBuffLayout =
        new QHBoxLayout(
            classBuffGroup
            );


    classBuffConfigButton =
        new QPushButton(
            "Configura",
            classBuffGroup
            );


    classBuffToggleButton =
        new QPushButton(
            "OFF",
            classBuffGroup
            );


    setupToggleButton(
        classBuffToggleButton
        );


    classBuffLayout->addWidget(
        classBuffConfigButton
        );

    classBuffLayout->addStretch();

    classBuffLayout->addWidget(
        classBuffToggleButton
        );


    mainLayout->addWidget(
        classBuffGroup
        );


    // ========================================================
    // DISTANCE GUIDES
    // ========================================================

    QGroupBox *distanceGroup =
        new QGroupBox(
            "Distance Guides",
            central
            );


    QHBoxLayout *distanceLayout =
        new QHBoxLayout(
            distanceGroup
            );


    distanceGuidesConfigButton =
        new QPushButton(
            "Configura",
            distanceGroup
            );


    distanceGuidesToggleButton =
        new QPushButton(
            "OFF",
            distanceGroup
            );


    setupToggleButton(
        distanceGuidesToggleButton
        );


    distanceLayout->addWidget(
        distanceGuidesConfigButton
        );

    distanceLayout->addStretch();

    distanceLayout->addWidget(
        distanceGuidesToggleButton
        );


    mainLayout->addWidget(
        distanceGroup
        );


    // ========================================================
    // BUFF TITLES
    // ========================================================

    QGroupBox *buffTitlesGroup =
        new QGroupBox(
            "Buff Titles",
            central
            );


    QHBoxLayout *buffTitlesLayout =
        new QHBoxLayout(
            buffTitlesGroup
            );


    buffTitlesConfigButton =
        new QPushButton(
            "Configura",
            buffTitlesGroup
            );


    buffTitlesToggleButton =
        new QPushButton(
            "OFF",
            buffTitlesGroup
            );


    setupToggleButton(
        buffTitlesToggleButton
        );


    buffTitlesLayout->addWidget(
        buffTitlesConfigButton
        );


    buffTitlesLayout->addStretch();


    buffTitlesLayout->addWidget(
        buffTitlesToggleButton
        );


    mainLayout->addWidget(
        buffTitlesGroup
        );


    QGroupBox *buffTranscendenceGroup =
        new QGroupBox(
            "Buff Trascendenza",
            central
            );


    QHBoxLayout *buffTranscendenceLayout =
        new QHBoxLayout(
            buffTranscendenceGroup
            );


    transcendenceConfigButton =
        new QPushButton(
            "Configura",
            buffTranscendenceGroup
            );


    buffTranscendenceToggleButton =
        new QPushButton(
            "OFF",
            buffTranscendenceGroup
            );


    setupToggleButton(
        buffTranscendenceToggleButton
        );


    buffTranscendenceLayout->addWidget(
        transcendenceConfigButton
        );

    buffTranscendenceLayout->addStretch();

    buffTranscendenceLayout->addWidget(
        buffTranscendenceToggleButton
        );


    mainLayout->addWidget(
        buffTranscendenceGroup
        );


    // ========================================================
    // BUFF TRACKER
    // ========================================================

    QGroupBox *trackerGroup =
        new QGroupBox(
            "Buff Tracker",
            central
            );


    QHBoxLayout *trackerLayout =
        new QHBoxLayout(
            trackerGroup
            );


    buffTrackerConfigButton =
        new QPushButton(
            "Configura",
            trackerGroup
            );


    trackerLayout->addWidget(
        buffTrackerConfigButton
        );


    mainLayout->addWidget(
        trackerGroup
        );


    // ========================================================
    // TASTO PAUSA
    // ========================================================

    pauseKeyButton =
        new QPushButton(
            central
            );


    mainLayout->addWidget(
        pauseKeyButton
        );


    // ========================================================
    // TASTO RESET GLOBALE
    // ========================================================

    resetKeyButton =
        new QPushButton(
            central
            );


    mainLayout->addWidget(
        resetKeyButton
        );


    // ========================================================
    // CARICAMENTO TASTI
    // ========================================================

    loadPauseKey();
    loadResetKey();


    // ========================================================
    // SPAZIO
    // ========================================================

    mainLayout->addStretch();


    // ========================================================
    // CHIUDI
    // ========================================================

    closeButton =
        new QPushButton(
            "Chiudi",
            central
            );


    mainLayout->addWidget(
        closeButton
        );


    // ========================================================
    // CONNECTIONS TASTI
    // ========================================================

    connect(
        pauseKeyButton,
        &QPushButton::clicked,
        this,
        &MainWindow::openPauseKeyDialog
        );


    connect(
        resetKeyButton,
        &QPushButton::clicked,
        this,
        &MainWindow::openResetKeyDialog
        );


    // ========================================================
    // ATMA
    // ========================================================

    connect(
        atmaConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::atmaConfigRequested
        );


    connect(
        atmaToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                atmaToggleButton,
                enabled
                );

            emit atmaToggled(
                enabled
                );
        }
        );


    // ========================================================
    // CLASS BUFF
    // ========================================================

    connect(
        classBuffConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::classBuffConfigRequested
        );


    connect(
        classBuffToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                classBuffToggleButton,
                enabled
                );

            emit classBuffToggled(
                enabled
                );
        }
        );


    // ========================================================
    // BUFF TITLES
    // ========================================================

    connect(
        buffTitlesConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::buffTitlesConfigRequested
        );

    connect(
        buffTitlesToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                buffTitlesToggleButton,
                enabled
                );

            emit buffTitlesToggled(
                enabled
                );
        }
        );


    // ========================================================
    // BUFF TRASCENDENZA
    // ========================================================

    connect(
        buffTranscendenceToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                buffTranscendenceToggleButton,
                enabled
                );

            emit buffTranscendenceToggled(
                enabled
                );
        }
        );
    connect(
        transcendenceConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::transcendenceConfigRequested
        );


    // ========================================================
    // BUFF TRACKER
    // ========================================================

    connect(
        buffTrackerConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::buffTrackerConfigRequested
        );


    // ========================================================
    // DISTANCE GUIDES
    // ========================================================

    connect(
        distanceGuidesConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::distanceGuidesConfigRequested
        );


    connect(
        distanceGuidesToggleButton,
        &QPushButton::toggled,
        this,
        [this](bool enabled)
        {
            updateToggleText(
                distanceGuidesToggleButton,
                enabled
                );

            emit distanceGuidesToggled(
                enabled
                );
        }
        );


    // ========================================================
    // CHIUDI
    // ========================================================

    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &QMainWindow::close
        );
}


// ============================================================
// TOGGLE
// ============================================================

void MainWindow::setupToggleButton(
    QPushButton *button
    )
{
    if(!button)
    {
        return;
    }


    button->setCheckable(
        true
        );

    button->setChecked(
        false
        );

    button->setMinimumWidth(
        70
        );


    updateToggleText(
        button,
        false
        );
}


void MainWindow::updateToggleText(
    QPushButton *button,
    bool enabled
    )
{
    if(!button)
    {
        return;
    }


    button->setText(
        enabled
            ? "ON"
            : "OFF"
        );
}


// ============================================================
// PAUSA - LOAD
// ============================================================

void MainWindow::loadPauseKey()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    m_pauseScanCode =
        settings.value(
                    "Keys/PauseScanCode",
                    0x01
                    ).toInt();


    m_pauseExtended =
        settings.value(
                    "Keys/PauseExtended",
                    false
                    ).toBool();


    updatePauseKeyButtonText();
}


// ============================================================
// PAUSA - SAVE
// ============================================================

void MainWindow::savePauseKey(
    int scanCode,
    bool extended
    )
{
    m_pauseScanCode =
        scanCode;

    m_pauseExtended =
        extended;


    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    settings.setValue(
        "Keys/PauseScanCode",
        scanCode
        );


    settings.setValue(
        "Keys/PauseExtended",
        extended
        );


    settings.sync();


    updatePauseKeyButtonText();


    emit pauseKeyChanged(
        scanCode,
        extended
        );
}


// ============================================================
// PAUSA - LABEL
// ============================================================

void MainWindow::updatePauseKeyButtonText()
{
    pauseKeyButton->setText(
        "Pausa: " +
        scanCodeToKeyName(
            m_pauseScanCode,
            m_pauseExtended
            )
        );
}


// ============================================================
// PAUSA - DIALOG
// ============================================================

void MainWindow::openPauseKeyDialog()
{
    KeyDialog dialog(
        m_pauseScanCode,
        m_pauseExtended,
        this
        );


    if(dialog.exec() == QDialog::Accepted)
    {
        savePauseKey(
            dialog.scanCode(),
            dialog.extended()
            );
    }
}


// ============================================================
// RESET - LOAD
// ============================================================

void MainWindow::loadResetKey()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    m_resetScanCode =
        settings.value(
                    "Keys/ResetScanCode",
                    0x1D
                    ).toInt();


    m_resetExtended =
        settings.value(
                    "Keys/ResetExtended",
                    true
                    ).toBool();


    updateResetKeyButtonText();
}


// ============================================================
// RESET - SAVE
// ============================================================

void MainWindow::saveResetKey(
    int scanCode,
    bool extended
    )
{
    m_resetScanCode =
        scanCode;

    m_resetExtended =
        extended;


    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    settings.setValue(
        "Keys/ResetScanCode",
        scanCode
        );


    settings.setValue(
        "Keys/ResetExtended",
        extended
        );


    settings.sync();


    updateResetKeyButtonText();


    // qDebug()
    //     << "RESET KEY SALVATO:"
    //     << "ScanCode =" << Qt::hex << scanCode
    //     << "Extended =" << extended;


    emit resetKeyChanged(
        scanCode,
        extended
        );
}


// ============================================================
// RESET - LABEL
// ============================================================

void MainWindow::updateResetKeyButtonText()
{
    resetKeyButton->setText(
        "Reset globale: " +
        scanCodeToKeyName(
            m_resetScanCode,
            m_resetExtended
            )
        );
}


// ============================================================
// RESET - DIALOG
// ============================================================

void MainWindow::openResetKeyDialog()
{
    KeyDialog dialog(
        m_resetScanCode,
        m_resetExtended,
        this
        );


    if(dialog.exec() == QDialog::Accepted)
    {
        saveResetKey(
            dialog.scanCode(),
            dialog.extended()
            );
    }
}
