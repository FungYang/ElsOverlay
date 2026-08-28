#include "mainwindow.h"

#define NOMINMAX
#include <windows.h>
#include <QDialog>
#include <QKeyEvent>
#include <QSettings>
#include <QCoreApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>
#include <QWidget>

namespace
{
QString vkCodeToKeyName(int vkCode)
{
    UINT scanCode = MapVirtualKeyW(static_cast<UINT>(vkCode), MAPVK_VK_TO_VSC);
    LONG lParam = static_cast<LONG>(scanCode << 16);

    switch(vkCode)
    {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_RCONTROL:
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
        lParam |= (1 << 24);
        break;
    default:
        break;
    }

    wchar_t buffer[64] = { 0 };

    if(GetKeyNameTextW(lParam, buffer, 64) > 0)
    {
        return QString::fromWCharArray(buffer);
    }

    return QString("0x%1").arg(vkCode, 0, 16);
}

class PauseKeyDialog : public QDialog
{
public:
    explicit PauseKeyDialog(int currentKey, QWidget *parent = nullptr)
        : QDialog(parent), m_key(currentKey)
    {
        setWindowTitle("Configura Tasto Pausa");
        setFixedSize(260, 120);

        QVBoxLayout *layout = new QVBoxLayout(this);

        m_infoLabel = new QLabel("Premi un tasto...", this);
        m_infoLabel->setAlignment(Qt::AlignCenter);

        m_saveButton = new QPushButton("Salva", this);
        m_saveButton->setEnabled(false);

        layout->addWidget(m_infoLabel);
        layout->addStretch();
        layout->addWidget(m_saveButton);

        connect(m_saveButton, &QPushButton::clicked, this, &QDialog::accept);
    }

    int key() const { return m_key; }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        int vk = event->nativeVirtualKey();

        if(vk != 0)
        {
            m_key = vk;
            m_infoLabel->setText("Tasto: " + vkCodeToKeyName(vk));
            m_saveButton->setEnabled(true);
        }
    }

private:
    int m_key;
    QLabel *m_infoLabel;
    QPushButton *m_saveButton;
};
}


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



    // =========================
    // TITOLO
    // =========================

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



    // =========================
    // ATMA
    // =========================

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



    // =========================
    // CLASS BUFF
    // =========================

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



    // =========================
    // DISTANCE GUIDES
    // =========================

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




    // =========================
    // BUFF TITLES
    // =========================

    QGroupBox *buffTitlesGroup =
        new QGroupBox(
            "Buff Titles",
            central
            );


    QHBoxLayout *buffTitlesLayout =
        new QHBoxLayout(
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


    buffTitlesLayout->addStretch();


    buffTitlesLayout->addWidget(
        buffTitlesToggleButton
        );


    mainLayout->addWidget(
        buffTitlesGroup
        );



    // =========================
    // BUFF TRASCENDENZA
    // =========================

    QGroupBox *buffTranscendenceGroup =
        new QGroupBox(
            "Buff Trascendenza",
            central
            );


    QHBoxLayout *buffTranscendenceLayout =
        new QHBoxLayout(
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


    buffTranscendenceLayout->addStretch();


    buffTranscendenceLayout->addWidget(
        buffTranscendenceToggleButton
        );


    mainLayout->addWidget(
        buffTranscendenceGroup
        );

    // =========================
    // BUFF TRACKER
    // =========================

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

    // =========================
    // TASTO PAUSA
    // =========================
    pauseKeyButton = new QPushButton(central);
    mainLayout->addWidget(pauseKeyButton);

    loadPauseKey();

    // =========================
    // SPAZIO
    // =========================

    mainLayout->addStretch();



    // =========================
    // CHIUDI
    // =========================

    closeButton =
        new QPushButton(
            "Chiudi",
            central
            );


    mainLayout->addWidget(
        closeButton
        );



    connect(
        pauseKeyButton,
        &QPushButton::clicked,
        this,
        &MainWindow::openPauseKeyDialog
        );
    // =========================
    // CONNECTIONS ATMA
    // =========================

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



    // =========================
    // CONNECTIONS CLASS BUFF
    // =========================

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



    // =========================
    // CONNECTIONS BUFF TITLES
    // =========================

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



    // =========================
    // CONNECTIONS BUFF TRASCENDENZA
    // =========================

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



    // =========================
    // CONNECTIONS BUFF TRACKER
    // =========================

    connect(
        buffTrackerConfigButton,
        &QPushButton::clicked,
        this,
        &MainWindow::buffTrackerConfigRequested
        );


    // =========================
    // CONNECTIONS DISTANCE
    // =========================

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





    // =========================
    // CHIUDI
    // =========================

    connect(
        closeButton,
        &QPushButton::clicked,
        this,
        &QMainWindow::close
        );
}



void MainWindow::setupToggleButton(
    QPushButton *button
    )
{
    if(!button)
        return;


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
        return;


    button->setText(
        enabled
            ? "ON"
            : "OFF"
        );
}
int MainWindow::pauseKey() const
{
    return m_pauseKey;
}

void MainWindow::loadPauseKey()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    m_pauseKey = settings.value("Keys/PauseKey", VK_SPACE).toInt();

    updatePauseKeyButtonText();
}

void MainWindow::savePauseKey(int vkCode)
{
    m_pauseKey = vkCode;

    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    settings.setValue("Keys/PauseKey", vkCode);

    updatePauseKeyButtonText();

    emit pauseKeyChanged(vkCode);
}

void MainWindow::updatePauseKeyButtonText()
{
    pauseKeyButton->setText("Pausa: " + vkCodeToKeyName(m_pauseKey));
}

void MainWindow::openPauseKeyDialog()
{
    PauseKeyDialog dialog(m_pauseKey, this);

    if(dialog.exec() == QDialog::Accepted)
    {
        savePauseKey(dialog.key());
    }
}