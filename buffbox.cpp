#include "buffbox.h"

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

    setFixedSize(
        42,
        42
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    m_timer.setSingleShot(
        true
        );


    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        &BuffBox::cooldownFinished
        );


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
                    m_glowIncreasing = false;

            }
            else
            {

                m_glowAlpha -= 8;


                if(m_glowAlpha <= 80)
                    m_glowIncreasing = true;

            }


            update();

        }
        );

}



void BuffBox::paintEvent(
    QPaintEvent *
    )
{

    QPainter painter(this);


    painter.setRenderHint(
        QPainter::Antialiasing
        );



    // =========================
    // EXPIRED GLOW
    // =========================

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



    // =========================
    // BOX
    // =========================

    if(m_configurationMode)
    {

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

    }

}



void BuffBox::mousePressEvent(
    QMouseEvent *event
    )
{

    if(event->button() != Qt::LeftButton)
        return;


    m_dragging = true;


    m_dragOffset =
        event->pos();


    event->accept();

}



void BuffBox::mouseMoveEvent(
    QMouseEvent *event
    )
{

    if(!m_dragging)
        return;


    move(
        event->globalPosition().toPoint()
        - m_dragOffset
        );


    emit positionChanged(
        pos()
        );


    event->accept();

}



void BuffBox::mouseReleaseEvent(
    QMouseEvent *event
    )
{

    if(event->button() != Qt::LeftButton)
        return;


    m_dragging = false;


    emit positionChanged(
        pos()
        );


    event->accept();

}



void BuffBox::startCooldown()
{

    if(m_cooldown <= 0)
        return;


    m_glowTimer.stop();


    m_state =
        State::Active;


    update();


    m_timer.start(
        m_cooldown * 1000
        );

}



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



int BuffBox::key() const
{
    return m_key;
}



void BuffBox::setConfigurationPosition(
    const QPoint &position
    )
{

    move(
        position
        );

}



QPoint BuffBox::configurationPosition() const
{
    return pos();
}



void BuffBox::setConfigurationMode(
    bool enabled
    )
{

    m_configurationMode =
        enabled;


    m_state =
        State::Ready;


    update();

}



bool BuffBox::isInConfigurationMode() const
{
    return m_configurationMode;
}



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


    if(key >= 'A' && key <= 'Z')
        return QString(QChar(key));


    if(key >= '0' && key <= '9')
        return QString(QChar(key));


#endif


    return QString::number(key);
}
