
#include "specialcooldownoverlay.h"

#include "globalkeyboard.h"
#include "specialcooldownmanager.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QtMath>

#include <functional>


    namespace
{

    class SpecialCooldownWidget : public QWidget
    {
    public:

        enum class ResizeCorner
        {
            None,
            TopLeft,
            TopRight,
            BottomLeft,
            BottomRight
        };


        SpecialCooldownWidget(
            const SpecialCooldownConfiguration &configuration,
            QWidget *parent
            )
            : QWidget(parent),
            m_configuration(configuration)
        {
            setMouseTracking(
                true
                );

            setMinimumSize(
                30,
                30
                );

            setAttribute(
                Qt::WA_TranslucentBackground,
                true
                );

            setAutoFillBackground(
                false
                );

            m_pixmap =
                QPixmap(
                    m_configuration.imagePath
                    );
        }


        SpecialCooldownConfiguration configuration() const
        {
            return m_configuration;
        }


        void setConfiguration(
            const SpecialCooldownConfiguration &configuration
            )
        {
            m_configuration =
                configuration;

            m_pixmap =
                QPixmap(
                    m_configuration.imagePath
                    );

            update();
        }


        void setRemainingMilliseconds(
            int milliseconds
            )
        {
            m_remainingMilliseconds =
                qMax(
                    0,
                    milliseconds
                    );

            update();
        }


        void setActive(
            bool active
            )
        {
            m_active =
                active;

            update();
        }


        bool isActive() const
        {
            return m_active;
        }


        std::function<void(
            const QPoint &,
            const QSize &
            )> geometryChanged;


        std::function<void()> interactionStarted;


    protected:

        void paintEvent(
            QPaintEvent *
            ) override
        {
            QPainter painter(this);

            painter.setRenderHint(
                QPainter::Antialiasing,
                true
                );

            painter.setRenderHint(
                QPainter::SmoothPixmapTransform,
                true
                );


            const QRect imageRect =
                rect().adjusted(
                    3,
                    3,
                    -3,
                    -3
                    );


            if(!m_pixmap.isNull())
            {
                painter.drawPixmap(
                    imageRect,
                    m_pixmap
                    );
            }


            /*
         * Bordo.
         */

            painter.setPen(
                QPen(
                    Qt::white,
                    1
                    )
                );

            painter.drawRect(
                rect().adjusted(
                    0,
                    0,
                    -1,
                    -1
                    )
                );


            /*
         * Countdown.
         */

            if(m_active)
            {
                const double seconds =
                    static_cast<double>(
                        m_remainingMilliseconds
                        ) /
                    1000.0;


                const QString text =
                    QString::number(
                        seconds,
                        'f',
                        1
                        );


                QFont font =
                    painter.font();

                font.setBold(
                    true
                    );

                font.setPointSize(
                    qMax(
                        8,
                        width() / 5
                        )
                    );

                painter.setFont(
                    font
                    );

                painter.setPen(
                    Qt::white
                    );


                painter.drawText(
                    rect(),
                    Qt::AlignCenter,
                    text
                    );
            }


            /*
         * Quattro maniglie.
         */

            const int handleSize =
                6;


            painter.setBrush(
                Qt::white
                );

            painter.setPen(
                Qt::NoPen
                );


            painter.drawRect(
                0,
                0,
                handleSize,
                handleSize
                );


            painter.drawRect(
                width() - handleSize,
                0,
                handleSize,
                handleSize
                );


            painter.drawRect(
                0,
                height() - handleSize,
                handleSize,
                handleSize
                );


            painter.drawRect(
                width() - handleSize,
                height() - handleSize,
                handleSize,
                handleSize
                );
        }


        void mousePressEvent(
            QMouseEvent *event
            ) override
        {
            if(event->button() != Qt::LeftButton)
            {
                return;
            }


            m_resizeCorner =
                resizeCornerAt(
                    event->position().toPoint()
                    );


            if(m_resizeCorner != ResizeCorner::None)
            {
                m_resizing = true;

                m_resizeStartGlobal =
                    event->globalPosition().toPoint();

                m_resizeStartPosition =
                    pos();

                m_resizeStartSize =
                    size();


                if(interactionStarted)
                {
                    interactionStarted();
                }


                event->accept();

                return;
            }


            m_dragging = true;

            m_dragOffset =
                event->globalPosition().toPoint() -
                frameGeometry().topLeft();


            if(interactionStarted)
            {
                interactionStarted();
            }


            event->accept();
        }


