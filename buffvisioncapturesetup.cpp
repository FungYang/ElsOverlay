#include "buffvisioncapturesetup.h"

#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QKeyEvent>

#include "buffvisionconfig.h"


BuffVisionCaptureSetup::BuffVisionCaptureSetup(
    QWidget *parent
    )
    : QWidget(parent)
{
    QScreen *screen =
        QApplication::primaryScreen();

    if(screen)
    {
        setGeometry(
            screen->geometry()
            );
    }

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


    const int size =
        currentCropSize();


    // Posizioni iniziali di default.
    cropRect1 =
        QRect(
            500,
            300,
            size,
            size
            );


    cropRect2 =
        QRect(
            600,
            300,
            size,
            size
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


// =========================================================
// CROP SIZE
// =========================================================

int BuffVisionCaptureSetup::currentCropSize() const
{
    QScreen *screen =
        QApplication::primaryScreen();

    if(!screen)
        return BuffVisionConfig::FULL_HD_CROP_SIZE;


    return BuffVisionConfig::cropSizeForScreen(
        screen->size()
        );
}


// =========================================================
// PAINT
// =========================================================

void BuffVisionCaptureSetup::paintEvent(
    QPaintEvent *
    )
{
    QPainter p(this);

    p.setRenderHint(
        QPainter::Antialiasing
        );


    // =========================
    // CROP 1
    // =========================

    p.setBrush(
        QColor(
            0,
            255,
            0,
            100
            )
        );

    p.setPen(
        QPen(
            Qt::green,
            2
            )
        );

    p.drawRect(
        cropRect1
        );


    // =========================
    // CROP 2
    // =========================

    p.setBrush(
        QColor(
            255,
            220,
            0,
            100
            )
        );

    p.setPen(
        QPen(
            Qt::yellow,
            2
            )
        );

    p.drawRect(
        cropRect2
        );


    // =========================
    // FEEDBACK
    // =========================

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


// =========================================================
// MOUSE PRESS
// =========================================================

void BuffVisionCaptureSetup::mousePressEvent(
    QMouseEvent *event
    )
{
    const QPoint pos =
        event->position().toPoint();


    if(cropRect1.contains(pos))
    {
        dragMode = Crop1;
    }
    else if(cropRect2.contains(pos))
    {
        dragMode = Crop2;
    }
    else
    {
        dragMode = None;
    }


    lastMousePosition = pos;
}


// =========================================================
// MOUSE MOVE
// =========================================================

void BuffVisionCaptureSetup::mouseMoveEvent(
    QMouseEvent *event
    )
{
    if(dragMode == None)
        return;


    const QPoint current =
        event->position().toPoint();


    const QPoint delta =
        current - lastMousePosition;


    switch(dragMode)
    {
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


// =========================================================
// MOUSE RELEASE
// =========================================================

void BuffVisionCaptureSetup::mouseReleaseEvent(
    QMouseEvent *
    )
{
    dragMode = None;
}


// =========================================================
// MOVE CROP
// =========================================================

void BuffVisionCaptureSetup::moveCrop(
    QRect &crop,
    QPoint delta
    )
{
    crop.translate(
        delta
        );

    keepInsideScreen(
        crop
        );
}


// =========================================================
// KEEP INSIDE SCREEN
// =========================================================

void BuffVisionCaptureSetup::keepInsideScreen(
    QRect &crop
    )
{
    const QRect screenRect =
        rect();


    if(crop.left() < screenRect.left())
    {
        crop.moveLeft(
            screenRect.left()
            );
    }


    if(crop.top() < screenRect.top())
    {
        crop.moveTop(
            screenRect.top()
            );
    }


    if(crop.right() > screenRect.right())
    {
        crop.moveRight(
            screenRect.right()
            );
    }


    if(crop.bottom() > screenRect.bottom())
    {
        crop.moveBottom(
            screenRect.bottom()
            );
    }
}


// =========================================================
// SETTINGS
// =========================================================

void BuffVisionCaptureSetup::loadSettings()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    QScreen *screen =
        QApplication::primaryScreen();

    if(!screen)
        return;


    const QSize resolution =
        screen->size();

    const QString suffix =
        QString("_%1x%2")
            .arg(resolution.width())
            .arg(resolution.height());


    const int size =
        currentCropSize();


    cropRect1.setSize(
        QSize(size, size)
        );

    cropRect2.setSize(
        QSize(size, size)
        );


    cropRect1.moveTo(
        settings.value(
                    "BuffVision/Crop1X" + suffix,
                    cropRect1.x()
                    ).toInt(),

        settings.value(
                    "BuffVision/Crop1Y" + suffix,
                    cropRect1.y()
                    ).toInt()
        );


    cropRect2.moveTo(
        settings.value(
                    "BuffVision/Crop2X" + suffix,
                    cropRect2.x()
                    ).toInt(),

        settings.value(
                    "BuffVision/Crop2Y" + suffix,
                    cropRect2.y()
                    ).toInt()
        );


    keepInsideScreen(cropRect1);
    keepInsideScreen(cropRect2);
}


// =========================================================
// SAVE SETTINGS
// =========================================================

void BuffVisionCaptureSetup::saveSettings()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    QScreen *screen =
        QApplication::primaryScreen();

    if(!screen)
        return;


    const QSize resolution =
        screen->size();


    const QString suffix =
        QString("_%1x%2")
            .arg(resolution.width())
            .arg(resolution.height());


    settings.setValue(
        "BuffVision/Crop1X" + suffix,
        cropRect1.x()
        );

    settings.setValue(
        "BuffVision/Crop1Y" + suffix,
        cropRect1.y()
        );


    settings.setValue(
        "BuffVision/Crop2X" + suffix,
        cropRect2.x()
        );

    settings.setValue(
        "BuffVision/Crop2Y" + suffix,
        cropRect2.y()
        );


    settings.sync();
}


// =========================================================
// RESET
// =========================================================

void BuffVisionCaptureSetup::resetSetup()
{
    configured = false;

    hide();
}


// =========================================================
// GET CROP
// =========================================================

QRect BuffVisionCaptureSetup::getCropRect1() const
{
    return cropRect1;
}


QRect BuffVisionCaptureSetup::getCropRect2() const
{
    return cropRect2;
}


// =========================================================
// FEEDBACK
// =========================================================

void BuffVisionCaptureSetup::showFeedback(
    const QString &text
    )
{
    feedbackText = text;

    feedbackTimer.start(
        1500
        );

    update();
}


// =========================================================
// CAPTURE MODE
// =========================================================

void BuffVisionCaptureSetup::setCaptureMode(
    bool active
    )
{
    captureMode = active;

    update();
}


// =========================================================
// KEYBOARD
// =========================================================

void BuffVisionCaptureSetup::keyPressEvent(
    QKeyEvent *event
    )
{
    if(event->key() == Qt::Key_Escape)
    {
        hide();

        event->accept();

        return;
    }


    QWidget::keyPressEvent(
        event
        );
}