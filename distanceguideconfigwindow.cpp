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


            clearConfigurationObject();


            m_configurationGuideId =
                guideId;


            const QList<DistanceGuideConfiguration>
                guides =
                m_manager->guides();


            for(const DistanceGuideConfiguration &guide :
                 guides)
            {
                if(guide.id != guideId)
                    continue;


                const int centerX =
                    m_manager->effectivePositionX(
                        guide
                        );


                switch(guide.type)
                {
                case DistanceGuideType::VerticalLine:
                {
                    m_configurationLine =
                        new DistanceGuideLine(
                            guide.color
                            );


                    m_configurationLine->move(
                        centerX,
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


                case DistanceGuideType::Rectangle:
                {
                    m_configurationRectangle =
                        new DistanceGuideRectangle(
                            guide.color
                            );


                    const int width =
                        qMax(
                            20,
                            guide.width
                            );


                    const int height =
                        qMax(
                            20,
                            guide.height
                            );


                    const int x =
                        centerX -
                        width / 2;


                    const int y =
                        guide.positionY;


                    m_configurationRectangle->setGeometry(
                        x,
                        y,
                        width,
                        height
                        );


                    m_configurationRectangle->setConfigurationMode(
                        true
                        );


                    m_configurationRectangle->show();
                    m_configurationRectangle->raise();

                    break;
                }


                case DistanceGuideType::Circle:
                {
                    m_configurationCircle =
                        new DistanceGuideCircle(
                            guide.color
                            );


                    const int size =
                        qMax(
                            20,
                            guide.width
                            );


                    const int x =
                        centerX -
                        size / 2;


                    const int y =
                        guide.positionY -
                        size / 2;


                    m_configurationCircle->setGeometry(
                        x,
                        y,
                        size,
                        size
                        );


                    m_configurationCircle->setConfigurationMode(
                        true
                        );


                    m_configurationCircle->show();
                    m_configurationCircle->raise();

                    break;
                }
                }


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


    // =========================
    // TITOLO
    // =========================

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


    // =========================
    // LISTA
    // =========================

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


    // =========================
    // ADD BUTTONS
    // =========================

    QHBoxLayout *addLayout =
        new QHBoxLayout();


    m_addButton =
        new QPushButton(
            "Add Line",
            this
            );


    m_addRectangleButton =
        new QPushButton(
            "Add Rectangle",
            this
            );


    m_addCircleButton =
        new QPushButton(
            "Add Circle",
            this
            );


    m_addGroupButton =
        new QPushButton(
            "Add Group",
            this
            );


    m_addButton->setMinimumHeight(
        40
        );


    m_addRectangleButton->setMinimumHeight(
        40
        );


    m_addCircleButton->setMinimumHeight(
        40
        );


    m_addGroupButton->setMinimumHeight(
        40
        );


    addLayout->addWidget(
        m_addButton
        );


    addLayout->addWidget(
        m_addRectangleButton
        );


    addLayout->addWidget(
        m_addCircleButton
        );


    addLayout->addWidget(
        m_addGroupButton
        );


    mainLayout->addLayout(
        addLayout
        );


    // =========================
    // GLOBAL OPACITY
    // =========================

    QHBoxLayout *opacityLayout =
        new QHBoxLayout();


    QLabel *opacityLabel =
        new QLabel(
            "Global Opacity",
            this
            );


    m_opacitySlider =
        new QSlider(
            Qt::Horizontal,
            this
            );


    m_opacitySlider->setRange(
        0,
        255
        );


    m_opacitySlider->setValue(
        m_manager
            ? m_manager->globalOpacity()
            : 255
        );


    m_opacityValueLabel =
        new QLabel(
            this
            );


    m_opacityValueLabel->setMinimumWidth(
        40
        );


    m_opacityValueLabel->setAlignment(
        Qt::AlignRight |
        Qt::AlignVCenter
        );


    m_opacityValueLabel->setText(
        QString::number(
            m_opacitySlider->value()
            )
        );


    opacityLayout->addWidget(
        opacityLabel
        );


    opacityLayout->addWidget(
        m_opacitySlider
        );


    opacityLayout->addWidget(
        m_opacityValueLabel
        );


    mainLayout->addLayout(
        opacityLayout
        );


    // =========================
    // FIX CHARACTER
    // =========================

    m_fixCharacterButton =
        new QPushButton(
            "Fix Character",
            this
            );


    m_fixCharacterButton->setMinimumHeight(
        40
        );


    mainLayout->addWidget(
        m_fixCharacterButton
        );


    // =========================
    // CHIUDI
    // =========================

    m_closeButton =
        new QPushButton(
            "Chiudi",
            this
            );


    mainLayout->addWidget(
        m_closeButton
        );


    // =========================
    // CONNECTIONS
    // =========================

    connect(
        m_addCircleButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if(!m_manager)
                return;


            bool ok =
                false;


            const QString name =
                QInputDialog::getText(
                    this,
                    "New Circle",
                    "Nome:",
                    QLineEdit::Normal,
                    "New Circle",
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
                    "Choose Circle Color"
                    );


            if(!color.isValid())
                return;


            if(m_manager->addCircleGuide(
                    name,
                    color
                    ))
            {
                refresh();
            }
        }
        );
    connect(
        m_addButton,
        &QPushButton::clicked,
        this,
        &DistanceGuideConfigWindow::addGuide
        );
    connect(
        m_addRectangleButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            if(!m_manager)
                return;


            bool ok =
                false;


            const QString name =
                QInputDialog::getText(
                    this,
                    "New Rectangle",
                    "Nome:",
                    QLineEdit::Normal,
                    "New Rectangle",
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
                    "Choose Rectangle Color"
                    );


            if(!color.isValid())
                return;


            if(m_manager->addRectangleGuide(
                    name,
                    color
                    ))
            {
                refresh();
            }
        }
        );


    connect(
        m_closeButton,
        &QPushButton::clicked,
        this,
        &QWidget::close
        );


    // =========================
    // GLOBAL OPACITY
    // =========================

    connect(
        m_opacitySlider,
        &QSlider::valueChanged,
        this,
        [this](int value)
        {
            if(m_opacityValueLabel)
            {
                m_opacityValueLabel->setText(
                    QString::number(
                        value
                        )
                    );
            }


            if(m_manager)
            {
                m_manager->setGlobalOpacity(
                    value
                    );
            }
        }
        );


    connect(
        m_manager,
        &DistanceGuideManager::globalOpacityChanged,
        this,
        [this](int opacity)
        {
            if(m_opacitySlider)
            {
                m_opacitySlider->blockSignals(
                    true
                    );


                m_opacitySlider->setValue(
                    opacity
                    );


                m_opacitySlider->blockSignals(
                    false
                    );
            }


            if(m_opacityValueLabel)
            {
                m_opacityValueLabel->setText(
                    QString::number(
                        opacity
                        )
                    );
            }
        }
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



void DistanceGuideConfigWindow::clearConfigurationObject()
{
    if(m_configurationLine)
    {
        delete m_configurationLine;

        m_configurationLine =
            nullptr;
    }


    if(m_configurationRectangle)
    {
        delete m_configurationRectangle;

        m_configurationRectangle =
            nullptr;
    }


    if(m_configurationCircle)
    {
        delete m_configurationCircle;

        m_configurationCircle =
            nullptr;
    }


    m_configurationGuideId.clear();
}



void DistanceGuideConfigWindow::confirmPositions()
{
    if(!m_manager)
        return;


    if(m_configurationGuideId.isEmpty())
        return;


    if(!m_configurationLine &&
        !m_configurationRectangle &&
        !m_configurationCircle)
    {
        return;
    }


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


    // ==================================================
    // LINEA
    // ==================================================

    if(m_configurationLine)
    {
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
    }


    // ==================================================
    // RETTANGOLO
    // ==================================================

    if(m_configurationRectangle)
    {
        const int rectangleX =
            m_configurationRectangle->
            configurationPositionX();


        const int rectangleY =
            m_configurationRectangle->
            configurationPositionY();


        const int rectangleWidth =
            m_configurationRectangle->
            configurationWidth();


        const int rectangleHeight =
            m_configurationRectangle->
            configurationHeight();


        /*
         * Il distance viene riferito al CENTRO
         * del rettangolo, non al suo bordo sinistro.
         */

        const int rectangleCenterX =
            rectangleX +
            rectangleWidth / 2;


        updated.distance =
            qAbs(
                rectangleCenterX -
                center
                );


        if(rectangleCenterX < center)
        {
            updated.side =
                DistanceGuideSide::Left;
        }
        else
        {
            updated.side =
                DistanceGuideSide::Right;
        }


        updated.positionY =
            rectangleY;


        updated.width =
            rectangleWidth;


        updated.height =
            rectangleHeight;
    }
    // ==================================================
    // CERCHIO
    // ==================================================

    if(m_configurationCircle)
    {
        const int circleCenterX =
            m_configurationCircle->
            configurationPositionX();


        const int circleCenterY =
            m_configurationCircle->
            configurationPositionY();


        const int circleSize =
            m_configurationCircle->
            configurationSize();


        updated.distance =
            qAbs(
                circleCenterX -
                center
                );


        if(circleCenterX < center)
        {
            updated.side =
                DistanceGuideSide::Left;
        }
        else
        {
            updated.side =
                DistanceGuideSide::Right;
        }


        updated.positionY =
            circleCenterY;


        updated.width =
            circleSize;


        updated.height =
            circleSize;
    }


    // ==================================================
    // SALVATAGGIO
    // ==================================================

    m_manager->updateGuide(
        updated
        );


    clearConfigurationObject();


    refresh();
}



void DistanceGuideConfigWindow::closeEvent(
    QCloseEvent *event
    )
{
    clearConfigurationObject();

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