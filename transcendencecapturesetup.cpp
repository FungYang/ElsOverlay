#include "transcendencecapturesetup.h"
#include "transcendencevisionconfig.h"

#include <QPainter>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>


TranscendenceCaptureSetup::TranscendenceCaptureSetup(
    OverlayRoot *parent
    )
    : QWidget(
          reinterpret_cast<QWidget *>(parent)
          )
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

    QRect screenGeometry =
        screen
            ? screen->geometry()
            : QRect(0, 0, 1920, 1080);

    setGeometry(
        screenGeometry
        );


    m_searchArea =
        QRect(
            700,
            600,
            300,
            150
            );

    m_iconRect =
        QRect(
            m_searchArea.center().x() -
                TranscendenceVisionConfig::ICON_WIDTH / 2,
            m_searchArea.center().y() -
                TranscendenceVisionConfig::ICON_HEIGHT / 2,
            TranscendenceVisionConfig::ICON_WIDTH,
            TranscendenceVisionConfig::ICON_HEIGHT
            );
}


QRect TranscendenceCaptureSetup::searchArea() const
{
    return m_searchArea;
}


QRect TranscendenceCaptureSetup::iconRect() const
{
    return m_iconRect;
}


void TranscendenceCaptureSetup::setSearchArea(
    const QRect &area
    )
{
    m_searchArea = area;

    update();
}


void TranscendenceCaptureSetup::setIconRect(
    const QRect &rect
    )
{
    m_iconRect = rect;

    update();
}


void TranscendenceCaptureSetup::setCaptureMode(
    bool capturing
    )
{
    m_hideRectangles = capturing;

    update();
}


void TranscendenceCaptureSetup::showFeedback(
    const QString &text
    )
{
    m_feedback = text;

    update();
}


// =========================================================
// HIT TEST
// =========================================================

TranscendenceCaptureSetup::Handle
TranscendenceCaptureSetup::hitTest(
    const QPoint &pos
    ) const
{
    if(m_iconRect.contains(pos))
    {
        return Handle::MoveIcon;
    }


    const QRect &r =
        m_searchArea;

    const int h =
        HANDLE_SIZE;


    QRect topLeft(
        r.left() - h/2, r.top() - h/2, h, h
        );

    QRect topRight(
        r.right() - h/2, r.top() - h/2, h, h
        );

    QRect bottomLeft(
        r.left() - h/2, r.bottom() - h/2, h, h
        );

    QRect bottomRight(
        r.right() - h/2, r.bottom() - h/2, h, h
        );

    QRect top(
        r.left() + h, r.top() - h/2,
        r.width() - 2*h, h
        );

    QRect bottom(
        r.left() + h, r.bottom() - h/2,
        r.width() - 2*h, h
        );

    QRect left(
        r.left() - h/2, r.top() + h,
        h, r.height() - 2*h
        );

    QRect right(
        r.right() - h/2, r.top() + h,
        h, r.height() - 2*h
        );


    if(topLeft.contains(pos))     return Handle::TopLeft;
    if(topRight.contains(pos))    return Handle::TopRight;
    if(bottomLeft.contains(pos))  return Handle::BottomLeft;
    if(bottomRight.contains(pos)) return Handle::BottomRight;
    if(top.contains(pos))         return Handle::Top;
    if(bottom.contains(pos))      return Handle::Bottom;
    if(left.contains(pos))        return Handle::Left;
    if(right.contains(pos))       return Handle::Right;

    if(r.contains(pos))           return Handle::Move;

    return Handle::None;
}


void TranscendenceCaptureSetup::updateCursor(
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
    case Handle::MoveIcon:
        setCursor(Qt::SizeAllCursor);
        break;

    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}


// =========================================================
// PAINT
// =========================================================

