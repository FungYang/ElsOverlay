
#include "buffbox.h"

#include <QEnterEvent>
#include <QFont>
#include <QMouseEvent>
#include <QPainter>


#ifdef Q_OS_WIN
#include <windows.h>
#endif


BuffBox::BuffBox(
    int key,
    int cooldown,
    QWidget *parent
    )
    : QWidget(parent),
    m_key(key),
    m_cooldown(cooldown),
    m_configurationMode(false)
{
    // ========================================================
    // DIMENSIONE INIZIALE
    // ========================================================

    resize( 42, 42 );


    setMinimumSize(
        MinimumSize,
        MinimumSize
        );


    // ========================================================
    // TRASPARENZA
    // ========================================================

    setAttribute(
        Qt::WA_TranslucentBackground
        );


    // IMPORTANTE:
    // NON usare WA_TransparentForMouseEvents.
    //
    // Anche se il box è trasparente, deve continuare
    // a ricevere gli eventi del mouse.


    setMouseTracking(
        true
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    // ========================================================
    // TIMER COOLDOWN
    // ========================================================

    m_timer.setSingleShot(
        true
        );


    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        &BuffBox::cooldownFinished
        );


    // ========================================================
    // TIMER GLOW
    // ========================================================

    m_glowTimer.setInterval(
        30
        );


    connect(
        &m_glowTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            if(m_glowIncreasing)
            {
                m_glowAlpha += 8;


                if(m_glowAlpha >= 160)
                {
                    m_glowIncreasing =
                        false;
                }
            }
            else
            {
                m_glowAlpha -= 8;


                if(m_glowAlpha <= 80)
                {
                    m_glowIncreasing =
                        true;
                }
            }


            update();
        }
        );
}


// ============================================================
// PAINT
// ============================================================

void BuffBox::paintEvent(
    QPaintEvent *
    )
{
    QPainter painter(this);


    painter.setRenderHint(
        QPainter::Antialiasing
        );


    // ========================================================
    // GLOW SCADUTO
    // ========================================================

    if(m_state == State::Expired)
    {
        painter.setBrush(
            QColor(
                255,
                255,
                255,
                m_glowAlpha
                )
            );


        painter.setPen(
            Qt::NoPen
            );


        painter.drawRect(
            rect()
            );
    }


    // ========================================================
    // MODALITÀ CONFIGURAZIONE
    // ========================================================

    if(m_configurationMode)
    {
        // ----------------------------------------------------
        // AREA INTERNA QUASI INVISIBILE
        // ----------------------------------------------------
        //
        // Questa zona rende evidente che il box è attivo
        // solo durante la configurazione.
        //
        // L'alpha è volutamente molto basso.
        //

        painter.setBrush(
            QColor(
                255,
                255,
                255,
                8
                )
            );


        painter.setPen(
            Qt::NoPen
            );


        painter.drawRect(
            rect()
            );


        // ----------------------------------------------------
        // BORDO
        // ----------------------------------------------------

        painter.setPen(
            QPen(
                Qt::white,
                2
                )
            );


        painter.setBrush(
            Qt::NoBrush
            );


        painter.drawRect(
            1,
            1,
            width() - 2,
            height() - 2
            );


        // ----------------------------------------------------
        // TASTO
        // ----------------------------------------------------

        painter.setPen(
            Qt::white
            );


        painter.setFont(
            QFont(
                "Arial",
                18,
                QFont::Bold
                )
            );


        painter.drawText(
            rect(),
            Qt::AlignCenter,
            keyName(m_key)
            );


        // ----------------------------------------------------
        // INDICATORI AGLI ANGOLI
        // ----------------------------------------------------
        //
        // Sono molto piccoli e poco invasivi.
        // Servono per far capire dove si può ridimensionare.
        //

        const int handleSize =
            5;


        painter.setBrush(
            QColor(
                255,
                255,
                255,
                180
                )
            );


        painter.setPen(
            Qt::NoPen
            );


        // TOP LEFT

        painter.drawRect(
            1,
            1,
            handleSize,
            handleSize
            );


        // TOP RIGHT

        painter.drawRect(
            width() - handleSize - 1,
            1,
            handleSize,
            handleSize
            );


        // BOTTOM LEFT

        painter.drawRect(
            1,
            height() - handleSize - 1,
            handleSize,
            handleSize
            );


        // BOTTOM RIGHT

        painter.drawRect(
            width() - handleSize - 1,
            height() - handleSize - 1,
            handleSize,
            handleSize
            );
    }
}


