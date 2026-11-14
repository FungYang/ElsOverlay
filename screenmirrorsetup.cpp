#include "screenmirrorsetup.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>


ScreenMirrorSetup::ScreenMirrorSetup(
    QWidget *parent
    )
    : QWidget(parent)
{
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(
        Qt::WA_TranslucentBackground
        );

    setMouseTracking(
        true
        );

    QScreen *screen =
        QGuiApplication::primaryScreen();

    setGeometry(
        screen
            ? screen->geometry()
            : QRect(0, 0, 1920, 1080)
        );

    // Valori iniziali (verranno sostituiti da setRects()).
    m_source =
        QRect(100, 100, 300, 200);

    m_dest =
        QRect(500, 100, 300, 200);
}


QRect ScreenMirrorSetup::sourceRect() const
{
    return m_source;
}


QRect ScreenMirrorSetup::destRect() const
{
    return m_dest;
}


void ScreenMirrorSetup::setRects(
    const QRect &source,
    const QRect &dest
    )
{
    m_source =
        sanitized(source);

    m_dest =
        sanitized(dest);

    update();
}


void ScreenMirrorSetup::showFeedback(
    const QString &text
    )
{
    m_feedback =
        text;

    update();
}


// ============================================================
// HELPERS
// ============================================================

QRect &ScreenMirrorSetup::rectOf(
    Box box
    )
{
    return (box == Box::Dest)
               ? m_dest
               : m_source;
}


const QRect &ScreenMirrorSetup::rectOf(
    Box box
    ) const
{
    return (box == Box::Dest)
               ? m_dest
               : m_source;
}


QRect ScreenMirrorSetup::sanitized(
    const QRect &rect
    ) const
{
    QRect r =
        rect.normalized();

    if(r.width() < MIN_SIZE)
        r.setWidth(MIN_SIZE);

    if(r.height() < MIN_SIZE)
        r.setHeight(MIN_SIZE);

    // Dentro lo schermo: una regione fuori dallo schermo non si
    // potrebbe ne' catturare ne' mostrare.
    const QRect screenRect =
        QRect(
            QPoint(0, 0),
            size()
            );

    if(r.width() > screenRect.width())
        r.setWidth(screenRect.width());

    if(r.height() > screenRect.height())
        r.setHeight(screenRect.height());

    if(r.left() < screenRect.left())
        r.moveLeft(screenRect.left());

    if(r.top() < screenRect.top())
        r.moveTop(screenRect.top());

    if(r.right() > screenRect.right())
        r.moveRight(screenRect.right());

    if(r.bottom() > screenRect.bottom())
        r.moveBottom(screenRect.bottom());

    return r;
}


ScreenMirrorSetup::Handle ScreenMirrorSetup::handleAt(
    const QRect &r,
    const QPoint &pos
    ) const
{
    const int h = HANDLE_SIZE;

    const QRect topLeft(r.left() - h / 2, r.top() - h / 2, h, h);
    const QRect topRight(r.right() - h / 2, r.top() - h / 2, h, h);
    const QRect bottomLeft(r.left() - h / 2, r.bottom() - h / 2, h, h);
    const QRect bottomRight(r.right() - h / 2, r.bottom() - h / 2, h, h);

    const QRect top(r.left() + h, r.top() - h / 2, r.width() - 2 * h, h);
    const QRect bottom(r.left() + h, r.bottom() - h / 2, r.width() - 2 * h, h);
    const QRect left(r.left() - h / 2, r.top() + h, h, r.height() - 2 * h);
    const QRect right(r.right() - h / 2, r.top() + h, h, r.height() - 2 * h);

    if(topLeft.contains(pos))     return Handle::TopLeft;
    if(topRight.contains(pos))    return Handle::TopRight;
    if(bottomLeft.contains(pos))  return Handle::BottomLeft;
    if(bottomRight.contains(pos)) return Handle::BottomRight;
    if(top.contains(pos))         return Handle::Top;
    if(bottom.contains(pos))      return Handle::Bottom;
    if(left.contains(pos))        return Handle::Left;
    if(right.contains(pos))       return Handle::Right;

    return Handle::None;
}


ScreenMirrorSetup::Hit ScreenMirrorSetup::hitTest(
    const QPoint &pos
    ) const
{
    Hit hit;

    // Prima le maniglie di entrambi (la destinazione e' disegnata
    // sopra, quindi ha la precedenza), cosi' una maniglia coperta
    // dall'interno dell'altro rettangolo resta raggiungibile...
    for(Box box : { Box::Dest, Box::Source })
    {
        const Handle handle =
            handleAt(
                rectOf(box),
                pos
                );

        if(handle != Handle::None)
        {
            hit.box = box;
            hit.handle = handle;
            return hit;
        }
    }

    // ...poi l'interno (spostamento).
    for(Box box : { Box::Dest, Box::Source })
    {
        if(rectOf(box).contains(pos))
        {
            hit.box = box;
            hit.handle = Handle::Move;
            return hit;
        }
    }

    return hit;
}


