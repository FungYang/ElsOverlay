#include "customsearcheroverlay.h"

#include <QDateTime>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>

CustomSearcherOverlay::CustomSearcherOverlay(
    int templateId,
    const QImage &displayImage,
    QWidget *parent
    )
    : QWidget(parent),
    m_templateId(templateId),
    m_displayImage(
        displayImage.convertToFormat(
            QImage::Format_ARGB32
            )
        )
{
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint |
        Qt::WindowDoesNotAcceptFocus
        );

    setAttribute(Qt::WA_TranslucentBackground);

    /*
     * IMPORTANTE:
     * NON usiamo WA_TransparentForMouseEvents.
     *
     * Il box deve poter ricevere il mouse per essere
     * spostato e ridimensionato.
     */

    setMouseTracking(true);

    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(50);

    connect(
        m_countdownTimer,
        &QTimer::timeout,
        this,
        &CustomSearcherOverlay::updateCountdown
        );

    if (!m_displayImage.isNull())
    {
        m_grayImage =
            m_displayImage.convertToFormat(
                QImage::Format_ARGB32
                );

        for (int y = 0;
             y < m_grayImage.height();
             ++y)
        {
            QRgb *line =
                reinterpret_cast<QRgb *>(
                    m_grayImage.scanLine(y)
                    );

            for (int x = 0;
                 x < m_grayImage.width();
                 ++x)
            {
                const QRgb pixel = line[x];

                const int gray = qGray(pixel);

                line[x] = qRgba(
                    gray,
                    gray,
                    gray,
                    qAlpha(pixel)
                    );
            }
        }

        resize(m_displayImage.size());
    }

    hide();
}

int CustomSearcherOverlay::templateId() const
{
    return m_templateId;
}

void CustomSearcherOverlay::setScreenRect(const QRect &rect)
{
    if (!rect.isValid())
        return;

    m_screenRect = rect;

    setGeometry(rect);
}

QRect CustomSearcherOverlay::screenRect() const
{
    return m_screenRect;
}

void CustomSearcherOverlay::setVisibleBySearcher(bool visible)
{
    m_visibleBySearcher = visible;

    if (visible)
        show();
    else
        hide();
}

void CustomSearcherOverlay::startCooldown(int cooldownMs)
{
    if (cooldownMs <= 0)
    {
        stopCooldown();
        return;
    }

    m_coolingDown = true;

    m_cooldownEndMs =
        QDateTime::currentMSecsSinceEpoch()
        + cooldownMs;

    m_lastDisplayedSecond = -1;

    update();

    m_countdownTimer->start();
}

void CustomSearcherOverlay::stopCooldown()
{
    m_coolingDown = false;
    m_cooldownEndMs = 0;
    m_lastDisplayedSecond = -1;

    m_countdownTimer->stop();

    update();
}

bool CustomSearcherOverlay::isCoolingDown() const
{
    return m_coolingDown;
}

CustomSearcherOverlay::Interaction
CustomSearcherOverlay::hitTest(const QPoint &pos) const
{
    const QRect r = rect();

    const int hs = HANDLE_SIZE;

    const QRect topLeft(
        0,
        0,
        hs,
        hs
        );

    const QRect topRight(
        r.width() - hs,
        0,
        hs,
        hs
        );

    const QRect bottomLeft(
        0,
        r.height() - hs,
        hs,
        hs
        );

    const QRect bottomRight(
        r.width() - hs,
        r.height() - hs,
        hs,
        hs
        );

    if (topLeft.contains(pos))
        return Interaction::ResizeTopLeft;

    if (topRight.contains(pos))
        return Interaction::ResizeTopRight;

    if (bottomLeft.contains(pos))
        return Interaction::ResizeBottomLeft;

    if (bottomRight.contains(pos))
        return Interaction::ResizeBottomRight;

    const int edge = hs / 2;

    if (pos.y() <= edge)
        return Interaction::ResizeTop;

    if (pos.y() >= r.height() - edge)
        return Interaction::ResizeBottom;

    if (pos.x() <= edge)
        return Interaction::ResizeLeft;

    if (pos.x() >= r.width() - edge)
        return Interaction::ResizeRight;

    return Interaction::Move;
}