        void mouseMoveEvent(
            QMouseEvent *event
            ) override
        {
            const QPoint position =
                event->position().toPoint();


            if(m_resizing)
            {
                resizeFromGlobalPosition(
                    event->globalPosition().toPoint()
                    );

                event->accept();

                return;
            }


            if(m_dragging)
            {
                const QPoint newGlobalPosition =
                    event->globalPosition().toPoint() -
                    m_dragOffset;


                if(parentWidget())
                {
                    const QPoint newPosition =
                        parentWidget()->mapFromGlobal(
                            newGlobalPosition
                            );


                    move(
                        newPosition
                        );
                }


                event->accept();

                return;
            }


            updateCursor(
                position
                );
        }


        void mouseReleaseEvent(
            QMouseEvent *event
            ) override
        {
            if(event->button() != Qt::LeftButton)
            {
                return;
            }


            m_dragging = false;
            m_resizing = false;

            m_resizeCorner =
                ResizeCorner::None;


            unsetCursor();


            notifyGeometryChanged();


            event->accept();
        }


        void leaveEvent(
            QEvent *event
            ) override
        {
            if(!m_dragging &&
                !m_resizing)
            {
                unsetCursor();
            }


            QWidget::leaveEvent(
                event
                );
        }


    private:

        SpecialCooldownConfiguration m_configuration;

        QPixmap m_pixmap;

        int m_remainingMilliseconds = 0;

        bool m_active = false;

        bool m_dragging = false;
        bool m_resizing = false;

        ResizeCorner m_resizeCorner =
            ResizeCorner::None;

        QPoint m_dragOffset;

        QPoint m_resizeStartGlobal;
        QPoint m_resizeStartPosition;

        QSize m_resizeStartSize;


        static constexpr int ResizeMargin = 10;
        static constexpr int MinimumSize = 30;