void ScreenMirrorSetup::updateCursor(
    Handle handle
    )
{
    switch(handle)
    {
    case Handle::TopLeft:
    case Handle::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;

    case Handle::TopRight:
    case Handle::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;

    case Handle::Top:
    case Handle::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;

    case Handle::Left:
    case Handle::Right:
        setCursor(Qt::SizeHorCursor);
        break;

    case Handle::Move:
        setCursor(Qt::SizeAllCursor);
        break;

    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}


void ScreenMirrorSetup::applyResize(
    Handle handle,
    const QPoint &pos
    )
{
    QRect r =
        m_rectAtDragStart;

    const QPoint delta =
        pos - m_dragStart;

    switch(handle)
    {
    case Handle::TopLeft:
        r.setTopLeft(r.topLeft() + delta);
        break;

    case Handle::TopRight:
        r.setTopRight(r.topRight() + delta);
        break;

    case Handle::BottomLeft:
        r.setBottomLeft(r.bottomLeft() + delta);
        break;

    case Handle::BottomRight:
        r.setBottomRight(r.bottomRight() + delta);
        break;

    case Handle::Top:
        r.setTop(r.top() + delta.y());
        break;

    case Handle::Bottom:
        r.setBottom(r.bottom() + delta.y());
        break;

    case Handle::Left:
        r.setLeft(r.left() + delta.x());
        break;

    case Handle::Right:
        r.setRight(r.right() + delta.x());
        break;

    default:
        break;
    }

    rectOf(m_activeBox) =
        r;
}


// ============================================================
// PAINT
// ============================================================

void ScreenMirrorSetup::paintBox(
    QPainter &painter,
    const QRect &rect,
    const QColor &color,
    const QString &label
    )
{
    painter.setPen(QPen(color, 2));
    painter.setBrush(QColor(color.red(), color.green(), color.blue(), 28));
    painter.drawRect(rect);

    // Maniglie.
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    auto drawHandle =
        [&painter](const QPoint &center)
    {
        painter.drawRect(
            QRect(
                center.x() - HANDLE_SIZE / 2,
                center.y() - HANDLE_SIZE / 2,
                HANDLE_SIZE,
                HANDLE_SIZE
                )
            );
    };

    drawHandle(rect.topLeft());
    drawHandle(rect.topRight());
    drawHandle(rect.bottomLeft());
    drawHandle(rect.bottomRight());
    drawHandle(QPoint(rect.center().x(), rect.top()));
    drawHandle(QPoint(rect.center().x(), rect.bottom()));
    drawHandle(QPoint(rect.left(), rect.center().y()));
    drawHandle(QPoint(rect.right(), rect.center().y()));

    // Etichetta con le dimensioni.
    painter.setPen(color);

    QFont font =
        painter.font();

    font.setPointSize(10);
    font.setBold(true);

    painter.setFont(font);

    painter.drawText(
        rect.adjusted(6, 4, -6, -4),
        Qt::AlignLeft | Qt::AlignTop,
        QString("%1  %2x%3")
            .arg(label)
            .arg(rect.width())
            .arg(rect.height())
        );
}


void ScreenMirrorSetup::paintEvent(
    QPaintEvent *
    )
{
    QPainter painter(
        this
        );

    painter.setRenderHint(
        QPainter::Antialiasing
        );

    // La sorgente sotto, la destinazione sopra.
    paintBox(
        painter,
        m_source,
        QColor(0, 200, 255),
        "CATTURA"
        );

    paintBox(
        painter,
        m_dest,
        QColor(60, 220, 90),
        "OVERLAY"
        );

    // Istruzioni.
    painter.setPen(Qt::white);

    QFont font =
        painter.font();

    font.setPointSize(11);

    painter.setFont(font);

    painter.drawText(
        QRect(20, 20, 760, 90),
        Qt::AlignLeft | Qt::TextWordWrap,
        "Screen Mirror\n"
        "Azzurro = regione CATTURATA (ogni secondo)    "
        "Verde = dove viene MOSTRATA (1:1, senza scalare)\n"
        "Trascina i bordi/angoli per ridimensionare, dentro per spostare\n"
        "Invio = conferma e chiudi    Esc = annulla"
        );

    if(!m_feedback.isEmpty())
    {
        painter.setPen(Qt::green);

        QFont feedbackFont =
            painter.font();

        feedbackFont.setPointSize(14);
        feedbackFont.setBold(true);

        painter.setFont(feedbackFont);

        painter.drawText(
            QRect(20, 115, 760, 40),
            Qt::AlignLeft,
            m_feedback
            );
    }
}


// ============================================================
// MOUSE
// ============================================================

void ScreenMirrorSetup::mousePressEvent(
    QMouseEvent *event
    )
{
    m_feedback.clear();

    const QPoint pos =
        event->pos();

    const Hit hit =
        hitTest(pos);

    m_activeBox =
        hit.box;

    m_activeHandle =
        hit.handle;

    m_dragStart =
        pos;

    if(m_activeBox != Box::None)
    {
        m_rectAtDragStart =
            rectOf(m_activeBox);
    }

    update();
}


void ScreenMirrorSetup::mouseMoveEvent(
    QMouseEvent *event
    )
{
    const QPoint pos =
        event->pos();

    if(m_activeHandle == Handle::None)
    {
        updateCursor(
            hitTest(pos).handle
            );

        return;
    }

    if(m_activeHandle == Handle::Move)
    {
        rectOf(m_activeBox) =
            m_rectAtDragStart.translated(
                pos - m_dragStart
                );
    }
    else
    {
        applyResize(
            m_activeHandle,
            pos
            );
    }

    update();
}


void ScreenMirrorSetup::mouseReleaseEvent(
    QMouseEvent *
    )
{
    if(m_activeBox != Box::None)
    {
        // Normalizza e porta il rettangolo dentro lo schermo
        // con dimensioni minime.
        rectOf(m_activeBox) =
            sanitized(
                rectOf(m_activeBox)
                );
    }

    m_activeBox =
        Box::None;

    m_activeHandle =
        Handle::None;

    update();
}


void ScreenMirrorSetup::keyPressEvent(
    QKeyEvent *event
    )
{
    // Esc: chiude senza salvare (configure() rilegge i valori
    // salvati ogni volta che si riapre).
    if(event->key() == Qt::Key_Escape)
    {
        hide();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}