void CustomSearcherOverlay::updateCursor(
    Interaction interaction)
{
    switch (interaction)
    {
    case Interaction::ResizeTopLeft:
    case Interaction::ResizeBottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;

    case Interaction::ResizeTopRight:
    case Interaction::ResizeBottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;

    case Interaction::ResizeTop:
    case Interaction::ResizeBottom:
        setCursor(Qt::SizeVerCursor);
        break;

    case Interaction::ResizeLeft:
    case Interaction::ResizeRight:
        setCursor(Qt::SizeHorCursor);
        break;

    case Interaction::Move:
        setCursor(Qt::SizeAllCursor);
        break;

    case Interaction::None:
    default:
        unsetCursor();
        break;
    }
}

void CustomSearcherOverlay::applyResize(
    Interaction interaction,
    const QPoint &pos)
{
    QRect rect = m_rectAtDragStart;

    const QPoint delta =
        pos - m_dragStartGlobal;

    switch (interaction)
    {
    case Interaction::ResizeTopLeft:
        rect.setTopLeft(
            m_rectAtDragStart.topLeft()
            + delta
            );
        break;

    case Interaction::ResizeTopRight:
        rect.setTopRight(
            m_rectAtDragStart.topRight()
            + QPoint(delta.x(), delta.y())
            );
        break;

    case Interaction::ResizeBottomLeft:
        rect.setBottomLeft(
            m_rectAtDragStart.bottomLeft()
            + QPoint(delta.x(), delta.y())
            );
        break;

    case Interaction::ResizeBottomRight:
        rect.setBottomRight(
            m_rectAtDragStart.bottomRight()
            + delta
            );
        break;

    case Interaction::ResizeTop:
        rect.setTop(
            m_rectAtDragStart.top()
            + delta.y()
            );
        break;

    case Interaction::ResizeBottom:
        rect.setBottom(
            m_rectAtDragStart.bottom()
            + delta.y()
            );
        break;

    case Interaction::ResizeLeft:
        rect.setLeft(
            m_rectAtDragStart.left()
            + delta.x()
            );
        break;

    case Interaction::ResizeRight:
        rect.setRight(
            m_rectAtDragStart.right()
            + delta.x()
            );
        break;

    default:
        return;
    }

    /*
     * Manteniamo sempre una dimensione minima.
     */

    if (rect.width() < MIN_WIDTH)
    {
        if (interaction == Interaction::ResizeLeft ||
            interaction == Interaction::ResizeTopLeft ||
            interaction == Interaction::ResizeBottomLeft)
        {
            rect.setLeft(
                rect.right() - MIN_WIDTH + 1
                );
        }
        else
        {
            rect.setWidth(MIN_WIDTH);
        }
    }

    if (rect.height() < MIN_HEIGHT)
    {
        if (interaction == Interaction::ResizeTop ||
            interaction == Interaction::ResizeTopLeft ||
            interaction == Interaction::ResizeTopRight)
        {
            rect.setTop(
                rect.bottom() - MIN_HEIGHT + 1
                );
        }
        else
        {
            rect.setHeight(MIN_HEIGHT);
        }
    }

    m_screenRect = rect;

    setGeometry(m_screenRect);

    update();
}

void CustomSearcherOverlay::mousePressEvent(
    QMouseEvent *event)
{
    if (!m_visibleBySearcher)
        return;

    if (event->button() != Qt::LeftButton)
        return;

    m_interaction = hitTest(
        event->position().toPoint()
        );

    if (m_interaction == Interaction::None)
        return;

    m_dragging = true;

    m_dragStartGlobal =
        event->globalPosition().toPoint();

    m_rectAtDragStart = m_screenRect;

    event->accept();
}

void CustomSearcherOverlay::mouseMoveEvent(
    QMouseEvent *event)
{
    if (!m_visibleBySearcher)
        return;

    const QPoint localPos =
        event->position().toPoint();

    if (!m_dragging)
    {
        updateCursor(hitTest(localPos));
        return;
    }

    const QPoint globalPos =
        event->globalPosition().toPoint();

    if (m_interaction == Interaction::Move)
    {
        const QPoint delta =
            globalPos - m_dragStartGlobal;

        m_screenRect =
            m_rectAtDragStart.translated(delta);

        setGeometry(m_screenRect);
    }
    else
    {
        applyResize(
            m_interaction,
            globalPos
            );
    }

    event->accept();
}

