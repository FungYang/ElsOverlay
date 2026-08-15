#include "distanceguideconfigwindow.h"
#include "distanceguideline.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QtGlobal>
#include <QDebug>


DistanceGuideConfigWindow::DistanceGuideConfigWindow(
    DistanceGuideManager *manager,
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    m_manager(manager),
    m_keyboard(keyboard)
{
    setWindowTitle(
        "Distance Guides"
        );


    setFixedSize(
        650,
        600
        );


    createUi();


    connect(
        this,
        &DistanceGuideConfigWindow::configureRequested,
        this,
        [this](const QString &guideId)
        {
            if(!m_manager)
                return;


            const QList<DistanceGuideConfiguration>
                guides =
                m_manager->guides();


            for(const DistanceGuideConfiguration &guide :
                 guides)
            {
                if(guide.id != guideId)
                    continue;


                m_configurationGuideId =
                    guideId;


                if(m_configurationLine)
                {
                    delete m_configurationLine;

                    m_configurationLine =
                        nullptr;
                }


                m_configurationLine =
                    new DistanceGuideLine(
                        guide.color
                        );


                const int positionX =
                    m_manager->effectivePositionX(
                        guide
                        );


                m_configurationLine->move(
                    positionX,
                    0
                    );


                m_configurationLine->resize(
                    5,
                    1080
                    );


                m_configurationLine->setConfigurationMode(
                    true
                    );


                m_configurationLine->show();
                m_configurationLine->raise();


                break;
            }
        }
        );


    connect(
        m_keyboard,
        &GlobalKeyboard::confirmPressed,
        this,
        [this]()
        {
            if(m_characterCenterLine)
            {
                confirmCharacterCenter();
                return;
            }


            confirmPositions();
        }
        );


    refresh();
}



void DistanceGuideConfigWindow::createUi()
{
    QVBoxLayout *mainLayout =
        new QVBoxLayout(
            this
            );


    mainLayout->setContentsMargins(
        15,
        15,
        15,
        15
        );


    mainLayout->setSpacing(
        10
        );


    QLabel *title =
        new QLabel(
            "DISTANCE GUIDES",
            this
            );


    QFont font =
        title->font();


    font.setBold(
        true
        );


    font.setPointSize(
        16
        );


    title->setFont(
        font
        );


    title->setAlignment(
        Qt::AlignCenter
        );


    mainLayout->addWidget(
        title
        );


    m_list =
        new QListWidget(
            this
            );


    m_list->setSpacing(
        5
        );


    mainLayout->addWidget(
        m_list
        );


    m_addButton =
        new QPushButton(
            "Add Distance Guide",
            this
            );


    m_addButton->setMinimumHeight(
        40
        );


    mainLayout->addWidget(
        m_addButton
        );


    m_closeButton =
        new QPushButton(
            "Chiudi",
            this
            );


    mainLayout->addWidget(
        m_closeButton
        );


    connect(
        m_addButton,
        &QPushButton::clicked,
        this,
        &DistanceGuideConfigWindow::addGuide
        );


    connect(
        m_closeButton,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );
}



void DistanceGuideConfigWindow::refresh()
{
    populateList();
}



void DistanceGuideConfigWindow::populateList()
{
    m_list->clear();


    if(!m_manager)
        return;


    const QList<DistanceGuideConfiguration>
        guides =
        m_manager->guides();

    // =========================
    // CHARACTER CENTER
    // =========================

    QListWidgetItem *centerItem =
        new QListWidgetItem(
            m_list
            );


    QWidget *centerRow =
        new QWidget(
            m_list
            );


    QHBoxLayout *centerLayout =
        new QHBoxLayout(
            centerRow
            );


    centerLayout->setContentsMargins(
        5,
        5,
        5,
        5
        );


    QLabel *centerLabel =
        new QLabel(
            "CENTER",
            centerRow
            );


    centerLabel->setMinimumWidth(
        150
        );


    QFont centerFont =
        centerLabel->font();


    centerFont.setBold(
        true
        );


    centerLabel->setFont(
        centerFont
        );


    centerLayout->addWidget(
        centerLabel
        );


    centerLayout->addStretch();


    QPushButton *fixCharacterButton =
        new QPushButton(
            "FIX CHARACTER",
            centerRow
            );


    centerLayout->addWidget(
        fixCharacterButton
        );


    centerItem->setSizeHint(
        centerRow->sizeHint()
        );


    m_list->setItemWidget(
        centerItem,
        centerRow
        );


    connect(
        fixCharacterButton,
        &QPushButton::clicked,
        this,
        &DistanceGuideConfigWindow::fixCharacterCenter
        );


    // =========================
    // DISTANCE GUIDES
    // =========================


    for(const DistanceGuideConfiguration &guide :
         guides)
    {
        QListWidgetItem *item =
            new QListWidgetItem(
                m_list
                );


        QWidget *row =
            new QWidget(
                m_list
                );


        QHBoxLayout *layout =
            new QHBoxLayout(
                row
                );


        layout->setContentsMargins(
            5,
            5,
            5,
            5
            );


        // =========================
        // NOME
        // =========================

        QLineEdit *nameEdit =
            new QLineEdit(
                guide.name,
                row
                );


        nameEdit->setMinimumWidth(
            150
            );


        layout->addWidget(
            nameEdit
            );


        connect(
            nameEdit,
            &QLineEdit::editingFinished,
            this,
            [this,
             guideId = guide.id,
             nameEdit]()
            {
                if(!m_manager)
                    return;


                DistanceGuideConfiguration updated;


                for(const auto &guide :
                     m_manager->guides())
                {
                    if(guide.id == guideId)
                    {
                        updated =
                            guide;

                        break;
                    }
                }


                updated.name =
                    nameEdit->text().trimmed();


                if(updated.name.isEmpty())
                    return;


                m_manager->updateGuide(
                    updated
                    );
            }
            );


        // =========================
        // COLORE
        // =========================

        QPushButton *colorButton =
            new QPushButton(
                row
                );


        colorButton->setFixedSize(
            32,
            32
            );


        colorButton->setStyleSheet(
            QString(
                "background-color: %1;"
                ).arg(
                    guide.color.name()
                    )
            );


        layout->addWidget(
            colorButton
            );


        connect(
            colorButton,
            &QPushButton::clicked,
            this,
            [this,
             guideId = guide.id,
             colorButton]()
            {
                if(!m_manager)
                    return;


                QColor currentColor;


                for(const auto &guide :
                     m_manager->guides())
                {
                    if(guide.id == guideId)
                    {
                        currentColor =
                            guide.color;

                        break;
                    }
                }


                const QColor color =
                    QColorDialog::getColor(
                        currentColor,
                        this,
                        "Choose Guide Color"
                        );


                if(!color.isValid())
                    return;


                DistanceGuideConfiguration updated;


                for(const auto &guide :
                     m_manager->guides())
                {
                    if(guide.id == guideId)
                    {
                        updated =
                            guide;

                        break;
                    }
                }


                updated.color =
                    color;


                m_manager->updateGuide(
                    updated
                    );


                colorButton->setStyleSheet(
                    QString(
                        "background-color: %1;"
                        ).arg(
                            color.name()
                            )
                    );
            }
            );


        // =========================
        // TOGGLE
        // =========================

        QPushButton *enabledButton =
            new QPushButton(
                guide.enabled
                    ? "ON"
                    : "OFF",
                row
                );


        enabledButton->setCheckable(
            true
            );


        enabledButton->setChecked(
            guide.enabled
            );


        layout->addWidget(
            enabledButton
            );


        connect(
            enabledButton,
            &QPushButton::toggled,
            this,
            [this,
             guideId = guide.id,
             enabledButton](bool enabled)
            {
                if(!m_manager)
                    return;


                m_manager->setGuideEnabled(
                    guideId,
                    enabled
                    );


                enabledButton->setText(
                    enabled
                        ? "ON"
                        : "OFF"
                    );
            }
            );


        // =========================
        // CONFIGURA
        // =========================

        QPushButton *configureButton =
            new QPushButton(
                "Configura",
                row
                );


        layout->addWidget(
            configureButton
            );


        connect(
            configureButton,
            &QPushButton::clicked,
            this,
            [this,
             guideId = guide.id]()
            {
                qDebug()
                << "PREMUTO CONFIGURA:"
                << guideId;


                emit configureRequested(
                    guideId
                    );
            }
            );


        // =========================
        // DELETE
        // =========================

        QPushButton *deleteButton =
            new QPushButton(
                "Delete",
                row
                );


        layout->addWidget(
            deleteButton
            );


        connect(
            deleteButton,
            &QPushButton::clicked,
            this,
            [this,
             guideId = guide.id]()
            {
                if(!m_manager)
                    return;


                m_manager->removeGuide(
                    guideId
                    );


                refresh();
            }
            );


        layout->addStretch();


        item->setSizeHint(
            row->sizeHint()
            );


        m_list->setItemWidget(
            item,
            row
            );
    }
}



void DistanceGuideConfigWindow::addGuide()
{
    if(!m_manager)
        return;


    if(!m_manager->characterCenterConfigured())
        return;


    bool ok =
        false;


    const QString name =
        QInputDialog::getText(
            this,
            "New Distance Guide",
            "Nome:",
            QLineEdit::Normal,
            "New Guide",
            &ok
            );


    if(!ok)
        return;


    if(name.trimmed().isEmpty())
        return;


    const QColor color =
        QColorDialog::getColor(
            Qt::white,
            this,
            "Choose Guide Color"
            );


    if(!color.isValid())
        return;


    if(m_manager->addGuide(
            name,
            color
            ))
    {
        refresh();
    }
}



void DistanceGuideConfigWindow::clearConfigurationLine()
{
    if(!m_configurationLine)
        return;


    delete m_configurationLine;


    m_configurationLine =
        nullptr;


    m_configurationGuideId.clear();
}



void DistanceGuideConfigWindow::confirmPositions()
{
    if(!m_manager)
        return;


    if(!m_configurationLine)
        return;


    if(m_configurationGuideId.isEmpty())
        return;


    if(!m_manager->characterCenterConfigured())
        return;


    DistanceGuideConfiguration updated;


    bool found =
        false;


    for(const DistanceGuideConfiguration &guide :
         m_manager->guides())
    {
        if(guide.id != m_configurationGuideId)
            continue;


        updated =
            guide;


        found =
            true;


        break;
    }


    if(!found)
        return;


    const int center =
        m_manager->characterCenter();


    const int lineX =
        m_configurationLine->configurationPositionX();


    updated.distance =
        qAbs(
            lineX -
            center
            );


    if(lineX < center)
    {
        updated.side =
            DistanceGuideSide::Left;
    }
    else
    {
        updated.side =
            DistanceGuideSide::Right;
    }


    m_manager->updateGuide(
        updated
        );


    clearConfigurationLine();
}



void DistanceGuideConfigWindow::closeEvent(
    QCloseEvent *event
    )
{
    clearConfigurationLine();

     clearCharacterCenterLine();
    event->accept();
}

void DistanceGuideConfigWindow::fixCharacterCenter()
{
    if(!m_manager)
        return;


    clearCharacterCenterLine();


    m_characterCenterLine =
        new DistanceGuideLine(
            Qt::white
            );


    m_characterCenterLine->move(
        m_manager->characterCenter(),
        0
        );


    m_characterCenterLine->resize(
        5,
        1080
        );


    m_characterCenterLine->setConfigurationMode(
        true
        );


    m_characterCenterLine->show();

    m_characterCenterLine->raise();
}

void DistanceGuideConfigWindow::confirmCharacterCenter()
{
    if(!m_manager)
        return;


    if(!m_characterCenterLine)
        return;


    const int centerX =
        m_characterCenterLine->configurationPositionX();


    m_manager->setCharacterCenter(
        centerX
        );


    clearCharacterCenterLine();
}

void DistanceGuideConfigWindow::clearCharacterCenterLine()
{
    if(!m_characterCenterLine)
        return;


    delete m_characterCenterLine;


    m_characterCenterLine =
        nullptr;
}