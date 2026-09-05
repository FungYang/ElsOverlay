
#include "buffvisionoverlay.h"

#include <QColor>
#include <QFont>
#include <QPainter>


BuffVisionOverlay::BuffVisionOverlay(
    BuffVisionCore *core,
    QWidget *parent
    )
    : QWidget(parent),
    core(core)
{
    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TransparentForMouseEvents,
        false
        );


    setMouseTracking(
        true
        );


    // ========================================================
    // SETTINGS
    // ========================================================

    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    int savedSize =
        settings.value(
                    "Overlay/BuffVision/size",
                    80
                    ).toInt();


    if(savedSize < MinimumSize)
    {
        savedSize =
            80;
    }


    setMinimumSize(
        MinimumSize,
        MinimumSize
        );


    resize(
        savedSize,
        savedSize
        );


    move(
        settings.value(
                    "Overlay/BuffVision/position",
                    QPoint(700,300)
                    ).toPoint()
        );


    // ========================================================
    // IMMAGINI
    // ========================================================

    buffImage.load(
        "images/Flow.png"
        );


    grayBuffImage =
        createGrayImage(
            buffImage
            );


    // ========================================================
    // BUFF 15
    // ========================================================

    connect(
        core,
        &BuffVisionCore::buff15Activated,
        this,
        [this]()
        {
            cooldown15 =
                15;

            update();
        }
        );


    connect(
        core,
        &BuffVisionCore::buff15Expired,
        this,
        [this]()
        {
            cooldown15 =
                0;

            update();
        }
        );


    // ========================================================
    // BUFF 60
    // ========================================================

    connect(
        core,
        &BuffVisionCore::buff60Activated,
        this,
        [this]()
        {
            update();
        }
        );


    connect(
        core,
        &BuffVisionCore::buff60Expired,
        this,
        [this]()
        {
            update();
        }
        );


    // ========================================================
    // TIMER
    // ========================================================

    connect(
        &timer,
        &QTimer::timeout,
        this,
        [this]()
        {
            refreshCounter +=
                100;


            if(refreshCounter >= 1000)
            {
                refreshCounter =
                    0;


                if(cooldown15 > 0)
                {
                    cooldown15--;
                }
            }


            update();
        }
        );


    timer.start(
        100
        );
}


// ============================================================
// PAINT
// ============================================================

void BuffVisionOverlay::paintEvent(
    QPaintEvent *
    )
{
    QPainter p(this);


    p.setRenderHint(
        QPainter::Antialiasing
        );


    // ========================================================
    // IMMAGINE
    // ========================================================

    const QPixmap &image =
        core->isBuff60Active()
            ? buffImage
            : grayBuffImage;


    p.drawPixmap(
        10,
        10,
        width() - 20,
        height() - 20,
        image
        );


    // ========================================================
    // COUNTDOWN
    // ========================================================

    if(cooldown15 > 0)
    {
        QFont font;

        font.setBold(
            true
            );


        font.setPointSize(
            qMax(
                10,
                width() / 4
                )
            );


        p.setFont(
            font
            );


        p.setPen(
            Qt::white
            );


        p.drawText(
            rect(),
            Qt::AlignCenter,
            QString::number(
                cooldown15
                )
            );
    }


    // ========================================================
    // MANIGLIE AGLI ANGOLI
    // ========================================================

    const int half =
        HandleSize / 2;


    p.setPen(
        Qt::NoPen
        );


    p.setBrush(
        Qt::white
        );


    // TOP LEFT

    p.drawRect(
        ResizeMargin - half,
        ResizeMargin - half,
        HandleSize,
        HandleSize
        );


    // TOP RIGHT

    p.drawRect(
        width() -
            ResizeMargin -
            half,
        ResizeMargin - half,
        HandleSize,
        HandleSize
        );


    // BOTTOM LEFT

    p.drawRect(
        ResizeMargin - half,
        height() -
            ResizeMargin -
            half,
        HandleSize,
        HandleSize
        );


    // BOTTOM RIGHT

    p.drawRect(
        width() -
            ResizeMargin -
            half,
        height() -
            ResizeMargin -
            half,
        HandleSize,
        HandleSize
        );
}


// ============================================================
// MOUSE PRESS
// ============================================================

