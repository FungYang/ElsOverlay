#include "buffbox.h"

#include <QPainter>
#include <QMouseEvent>


BuffBox::BuffBox(
    QChar key,
    QVector<int> cooldowns,
    QString className,
    QString boxId,
    QWidget *parent
    )
    :
    QWidget(parent),
    m_key(key),
    m_cooldowns(cooldowns),
    m_currentState(0),
    m_configured(false),
    m_dragging(false),
    m_remainingTime(0),
    m_glowAlpha(80),
    m_glowIncreasing(true),
    className(className),
    boxId(boxId)
{

    m_glowTimer.setInterval(30);
    setFixedSize(42,42);
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

    setAttribute(
        Qt::WA_TranslucentBackground
        );

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    m_timer.setSingleShot(true);


    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        &BuffBox::cooldownFinished
        );
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    move(
        settings.value(
                    "Classes/" + className + "/" + boxId + "/position",
                    QPoint(100,100)
                    ).toPoint()
        );


}



void BuffBox::paintEvent(QPaintEvent *)
{

    QPainter painter(this);

    painter.setRenderHint(
        QPainter::Antialiasing
        );


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



    if(!m_configured)
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
            width()-2,
            height()-2
            );


        painter.setPen(
            Qt::white
            );


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
    // DEBUG TEMPORANEO
    //drawDebugCountdown(painter);

}



void BuffBox::mousePressEvent(
    QMouseEvent *event
    )
{

    if(event->button() == Qt::LeftButton)
    {

        m_dragging = true;


        m_dragOffset =
            event->pos();


        event->accept();

    }

}



void BuffBox::mouseMoveEvent(
    QMouseEvent *event
    )
{

    if(m_dragging)
    {

        move(
            event->globalPosition().toPoint()
            - m_dragOffset
            );


        QSettings settings(
            "ElsOverlay.ini",
            QSettings::IniFormat
            );


        settings.setValue(
            "Classes/" + className + "/" + boxId + "/position",
            pos()
            );

        // qui NON cambiamo lo stato

    }

}


void BuffBox::mouseReleaseEvent(
    QMouseEvent *event
    )
{

    Q_UNUSED(event);


    if(m_dragging)
    {

        if(!m_configured)
        {

            m_configured = true;

            m_state = State::Ready;

            update();

        }

    }


    m_dragging = false;

}



void BuffBox::startCooldown()
{

    if(!m_configured)
        return;

    m_glowTimer.stop();
    m_state = State::Active;


    m_remainingTime =
        m_cooldowns[m_currentState];


    update();


    m_timer.start(
        m_cooldowns[m_currentState] * 1000
        );


    if(m_currentState < m_cooldowns.size()-1)
    {
        m_currentState++;
    }

}
void BuffBox::cooldownFinished()
{

    m_state = State::Expired;

    m_currentState = 0;

    m_glowAlpha = 80;
    m_glowIncreasing = true;

    m_glowTimer.start();

    update();

}



void BuffBox::reset()
{
    m_timer.stop();

    if(m_configured)
        m_state = State::Ready;
    else
        m_state = State::Placement;

    update();
}



QChar BuffBox::key() const
{
    return m_key;
}

void BuffBox::drawDebugCountdown(
    QPainter& painter
    )
{

    if(m_state != State::Active)
        return;


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
        QString::number(m_remainingTime)
        );

}
bool BuffBox::hasSavedPosition() const
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    return settings.contains(
        "Classes/" +
        className +
        "/" +
        QString(m_key) +
        "/position"
        );
}

void BuffBox::confirmPlacement()
{
    m_configured = true;

    if(m_state == State::Placement)
    {
        m_state = State::Ready;
    }

    update();
}