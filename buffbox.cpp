#include "buffbox.h"

#include <QPainter>
#include <QMouseEvent>


BuffBox::BuffBox(
    QChar key,
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

        painter.setBrush(Qt::NoBrush);

        painter.drawRect(
            1,
            1,
            width() - 2,
            height() - 2
            );

        painter.setPen(Qt::white);

        painter.setFont(
            QFont(
                "Arial",
                32,
                QFont::Bold
                )
            );

        painter.drawText(
            rect(),
            Qt::AlignCenter,
            m_key
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



QChar BuffBox::key() const
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

void BuffBox::setConfigurationMode(bool enabled)
{
    m_configurationMode = enabled;

    m_state = State::Ready;

    update();
}

bool BuffBox::isInConfigurationMode() const
{
    return m_configurationMode;
}