void TranscendenceCaptureSetup::paintEvent(
    QPaintEvent *
    )
{
    QPainter p(this);

    p.setRenderHint(
        QPainter::Antialiasing
        );


    if(m_hideRectangles)
    {
        return;
    }


    // =========================
    // AREA DI RICERCA
    // =========================

    p.setPen(
        QPen(
            QColor(0, 200, 255),
            2
            )
        );

    p.setBrush(
        QColor(0, 200, 255, 18)
        );

    p.drawRect(
        m_searchArea
        );


    // Maniglie agli angoli/bordi.

    p.setBrush(
        QColor(0, 200, 255)
        );

    p.setPen(
        Qt::NoPen
        );

    auto drawHandle =
        [&p](const QPoint &center)
    {
        p.drawRect(
            QRect(
                center.x() - HANDLE_SIZE/2,
                center.y() - HANDLE_SIZE/2,
                HANDLE_SIZE,
                HANDLE_SIZE
                )
            );
    };

    drawHandle(m_searchArea.topLeft());
    drawHandle(m_searchArea.topRight());
    drawHandle(m_searchArea.bottomLeft());
    drawHandle(m_searchArea.bottomRight());

    drawHandle(
        QPoint(
            m_searchArea.center().x(),
            m_searchArea.top()
            )
        );

    drawHandle(
        QPoint(
            m_searchArea.center().x(),
            m_searchArea.bottom()
            )
        );

    drawHandle(
        QPoint(
            m_searchArea.left(),
            m_searchArea.center().y()
            )
        );

    drawHandle(
        QPoint(
            m_searchArea.right(),
            m_searchArea.center().y()
            )
        );


    // =========================
    // ICONA (28x28)
    // =========================

    p.setPen(
        QPen(
            QColor(255, 200, 0),
            2
            )
        );

    p.setBrush(
        QColor(255, 200, 0, 50)
        );

    p.drawRect(
        m_iconRect
        );


    // =========================
    // ISTRUZIONI
    // =========================

    p.setPen(
        Qt::white
        );

    QFont font =
        p.font();

    font.setPointSize(11);

    p.setFont(font);


    QString instructions =
        "Trascina il bordo/angoli per ridimensionare l'area (azzurro)\n"
        "Trascina dentro l'area per spostarla\n"
        "Trascina il riquadro giallo (28x28) sull'icona da salvare\n"
        "P = salva icona    Invio = conferma e chiudi";


    p.drawText(
        QRect(
            20,
            20,
            600,
            90
            ),
        Qt::AlignLeft | Qt::TextWordWrap,
        instructions
        );


    if(!m_feedback.isEmpty())
    {
        p.setPen(
            Qt::green
            );

        QFont feedbackFont =
            p.font();

        feedbackFont.setPointSize(14);

        feedbackFont.setBold(true);

        p.setFont(feedbackFont);

        p.drawText(
            QRect(
                20,
                120,
                600,
                40
                ),
            Qt::AlignLeft,
            m_feedback
            );
    }
}


// =========================================================
// MOUSE
// =========================================================

void TranscendenceCaptureSetup::mousePressEvent(
    QMouseEvent *event
    )
{
    m_feedback.clear();


    QPoint pos =
        event->pos();


    m_activeHandle =
        hitTest(pos);

    m_dragStart =
        pos;

    m_areaAtDragStart =
        m_searchArea;


    if(m_activeHandle == Handle::MoveIcon)
    {
        m_iconDragOffset =
            pos - m_iconRect.topLeft();
    }


    update();
}


void TranscendenceCaptureSetup::mouseMoveEvent(
    QMouseEvent *event
    )
{
    QPoint pos =
        event->pos();


    if(m_activeHandle == Handle::None)
    {
        updateCursor(
            hitTest(pos)
            );

        return;
    }


    if(m_activeHandle == Handle::MoveIcon)
    {
        QPoint topLeft =
            pos - m_iconDragOffset;

        m_iconRect =
            QRect(
                topLeft,
                QSize(
                    TranscendenceVisionConfig::ICON_WIDTH,
                    TranscendenceVisionConfig::ICON_HEIGHT
                    )
                );

        update();

        return;
    }


    if(m_activeHandle == Handle::Move)
    {
        QPoint delta =
            pos - m_dragStart;

        m_searchArea =
            m_areaAtDragStart.translated(
                delta
                );

        update();

        return;
    }


    applyResize(
        m_activeHandle,
        pos
        );

    update();
}


void TranscendenceCaptureSetup::mouseReleaseEvent(
    QMouseEvent *
    )
{
    m_activeHandle =
        Handle::None;

    m_searchArea =
        m_searchArea.normalized();
}


void TranscendenceCaptureSetup::applyResize(
    Handle handle,
    const QPoint &pos
    )
{
    QRect r =
        m_areaAtDragStart;

    QPoint delta =
        pos - m_dragStart;


    switch(handle)
    {
    case Handle::TopLeft:
        r.setTopLeft(
            r.topLeft() + delta
            );
        break;

    case Handle::TopRight:
        r.setTopRight(
            r.topRight() + delta
            );
        break;

    case Handle::BottomLeft:
        r.setBottomLeft(
            r.bottomLeft() + delta
            );
        break;

    case Handle::BottomRight:
        r.setBottomRight(
            r.bottomRight() + delta
            );
        break;

    case Handle::Top:
        r.setTop(
            r.top() + delta.y()
            );
        break;

    case Handle::Bottom:
        r.setBottom(
            r.bottom() + delta.y()
            );
        break;

    case Handle::Left:
        r.setLeft(
            r.left() + delta.x()
            );
        break;

    case Handle::Right:
        r.setRight(
            r.right() + delta.x()
            );
        break;

    default:
        break;
    }


    m_searchArea = r;
}