        ResizeCorner resizeCornerAt(
            const QPoint &position
            ) const
        {
            const bool left =
                position.x() <= ResizeMargin;

            const bool right =
                position.x() >= width() - ResizeMargin;

            const bool top =
                position.y() <= ResizeMargin;

            const bool bottom =
                position.y() >= height() - ResizeMargin;


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


        void updateCursor(
            const QPoint &position
            )
        {
            switch(
                resizeCornerAt(
                    position
                    )
                )
            {
            case ResizeCorner::TopLeft:
            case ResizeCorner::BottomRight:

                setCursor(
                    Qt::SizeFDiagCursor
                    );

                break;


            case ResizeCorner::TopRight:
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


        void resizeFromGlobalPosition(
            const QPoint &globalPosition
            )
        {
            const QPoint delta =
                globalPosition -
                m_resizeStartGlobal;


            const int deltaX =
                delta.x();

            const int deltaY =
                delta.y();


            int newSize =
                m_resizeStartSize.width();


            switch(m_resizeCorner)
            {
            case ResizeCorner::BottomRight:

                newSize =
                    m_resizeStartSize.width() +
                    qMax(
                        deltaX,
                        deltaY
                        );

                break;


            case ResizeCorner::BottomLeft:

                newSize =
                    m_resizeStartSize.width() +
                    qMax(
                        -deltaX,
                        deltaY
                        );

                break;


            case ResizeCorner::TopRight:

                newSize =
                    m_resizeStartSize.width() +
                    qMax(
                        deltaX,
                        -deltaY
                        );

                break;


            case ResizeCorner::TopLeft:

                newSize =
                    m_resizeStartSize.width() +
                    qMax(
                        -deltaX,
                        -deltaY
                        );

                break;


            case ResizeCorner::None:

                return;
            }


            newSize =
                qMax(
                    MinimumSize,
                    newSize
                    );


            QPoint newPosition =
                m_resizeStartPosition;


            switch(m_resizeCorner)
            {
            case ResizeCorner::TopLeft:

                newPosition.setX(
                    m_resizeStartPosition.x() +
                    m_resizeStartSize.width() -
                    newSize
                    );

                newPosition.setY(
                    m_resizeStartPosition.y() +
                    m_resizeStartSize.height() -
                    newSize
                    );

                break;


            case ResizeCorner::TopRight:

                newPosition.setY(
                    m_resizeStartPosition.y() +
                    m_resizeStartSize.height() -
                    newSize
                    );

                break;


            case ResizeCorner::BottomLeft:

                newPosition.setX(
                    m_resizeStartPosition.x() +
                    m_resizeStartSize.width() -
                    newSize
                    );

                break;


            case ResizeCorner::BottomRight:
            case ResizeCorner::None:

                break;
            }


            move(
                newPosition
                );

            resize(
                newSize,
                newSize
                );
        }


        void notifyGeometryChanged()
        {
            if(geometryChanged)
            {
                geometryChanged(
                    pos(),
                    size()
                    );
            }
        }
    };

}


// ============================================================
// CONSTRUCTOR
// ============================================================

SpecialCooldownOverlay::SpecialCooldownOverlay(
    SpecialCooldownManager *manager,
    GlobalKeyboard *keyboard,
    QWidget *parent
    )
    : QWidget(parent),
    m_manager(manager),
    m_keyboard(keyboard)
{
    setAttribute(
        Qt::WA_TranslucentBackground,
        true
        );

    setAttribute(
        Qt::WA_NoSystemBackground,
        true
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setMouseTracking(
        true
        );


    m_timer.setInterval(
        100
        );


    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        &SpecialCooldownOverlay::updateCooldowns
        );


    if(m_keyboard)
    {
        connect(
            m_keyboard,
            &GlobalKeyboard::keyPressed,
            this,
            &SpecialCooldownOverlay::activateKey,
            Qt::QueuedConnection
            );


        connect(
            m_keyboard,
            &GlobalKeyboard::resetPressed,
            this,
            &SpecialCooldownOverlay::resetAll,
            Qt::QueuedConnection
            );
    }


    loadConfigurations();

    createWidgets();


    hide();
}


// ============================================================
// LOAD
// ============================================================

void SpecialCooldownOverlay::loadConfigurations()
{
    m_states.clear();


    if(!m_manager)
    {
        return;
    }


    const QList<SpecialCooldownConfiguration> configurations =
        m_manager->configurations();


    for(
        const SpecialCooldownConfiguration &configuration :
        configurations
        )
    {
        CooldownState state;

        state.configuration =
            configuration;

        state.remainingMilliseconds =
            0;

        state.active =
            false;


        m_states.append(
            state
            );
    }
}


// ============================================================
// CREATE WIDGETS
// ============================================================

void SpecialCooldownOverlay::createWidgets()
{
    clearWidgets();


    for(
        int i = 0;
        i < m_states.size();
        ++i
        )
    {
        SpecialCooldownWidget *widget =
            new SpecialCooldownWidget(
                m_states[i].configuration,
                this
                );


        widget->resize(
            m_states[i].configuration.size
            );


        widget->move(
            m_states[i].configuration.position
            );


        widget->geometryChanged =
            [this, i](
                const QPoint &position,
                const QSize &size
                )
        {
            if(i < 0 ||
                i >= m_states.size())
            {
                return;
            }


            m_states[i].configuration.position =
                position;


            m_states[i].configuration.size =
                size;


            if(m_manager)
            {
                QList<SpecialCooldownConfiguration> configurations;


                for(
                    const CooldownState &state :
                    m_states
                    )
                {
                    configurations.append(
                        state.configuration
                        );
                }


                m_manager->setConfigurations(
                    configurations
                    );

                m_manager->save();
            }
        };


        widget->interactionStarted =
            [this]()
        {
            if(!m_enabled)
            {
                return;
            }
        };


        widget->show();
    }
}


// ============================================================
// CLEAR WIDGETS
// ============================================================

void SpecialCooldownOverlay::clearWidgets()
{
    const QList<QWidget *> children =
        findChildren<QWidget *>(
            QString(),
            Qt::FindDirectChildrenOnly
            );


    for(
        QWidget *widget :
        children
        )
    {
        widget->deleteLater();
    }
}


// ============================================================
// ENABLED
// ============================================================

void SpecialCooldownOverlay::setEnabled(
    bool enabled
    )
{
    m_enabled =
        enabled;


    if(enabled)
    {
        loadConfigurations();

        createWidgets();


        if(parentWidget())
        {
            resize(
                parentWidget()->size()
                );
        }


        show();
        raise();


        if(!m_timer.isActive())
        {
            m_timer.start();
        }
    }
    else
    {
        m_timer.stop();

        hide();
    }
}


// ============================================================
// RESET
// ============================================================

void SpecialCooldownOverlay::resetAll()
{
    for(
        int i = 0;
        i < m_states.size();
        ++i
        )
    {
        m_states[i].remainingMilliseconds =
            0;

        m_states[i].active =
            false;


        updateWidget(
            i
            );
    }
}


// ============================================================
// ACTIVATE KEY
// ============================================================

void SpecialCooldownOverlay::activateKey(
    int key
    )
{
    if(!m_enabled)
    {
        return;
    }


    for(
        int i = 0;
        i < m_states.size();
        ++i
        )
    {
        if(
            m_states[i].configuration.key != key
            )
        {
            continue;
        }


        m_states[i].remainingMilliseconds =
            m_states[i].configuration.cooldown *
            1000;


        m_states[i].active =
            true;


        updateWidget(
            i
            );
    }
}


// ============================================================
// UPDATE COOLDOWNS
// ============================================================

void SpecialCooldownOverlay::updateCooldowns()
{
    if(!m_enabled)
    {
        return;
    }


    for(
        int i = 0;
        i < m_states.size();
        ++i
        )
    {
        if(!m_states[i].active)
        {
            continue;
        }


        m_states[i].remainingMilliseconds -=
            100;


        if(
            m_states[i].remainingMilliseconds <= 0
            )
        {
            m_states[i].remainingMilliseconds =
                0;

            m_states[i].active =
                false;
        }


        updateWidget(
            i
            );
    }
}


// ============================================================
// UPDATE WIDGET
// ============================================================

void SpecialCooldownOverlay::updateWidget(
    int index
    )
{
    const QList<QWidget *> children =
        findChildren<QWidget *>(
            QString(),
            Qt::FindDirectChildrenOnly
            );


    if(
        index < 0 ||
        index >= children.size() ||
        index >= m_states.size()
        )
    {
        return;
    }


    SpecialCooldownWidget *widget =
        dynamic_cast<SpecialCooldownWidget *>(
            children.at(index)
            );


    if(!widget)
    {
        return;
    }


    widget->setRemainingMilliseconds(
        m_states[index].remainingMilliseconds
        );


    widget->setActive(
        m_states[index].active
        );
}


// ============================================================
// SAVE POSITION / SIZE
// ============================================================

void SpecialCooldownOverlay::savePositionsAndSizes()
{
    if(!m_manager)
    {
        return;
    }


    QList<SpecialCooldownConfiguration> configurations;


    const QList<QWidget *> children =
        findChildren<QWidget *>(
            QString(),
            Qt::FindDirectChildrenOnly
            );


    for(
        int i = 0;
        i < m_states.size() &&
        i < children.size();
        ++i
        )
    {
        SpecialCooldownWidget *widget =
            dynamic_cast<SpecialCooldownWidget *>(
                children.at(i)
                );


        if(!widget)
        {
            continue;
        }


        m_states[i].configuration.position =
            widget->pos();


        m_states[i].configuration.size =
            widget->size();


        configurations.append(
            m_states[i].configuration
            );
    }


    m_manager->setConfigurations(
        configurations
        );

    m_manager->save();
}


// ============================================================
// RESIZE EVENT
// ============================================================

void SpecialCooldownOverlay::resizeEvent(
    QResizeEvent *event
    )
{
    QWidget::resizeEvent(
        event
        );
}


// ============================================================
// PAINT
// ============================================================

void SpecialCooldownOverlay::paintEvent(
    QPaintEvent *
    )
{
    /*
     * Overlay trasparente.
     *
     * I singoli SpecialCooldownWidget
     * disegnano direttamente le proprie immagini.
     */
}


// ============================================================
// KEY NAME
// ============================================================

QString SpecialCooldownOverlay::keyName(
    int key
    ) const
{
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


    if(
        key >= 112 &&
        key <= 123
        )
    {
        return QString(
                   "F%1"
                   ).arg(
                key - 111
                );
    }


    switch(key)
    {
    case 162:
        return "Left Ctrl";

    case 163:
        return "Right Ctrl";

    case 160:
        return "Left Shift";

    case 161:
        return "Right Shift";

    case 164:
        return "Left Alt";

    case 165:
        return "Right Alt";

    case 91:
        return "Left Win";

    case 92:
        return "Right Win";

    case 32:
        return "Space";

    case 9:
        return "Tab";

    case 13:
        return "Enter";

    case 27:
        return "Esc";

    case 37:
        return "Left";

    case 38:
        return "Up";

    case 39:
        return "Right";

    case 40:
        return "Down";

    default:
        return QString(
                   "VK %1"
                   ).arg(key);
    }
}