// ============================================================
// MOUSE PRESS
// ============================================================

void BuffBox::mousePressEvent(
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


    // --------------------------------------------------------
    // RIDIMENSIONAMENTO
    // --------------------------------------------------------

    if(m_configurationMode)
    {
        ResizeCorner corner =
            resizeCornerAt(
                event->position().toPoint()
                );


        if(
            corner !=
            ResizeCorner::None
            )
        {
            m_resizing =
                true;


            m_dragging =
                false;


            m_resizeCorner =
                corner;


            m_resizeStartGlobal =
                event->globalPosition().toPoint();


            m_resizeStartPosition =
                pos();


            m_resizeStartSize =
                size();


            event->accept();

            return;
        }
    }


    // --------------------------------------------------------
    // SPOSTAMENTO
    // --------------------------------------------------------

    m_dragging =
        true;


    m_resizing =
        false;


    m_resizeCorner =
        ResizeCorner::None;


    m_dragOffset =
        event->position().toPoint();


    event->accept();
}


// ============================================================
// MOUSE MOVE
// ============================================================

void BuffBox::mouseMoveEvent(
    QMouseEvent *event
    )
{
    QPoint localPosition =
        event->position().toPoint();


    // ========================================================
    // RIDIMENSIONAMENTO
    // ========================================================

    if(
        m_configurationMode &&
        m_resizing
        )
    {
        resizeFromCorner(
            event->globalPosition().toPoint()
            );


        event->accept();

        return;
    }


    // ========================================================
    // SPOSTAMENTO
    // ========================================================

    if(m_dragging)
    {
        move(
            event->globalPosition().toPoint()
            -
            m_dragOffset
            );


        emit positionChanged(
            pos()
            );


        event->accept();

        return;
    }


    // ========================================================
    // CURSORE RIDIMENSIONAMENTO
    // ========================================================

    if(m_configurationMode)
    {
        updateResizeCursor(
            localPosition
            );
    }
}


// ============================================================
// MOUSE RELEASE
// ============================================================

void BuffBox::mouseReleaseEvent(
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


    bool wasResizing =
        m_resizing;


    m_dragging =
        false;


    m_resizing =
        false;


    m_resizeCorner =
        ResizeCorner::None;


    if(wasResizing)
    {
        emit sizeChanged(
            size()
            );
    }


    if(m_configurationMode)
    {
        updateResizeCursor(
            event->position().toPoint()
            );
    }
    else
    {
        unsetCursor();
    }


    event->accept();
}


// ============================================================
// MOUSE ENTER
// ============================================================

void BuffBox::enterEvent(
    QEnterEvent *event
    )
{
    if(m_configurationMode)
    {
        updateResizeCursor(
            event->position().toPoint()
            );
    }


    QWidget::enterEvent(
        event
        );
}


// ============================================================
// MOUSE LEAVE
// ============================================================

void BuffBox::leaveEvent(
    QEvent *event
    )
{
    unsetCursor();


    QWidget::leaveEvent(
        event
        );
}


// ============================================================
// RIDIMENSIONAMENTO - RICONOSCIMENTO ANGOLO
// ============================================================

BuffBox::ResizeCorner BuffBox::resizeCornerAt(
    const QPoint &position
    ) const
{
    if(!m_configurationMode)
    {
        return ResizeCorner::None;
    }


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


    // --------------------------------------------------------
    // TOP LEFT
    // --------------------------------------------------------

    if(
        left &&
        top
        )
    {
        return ResizeCorner::TopLeft;
    }


    // --------------------------------------------------------
    // TOP RIGHT
    // --------------------------------------------------------

    if(
        right &&
        top
        )
    {
        return ResizeCorner::TopRight;
    }


    // --------------------------------------------------------
    // BOTTOM LEFT
    // --------------------------------------------------------

    if(
        left &&
        bottom
        )
    {
        return ResizeCorner::BottomLeft;
    }


    // --------------------------------------------------------
    // BOTTOM RIGHT
    // --------------------------------------------------------

    if(
        right &&
        bottom
        )
    {
        return ResizeCorner::BottomRight;
    }


    return ResizeCorner::None;
}