void BuffVisionOverlay::mousePressEvent(
    QMouseEvent *event
    )
{
    if(
        event->button() !=
        Qt::LeftButton
        )
    {
        return;
    }


    QPoint localPosition =
        event->position().toPoint();


    ResizeCorner corner =
        resizeCornerAt(
            localPosition
            );


    // ========================================================
    // RESIZE
    // ========================================================

    if(
        corner !=
        ResizeCorner::None
        )
    {
        resizing =
            true;


        dragging =
            false;


        resizeCorner =
            corner;


        resizeStartGlobal =
            event->globalPosition().toPoint();


        resizeStartPosition =
            pos();


        resizeStartSize =
            size();


        event->accept();

        return;
    }


    // ========================================================
    // DRAG
    // ========================================================

    dragging =
        true;


    resizing =
        false;


    resizeCorner =
        ResizeCorner::None;


    dragPosition =
        event->globalPosition().toPoint()
        -
        frameGeometry().topLeft();


    event->accept();
}


// ============================================================
// MOUSE MOVE
// ============================================================

void BuffVisionOverlay::mouseMoveEvent(
    QMouseEvent *event
    )
{
    QPoint localPosition =
        event->position().toPoint();


    // ========================================================
    // RESIZE
    // ========================================================

    if(resizing)
    {
        resizeFromCorner(
            event->globalPosition().toPoint()
            );


        event->accept();

        return;
    }


    // ========================================================
    // DRAG
    // ========================================================

    if(dragging)
    {
        move(
            event->globalPosition().toPoint()
            -
            dragPosition
            );


        saveGeometry();


        event->accept();

        return;
    }


    // ========================================================
    // CURSORE
    // ========================================================

    updateResizeCursor(
        localPosition
        );
}


// ============================================================
// MOUSE RELEASE
// ============================================================

void BuffVisionOverlay::mouseReleaseEvent(
    QMouseEvent *event
    )
{
    if(
        event->button() !=
        Qt::LeftButton
        )
    {
        return;
    }


    dragging =
        false;


    resizing =
        false;


    resizeCorner =
        ResizeCorner::None;


    saveGeometry();


    updateResizeCursor(
        event->position().toPoint()
        );


    event->accept();
}


// ============================================================
// ENTER
// ============================================================

void BuffVisionOverlay::enterEvent(
    QEnterEvent *event
    )
{
    updateResizeCursor(
        event->position().toPoint()
        );


    QWidget::enterEvent(
        event
        );
}


// ============================================================
// LEAVE
// ============================================================

void BuffVisionOverlay::leaveEvent(
    QEvent *event
    )
{
    unsetCursor();


    QWidget::leaveEvent(
        event
        );
}


// ============================================================
// TROVA ANGOLO
// ============================================================

BuffVisionOverlay::ResizeCorner
BuffVisionOverlay::resizeCornerAt(
    const QPoint &position
    ) const
{
    const int margin =
        ResizeMargin;


    const int x =
        position.x();


    const int y =
        position.y();


    const int w =
        width();


    const int h =
        height();


    bool left =
        x <= margin;


    bool right =
        x >= w - margin;


    bool top =
        y <= margin;


    bool bottom =
        y >= h - margin;


    if(left && top)
    {
        return ResizeCorner::TopLeft;
    }


    if(right && top)
    {
        return ResizeCorner::TopRight;
    }


    if(left && bottom)
    {
        return ResizeCorner::BottomLeft;
    }


    if(right && bottom)
    {
        return ResizeCorner::BottomRight;
    }


    return ResizeCorner::None;
}


// ============================================================
// CURSORE
// ============================================================

void BuffVisionOverlay::updateResizeCursor(
    const QPoint &position
    )
{
    ResizeCorner corner =
        resizeCornerAt(
            position
            );


    switch(corner)
    {
    case ResizeCorner::TopLeft:

        setCursor(
            Qt::SizeFDiagCursor
            );

        break;


    case ResizeCorner::BottomRight:

        setCursor(
            Qt::SizeFDiagCursor
            );

        break;


    case ResizeCorner::TopRight:

        setCursor(
            Qt::SizeBDiagCursor
            );

        break;


    case ResizeCorner::BottomLeft:

        setCursor(
            Qt::SizeBDiagCursor
            );

        break;


    case ResizeCorner::None:

        setCursor(
            Qt::ArrowCursor
            );

        break;
    }
}


// ============================================================
// RESIZE QUADRATO
// ============================================================