void CustomSearcherOverlay::mouseReleaseEvent(
    QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (!m_dragging)
        return;

    m_dragging = false;

    const Interaction finishedInteraction =
        m_interaction;

    m_interaction = Interaction::None;

    updateCursor(
        hitTest(
            event->position().toPoint()
            )
        );

    /*
     * Salviamo SOLO quando l'utente ha finito.
     *
     * Quindi durante il drag non viene fatto nessun
     * accesso a QSettings / disco.
     */
    if (finishedInteraction != Interaction::None)
    {
        emit geometryChangeFinished(
            m_templateId,
            m_screenRect
            );
    }

    event->accept();
}

void CustomSearcherOverlay::updateCountdown()
{
    if (!m_coolingDown)
        return;

    const qint64 now =
        QDateTime::currentMSecsSinceEpoch();

    const qint64 remainingMs =
        m_cooldownEndMs - now;

    if (remainingMs <= 0)
    {
        m_coolingDown = false;
        m_cooldownEndMs = 0;
        m_lastDisplayedSecond = -1;

        m_countdownTimer->stop();

        update();

        emit cooldownFinished(m_templateId);

        return;
    }

    const int remainingSecond =
        static_cast<int>(
            (remainingMs + 999) / 1000
            );

    if (remainingSecond != m_lastDisplayedSecond)
    {
        m_lastDisplayedSecond =
            remainingSecond;

        update();
    }
}

void CustomSearcherOverlay::paintEvent(
    QPaintEvent *event)
{
    Q_UNUSED(event);

    if (m_displayImage.isNull())
        return;


    QPainter painter(this);

    painter.setRenderHint(
        QPainter::SmoothPixmapTransform,
        false
        );


    const QImage &image =
        m_coolingDown
            ? m_grayImage
            : m_displayImage;


    /*
     * Applichiamo la trasparenza globale all'immagine.
     *
     * 255 = completamente visibile
     * 0   = completamente trasparente
     */
    painter.setOpacity(
        static_cast<qreal>(m_transparency) / 255.0
        );


    /*
     * L'immagine viene scalata al box.
     */
    painter.drawImage(
        rect(),
        image
        );


    /*
     * Ripristiniamo l'opacità per gli elementi grafici
     * sovrapposti.
     */
    painter.setOpacity(1.0);


    /*
     * Bordo leggero per rendere evidente che il box
     * è interattivo.
     *
     * Anche il bordo segue la trasparenza globale.
     */
    painter.setPen(
        QPen(
            QColor(
                255,
                255,
                255,
                m_transparency * 100 / 255
                ),
            1
            )
        );


    painter.drawRect(
        rect().adjusted(0, 0, -1, -1)
        );


    /*
     * Countdown.
     */
    if (!m_coolingDown)
        return;


    const qint64 now =
        QDateTime::currentMSecsSinceEpoch();


    const qint64 remainingMs =
        m_cooldownEndMs - now;


    if (remainingMs <= 0)
        return;


    const int remainingSecond =
        static_cast<int>(
            (remainingMs + 999) / 1000
            );


    const QColor baseColor =
        remainingSecond <= 3
            ? Qt::red
            : Qt::white;


    const QColor numberColor(
        baseColor.red(),
        baseColor.green(),
        baseColor.blue(),
        m_transparency
        );


    painter.setPen(
        numberColor
        );


    QFont font = painter.font();

    font.setBold(true);


    const int fontSize =
        qMax(
            12,
            qMin(width(), height()) / 2
            );


    font.setPixelSize(fontSize);

    painter.setFont(font);


    painter.drawText(
        rect(),
        Qt::AlignCenter,
        QString::number(remainingSecond)
        );
}
void CustomSearcherOverlay::setTransparency(
    int value
    )
{
    m_transparency =
        qBound(
            0,
            value,
            255
            );


    update();
}