// ============================================================
// CURSORE RIDIMENSIONAMENTO
// ============================================================

void BuffBox::updateResizeCursor(
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
// RIDIMENSIONAMENTO
// ============================================================

void BuffBox::resizeFromCorner(
    const QPoint &globalPosition
    )
{
    QPoint delta =
        globalPosition -
        m_resizeStartGlobal;


    QRect newGeometry;


    switch(m_resizeCorner)
    {
        // --------------------------------------------------------
        // TOP LEFT
        // --------------------------------------------------------

    case ResizeCorner::TopLeft:
    {
        int newLeft =
            m_resizeStartPosition.x() +
            delta.x();


        int newTop =
            m_resizeStartPosition.y() +
            delta.y();


        int newWidth =
            m_resizeStartSize.width() -
            delta.x();


        int newHeight =
            m_resizeStartSize.height() -
            delta.y();


        if(newWidth < MinimumSize)
        {
            newWidth =
                MinimumSize;


            newLeft =
                m_resizeStartPosition.x() +
                m_resizeStartSize.width() -
                MinimumSize;
        }


        if(newHeight < MinimumSize)
        {
            newHeight =
                MinimumSize;


            newTop =
                m_resizeStartPosition.y() +
                m_resizeStartSize.height() -
                MinimumSize;
        }


        newGeometry =
            QRect(
                newLeft,
                newTop,
                newWidth,
                newHeight
                );

        break;
    }


        // --------------------------------------------------------
        // TOP RIGHT
        // --------------------------------------------------------

    case ResizeCorner::TopRight:
    {
        int newTop =
            m_resizeStartPosition.y() +
            delta.y();


        int newWidth =
            m_resizeStartSize.width() +
            delta.x();


        int newHeight =
            m_resizeStartSize.height() -
            delta.y();


        if(newWidth < MinimumSize)
        {
            newWidth =
                MinimumSize;
        }


        if(newHeight < MinimumSize)
        {
            newHeight =
                MinimumSize;


            newTop =
                m_resizeStartPosition.y() +
                m_resizeStartSize.height() -
                MinimumSize;
        }


        newGeometry =
            QRect(
                m_resizeStartPosition.x(),
                newTop,
                newWidth,
                newHeight
                );

        break;
    }


        // --------------------------------------------------------
        // BOTTOM LEFT
        // --------------------------------------------------------

    case ResizeCorner::BottomLeft:
    {
        int newLeft =
            m_resizeStartPosition.x() +
            delta.x();


        int newWidth =
            m_resizeStartSize.width() -
            delta.x();


        int newHeight =
            m_resizeStartSize.height() +
            delta.y();


        if(newWidth < MinimumSize)
        {
            newWidth =
                MinimumSize;


            newLeft =
                m_resizeStartPosition.x() +
                m_resizeStartSize.width() -
                MinimumSize;
        }


        if(newHeight < MinimumSize)
        {
            newHeight =
                MinimumSize;
        }


        newGeometry =
            QRect(
                newLeft,
                m_resizeStartPosition.y(),
                newWidth,
                newHeight
                );

        break;
    }


        // --------------------------------------------------------
        // BOTTOM RIGHT
        // --------------------------------------------------------

    case ResizeCorner::BottomRight:
    {
        int newWidth =
            m_resizeStartSize.width() +
            delta.x();


        int newHeight =
            m_resizeStartSize.height() +
            delta.y();


        if(newWidth < MinimumSize)
        {
            newWidth =
                MinimumSize;
        }


        if(newHeight < MinimumSize)
        {
            newHeight =
                MinimumSize;
        }


        newGeometry =
            QRect(
                m_resizeStartPosition.x(),
                m_resizeStartPosition.y(),
                newWidth,
                newHeight
                );

        break;
    }


    case ResizeCorner::None:

        return;
    }


    setGeometry(
        newGeometry
        );


    emit sizeChanged(
        size()
        );


    update();
}


// ============================================================
// COOLDOWN
// ============================================================

void BuffBox::startCooldown()
{
    if(m_cooldown <= 0)
    {
        return;
    }


    m_glowTimer.stop();


    m_state =
        State::Active;


    update();


    m_timer.start(
        m_cooldown * 1000
        );
}


// ============================================================
// COOLDOWN FINISHED
// ============================================================

void BuffBox::cooldownFinished()
{
    m_state =
        State::Expired;


    m_glowAlpha =
        80;


    m_glowIncreasing =
        true;


    m_glowTimer.start();


    update();
}


// ============================================================
// RESET
// ============================================================

void BuffBox::reset()
{
    m_timer.stop();


    m_glowTimer.stop();


    m_glowAlpha =
        80;


    m_glowIncreasing =
        true;


    m_state =
        State::Ready;


    update();
}


// ============================================================
// KEY
// ============================================================

int BuffBox::key() const
{
    return m_key;
}


// ============================================================
// POSITION
// ============================================================

void BuffBox::setConfigurationPosition(
    const QPoint &position
    )
{
    move(
        position
        );
}


// ============================================================
// CONFIGURATION POSITION
// ============================================================

QPoint BuffBox::configurationPosition() const
{
    return pos();
}


// ============================================================
// CONFIGURATION MODE
// ============================================================

void BuffBox::setConfigurationMode(
    bool enabled
    )
{
    m_configurationMode =
        enabled;


    m_state =
        State::Ready;


    m_dragging =
        false;


    m_resizing =
        false;


    m_resizeCorner =
        ResizeCorner::None;


    if(!enabled)
    {
        unsetCursor();
    }


    update();
}


// ============================================================
// IS CONFIGURATION MODE
// ============================================================

bool BuffBox::isInConfigurationMode() const
{
    return m_configurationMode;
}


// ============================================================
// KEY NAME
// ============================================================

QString BuffBox::keyName(
    int key
    ) const
{
#ifdef Q_OS_WIN

    switch(key)
    {
    case VK_LCONTROL:
        return "L-Ctrl";


    case VK_RCONTROL:
        return "R-Ctrl";


    case VK_LSHIFT:
        return "L-Shift";


    case VK_RSHIFT:
        return "R-Shift";


    case VK_LMENU:
        return "L-Alt";


    case VK_RMENU:
        return "R-Alt";


    case VK_LWIN:
        return "L-Win";


    case VK_RWIN:
        return "R-Win";


    case VK_SPACE:
        return "Space";


    case VK_RETURN:
        return "Enter";


    case VK_ESCAPE:
        return "Esc";


    case VK_TAB:
        return "Tab";


    case VK_BACK:
        return "Back";


    case VK_UP:
        return "↑";


    case VK_DOWN:
        return "↓";


    case VK_LEFT:
        return "←";


    case VK_RIGHT:
        return "→";


    case VK_DELETE:
        return "Del";


    case VK_INSERT:
        return "Ins";


    case VK_HOME:
        return "Home";


    case VK_END:
        return "End";


    case VK_PRIOR:
        return "PgUp";


    case VK_NEXT:
        return "PgDn";


    case VK_F1:
        return "F1";


    case VK_F2:
        return "F2";


    case VK_F3:
        return "F3";


    case VK_F4:
        return "F4";


    case VK_F5:
        return "F5";


    case VK_F6:
        return "F6";


    case VK_F7:
        return "F7";


    case VK_F8:
        return "F8";


    case VK_F9:
        return "F9";


    case VK_F10:
        return "F10";


    case VK_F11:
        return "F11";


    case VK_F12:
        return "F12";


    case VK_F13:
        return "F13";


    case VK_F14:
        return "F14";


    case VK_F15:
        return "F15";


    case VK_F16:
        return "F16";


    case VK_F17:
        return "F17";


    case VK_F18:
        return "F18";


    case VK_F19:
        return "F19";


    case VK_F20:
        return "F20";


    case VK_F21:
        return "F21";


    case VK_F22:
        return "F22";


    case VK_F23:
        return "F23";


    case VK_F24:
        return "F24";


    default:
        break;
    }


    if(
        key >= 'A' &&
        key <= 'Z'
        )
    {
        return QString(
            QChar(key)
            );
    }


    if(
        key >= '0' &&
        key <= '9'
        )
    {
        return QString(
            QChar(key)
            );
    }

#endif


    return QString::number(
        key
        );
}