void BuffVisionOverlay::resizeFromCorner(
    const QPoint &globalPosition
    )
{
    QPoint delta =
        globalPosition -
        resizeStartGlobal;


    const int startSize =
        resizeStartSize.width();


    int newSize =
        startSize;


    QPoint newPosition =
        resizeStartPosition;


    // ========================================================
    // TOP LEFT
    // ========================================================

    if(
        resizeCorner ==
        ResizeCorner::TopLeft
        )
    {
        int deltaValue =
            qMax(
                qAbs(delta.x()),
                qAbs(delta.y())
                );


        bool shrinking =
            delta.x() > 0 ||
            delta.y() > 0;


        if(shrinking)
        {
            newSize =
                startSize -
                deltaValue;
        }
        else
        {
            newSize =
                startSize +
                deltaValue;
        }


        if(newSize < MinimumSize)
        {
            newSize =
                MinimumSize;
        }


        newPosition.setX(
            resizeStartPosition.x() +
            startSize -
            newSize
            );


        newPosition.setY(
            resizeStartPosition.y() +
            startSize -
            newSize
            );
    }


    // ========================================================
    // TOP RIGHT
    // ========================================================

    else if(
        resizeCorner ==
        ResizeCorner::TopRight
        )
    {
        int deltaValue =
            qMax(
                qAbs(delta.x()),
                qAbs(delta.y())
                );


        bool growing =
            delta.x() > 0 ||
            delta.y() < 0;


        if(growing)
        {
            newSize =
                startSize +
                deltaValue;
        }
        else
        {
            newSize =
                startSize -
                deltaValue;
        }


        if(newSize < MinimumSize)
        {
            newSize =
                MinimumSize;
        }


        newPosition.setY(
            resizeStartPosition.y() +
            startSize -
            newSize
            );
    }


    // ========================================================
    // BOTTOM LEFT
    // ========================================================

    else if(
        resizeCorner ==
        ResizeCorner::BottomLeft
        )
    {
        int deltaValue =
            qMax(
                qAbs(delta.x()),
                qAbs(delta.y())
                );


        bool growing =
            delta.x() < 0 ||
            delta.y() > 0;


        if(growing)
        {
            newSize =
                startSize +
                deltaValue;
        }
        else
        {
            newSize =
                startSize -
                deltaValue;
        }


        if(newSize < MinimumSize)
        {
            newSize =
                MinimumSize;
        }


        newPosition.setX(
            resizeStartPosition.x() +
            startSize -
            newSize
            );
    }


    // ========================================================
    // BOTTOM RIGHT
    // ========================================================

    else if(
        resizeCorner ==
        ResizeCorner::BottomRight
        )
    {
        int deltaValue =
            qMax(
                qAbs(delta.x()),
                qAbs(delta.y())
                );


        bool growing =
            delta.x() > 0 ||
            delta.y() > 0;


        if(growing)
        {
            newSize =
                startSize +
                deltaValue;
        }
        else
        {
            newSize =
                startSize -
                deltaValue;
        }


        if(newSize < MinimumSize)
        {
            newSize =
                MinimumSize;
        }
    }


    // ========================================================
    // APPLICA
    // ========================================================

    setGeometry(
        QRect(
            newPosition,
            QSize(
                newSize,
                newSize
                )
            )
        );


    update();
}


// ============================================================
// SALVA POSIZIONE + DIMENSIONE
// ============================================================

void BuffVisionOverlay::saveGeometry()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    settings.setValue(
        "Overlay/BuffVision/position",
        pos()
        );


    settings.setValue(
        "Overlay/BuffVision/size",
        width()
        );


    settings.sync();
}


// ============================================================
// CREATE GRAY IMAGE
// ============================================================

QPixmap BuffVisionOverlay::createGrayImage(
    const QPixmap &source
    )
{
    QImage img =
        source.toImage()
            .convertToFormat(
                QImage::Format_ARGB32
                );


    for(
        int y = 0;
        y < img.height();
        y++
        )
    {
        for(
            int x = 0;
            x < img.width();
            x++
            )
        {
            QColor c =
                img.pixelColor(
                    x,
                    y
                    );


            int gray =
                qGray(
                    c.rgb()
                    );


            img.setPixelColor(
                x,
                y,
                QColor(
                    gray,
                    gray,
                    gray,
                    c.alpha()
                    )
                );
        }
    }


    return QPixmap::fromImage(
        img
        );
}


// ============================================================
// RESET
// ============================================================

void BuffVisionOverlay::resetOverlay()
{
    buff60Active =
        false;


    cooldown15 =
        0;


    update();
}


// ============================================================
// SCORES
// ============================================================

void BuffVisionOverlay::setScores(
    double s1,
    double s2
    )
{
    score1 =
        s1;


    score2 =
        s2;


    update();
}


// ============================================================
// UPDATE BUFF COLOR
// ============================================================

void BuffVisionOverlay::updateBuffColor()
{
    update();
}
