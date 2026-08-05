#include "buffvisioncapturesetup.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include "buffvisionconfig.h"



BuffVisionCaptureSetup::BuffVisionCaptureSetup(
    QWidget *parent
    )
    : QWidget(parent)
{

    setGeometry(
        QApplication::primaryScreen()->geometry()
        );
    setFocusPolicy(
        Qt::StrongFocus
        );
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );



    captureRect =
        QRect(
            500,
            300,
            100,
            55
            );



    cropRect1 =
        QRect(
            505,
            312,
        BuffVisionConfig::CROP_WIDTH,
        BuffVisionConfig::CROP_HEIGHT
            );


    cropRect2 =
        QRect(
            565,
            312,
        BuffVisionConfig::CROP_WIDTH,
        BuffVisionConfig::CROP_HEIGHT
            );
    loadSettings();
    connect(
        &feedbackTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            feedbackText.clear();
            update();
        }
        );
}



void BuffVisionCaptureSetup::paintEvent(
    QPaintEvent *
    )
{

    QPainter p(this);

    p.setRenderHint(
        QPainter::Antialiasing
        );


    // =========================
    // AREA SCREENSHOT COMPLETA
    // =========================

    p.setPen(
        Qt::NoPen
        );

    p.setBrush(
        QColor(
            0,
            120,
            255,
            70
            )
        );


    if(!captureMode)
    {
        p.drawRect(
            captureRect
            );
    }



    // =========================
    // AREA CROP 1
    // =========================

    p.setBrush(
        QColor(
            0,
            255,
            0,
            100
            )
        );

    if(!captureMode)
    {
        p.drawRect(
            cropRect1
            );
    }



    // =========================
    // AREA CROP 2
    // =========================

    p.setBrush(
        QColor(
            255,
            220,
            0,
            100
            )
        );

    if(!captureMode)
    {
        p.drawRect(
            cropRect2
            );
    }
    if(!feedbackText.isEmpty())
    {

        QFont font;

        font.setBold(true);
        font.setPointSize(18);


        p.setFont(font);


        p.setPen(
            Qt::white
            );


        p.drawText(
            rect(),
            Qt::AlignCenter,
            feedbackText
            );

    }

}


void BuffVisionCaptureSetup::mousePressEvent(
    QMouseEvent *event
    )
{

    QPoint pos =
        event->position().toPoint();



    if(cropRect1.contains(pos))
    {
        dragMode = Crop1;
    }

    else if(cropRect2.contains(pos))
    {
        dragMode = Crop2;
    }

    else if(captureRect.contains(pos))
    {
        dragMode = Capture;
    }

    else
    {
        dragMode = None;
    }


    lastMousePosition = pos;

}

void BuffVisionCaptureSetup::mouseMoveEvent(
    QMouseEvent *event
    )
{

    if(dragMode == None)
        return;



    QPoint current =
        event->position().toPoint();



    QPoint delta =
        current - lastMousePosition;



    switch(dragMode)
    {

    case Capture:

        moveCapture(delta);

        break;


    case Crop1:

        moveCrop(
            cropRect1,
            delta
            );

        break;


    case Crop2:

        moveCrop(
            cropRect2,
            delta
            );

        break;


    default:
        break;

    }



    lastMousePosition = current;


    update();

}

void BuffVisionCaptureSetup::mouseReleaseEvent(
    QMouseEvent *
    )
{

    dragMode = None;

}

void BuffVisionCaptureSetup::moveCapture(
    QPoint delta
    )
{

    captureRect.translate(delta);


    cropRect1.translate(delta);

    cropRect2.translate(delta);

}

void BuffVisionCaptureSetup::moveCrop(
    QRect &crop,
    QPoint delta
    )
{

    crop.translate(delta);


    keepInsideCapture(crop);

}

void BuffVisionCaptureSetup::keepInsideCapture(
    QRect &crop
    )
{

    if(crop.left() < captureRect.left())
    {
        crop.moveLeft(
            captureRect.left()
            );
    }


    if(crop.top() < captureRect.top())
    {
        crop.moveTop(
            captureRect.top()
            );
    }


    if(crop.right() > captureRect.right())
    {
        crop.moveRight(
            captureRect.right()
            );
    }


    if(crop.bottom() > captureRect.bottom())
    {
        crop.moveBottom(
            captureRect.bottom()
            );
    }

}

void BuffVisionCaptureSetup::loadSettings()
{

    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    captureRect.moveTo(
        settings.value(
                    "BuffVision/CaptureX",
                    captureRect.x()
                    ).toInt(),

        settings.value(
                    "BuffVision/CaptureY",
                    captureRect.y()
                    ).toInt()
        );



    cropRect1.moveTo(
        settings.value(
                    "BuffVision/Crop1X",
                    cropRect1.x()
                    ).toInt(),

        settings.value(
                    "BuffVision/Crop1Y",
                    cropRect1.y()
                    ).toInt()
        );



    cropRect2.moveTo(
        settings.value(
                    "BuffVision/Crop2X",
                    cropRect2.x()
                    ).toInt(),

        settings.value(
                    "BuffVision/Crop2Y",
                    cropRect2.y()
                    ).toInt()
        );

}

void BuffVisionCaptureSetup::saveSettings()
{

    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    settings.setValue(
        "BuffVision/CaptureX",
        captureRect.x()
        );

    settings.setValue(
        "BuffVision/CaptureY",
        captureRect.y()
        );


    settings.setValue(
        "BuffVision/Crop1X",
        cropRect1.x()
        );

    settings.setValue(
        "BuffVision/Crop1Y",
        cropRect1.y()
        );


    settings.setValue(
        "BuffVision/Crop2X",
        cropRect2.x()
        );

    settings.setValue(
        "BuffVision/Crop2Y",
        cropRect2.y()
        );


    settings.sync();

}

void BuffVisionCaptureSetup::resetSetup()
{

    configured = false;

    hide();

}

QRect BuffVisionCaptureSetup::getCropRect1() const
{
    return cropRect1;
}


QRect BuffVisionCaptureSetup::getCropRect2() const
{
    return cropRect2;
}

void BuffVisionCaptureSetup::showFeedback(
    const QString &text
    )
{

    feedbackText = text;

    feedbackTimer.start(1500);

    update();

}

void BuffVisionCaptureSetup::setCaptureMode(
    bool active
    )
{

    captureMode = active;

    update();

}