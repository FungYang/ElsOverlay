#include "customsearchersearcharea.h"

#include <QCloseEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

CustomSearcherSearchArea::CustomSearcherSearchArea(
    const QRect &initialArea,
    QWidget *parent
    )
    : QWidget(parent)
{
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    QScreen *screen =
        QGuiApplication::primaryScreen();

    const QRect screenGeometry =
        screen
            ? screen->geometry()
            : QRect(0, 0, 1920, 1080);

    setGeometry(screenGeometry);

    if (initialArea.isValid() &&
        !initialArea.isEmpty())
    {
        m_searchArea =
            initialArea.normalized();
    }
    else
    {
        m_searchArea =
            QRect(
                screenGeometry.center().x() - 150,
                screenGeometry.center().y() - 75,
                300,
                150
                );
    }

    clampAreaToWindow();
}

QRect CustomSearcherSearchArea::searchArea() const
{
    return m_searchArea;
}

void CustomSearcherSearchArea::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);

    // ==================================================
    // AREA DI RICERCA
    // ==================================================

    p.setPen(
        QPen(
            QColor(0, 200, 255),
            2
            )
        );

    p.setBrush(
        QColor(
            0,
            200,
            255,
            18
            )
        );

    p.drawRect(m_searchArea);

    // ==================================================
    // MANIGLIE
    // ==================================================

    p.setBrush(
        QColor(
            0,
            200,
            255
            )
        );

    p.setPen(Qt::NoPen);

    auto drawHandle =
        [&p](const QPoint &center)
    {
        p.drawRect(
            QRect(
                center.x() - HANDLE_SIZE / 2,
                center.y() - HANDLE_SIZE / 2,
                HANDLE_SIZE,
                HANDLE_SIZE
                )
            );
    };

    drawHandle(
        m_searchArea.topLeft()
        );

    drawHandle(
        m_searchArea.topRight()
        );

    drawHandle(
        m_searchArea.bottomLeft()
        );

    drawHandle(
        m_searchArea.bottomRight()
        );

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

    // ==================================================
    // ISTRUZIONI
    // ==================================================

    p.setPen(Qt::white);

    QFont font =
        p.font();

    font.setPointSize(11);

    p.setFont(font);

    const QString instructions =
        "Custom Searcher - Search Area\n"
        "Trascina il bordo o gli angoli per ridimensionare l'area\n"
        "Trascina dentro l'area per spostarla\n"
        "Invio = conferma e chiudi    Esc = annulla";

    p.drawText(
        QRect(
            20,
            20,
            760,
            100
            ),
        Qt::AlignLeft |
            Qt::TextWordWrap,
        instructions
        );

    // ==================================================
    // DIMENSIONI
    // ==================================================

    const QString sizeText =
        QString(
            "%1 x %2"
            )
            .arg(m_searchArea.width())
            .arg(m_searchArea.height());

    QFont sizeFont =
        p.font();

    sizeFont.setPointSize(11);
    sizeFont.setBold(true);

    p.setFont(sizeFont);

    p.setPen(Qt::white);

    p.drawText(
        QRect(
            m_searchArea.left(),
            m_searchArea.bottom() + 12,
            200,
            25
            ),
        Qt::AlignLeft,
        sizeText
        );
}

CustomSearcherSearchArea::Handle
CustomSearcherSearchArea::hitTest(
    const QPoint &pos
    ) const
{
    const QRect &r =
        m_searchArea;

    const int h =
        HANDLE_SIZE;

    // Prima controlliamo le maniglie degli angoli/lati.

    const QRect topLeft(
        r.left() - h / 2,
        r.top() - h / 2,
        h,
        h
        );

    const QRect topRight(
        r.right() - h / 2,
        r.top() - h / 2,
        h,
        h
        );

    const QRect bottomLeft(
        r.left() - h / 2,
        r.bottom() - h / 2,
        h,
        h
        );

    const QRect bottomRight(
        r.right() - h / 2,
        r.bottom() - h / 2,
        h,
        h
        );

    const QRect top(
        r.left() + h,
        r.top() - h / 2,
        r.width() - 2 * h,
        h
        );

    const QRect bottom(
        r.left() + h,
        r.bottom() - h / 2,
        r.width() - 2 * h,
        h
        );

    const QRect left(
        r.left() - h / 2,
        r.top() + h,
        h,
        r.height() - 2 * h
        );

    const QRect right(
        r.right() - h / 2,
        r.top() + h,
        h,
        r.height() - 2 * h
        );

    if (topLeft.contains(pos))
        return Handle::TopLeft;

    if (topRight.contains(pos))
        return Handle::TopRight;

    if (bottomLeft.contains(pos))
        return Handle::BottomLeft;

    if (bottomRight.contains(pos))
        return Handle::BottomRight;

    if (top.contains(pos))
        return Handle::Top;

    if (bottom.contains(pos))
        return Handle::Bottom;

    if (left.contains(pos))
        return Handle::Left;

    if (right.contains(pos))
        return Handle::Right;

    if (r.contains(pos))
        return Handle::Move;

    return Handle::None;
}

void CustomSearcherSearchArea::updateCursor(
    Handle handle
    )
{
    switch (handle)
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

void CustomSearcherSearchArea::mousePressEvent(
    QMouseEvent *event
    )
{
    if (!event)
        return;

    const QPoint pos =
        event->pos();

    m_activeHandle =
        hitTest(pos);

    m_dragStart =
        pos;

    m_areaAtDragStart =
        m_searchArea;

    updateCursor(
        m_activeHandle
        );

    update();
}

void CustomSearcherSearchArea::mouseMoveEvent(
    QMouseEvent *event
    )
{
    if (!event)
        return;

    const QPoint pos =
        event->pos();

    if (m_activeHandle == Handle::None)
    {
        updateCursor(
            hitTest(pos)
            );

        return;
    }

    if (m_activeHandle == Handle::Move)
    {
        const QPoint delta =
            pos - m_dragStart;

        m_searchArea =
            m_areaAtDragStart.translated(
                delta
                );

        clampAreaToWindow();

        update();

        return;
    }

    applyResize(
        m_activeHandle,
        pos
        );

    clampAreaToWindow();

    update();
}

void CustomSearcherSearchArea::mouseReleaseEvent(
    QMouseEvent *
    )
{
    m_activeHandle =
        Handle::None;

    m_searchArea =
        m_searchArea.normalized();

    clampAreaToWindow();

    updateCursor(
        hitTest(
            mapFromGlobal(
                QCursor::pos()
                )
            )
        );

    update();
}

void CustomSearcherSearchArea::applyResize(
    Handle handle,
    const QPoint &pos
    )
{
    QRect r =
        m_areaAtDragStart;

    const QPoint delta =
        pos - m_dragStart;

    switch (handle)
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

    m_searchArea =
        r.normalized();

    // Dimensione minima.

    if (m_searchArea.width() < MIN_WIDTH)
    {
        if (handle == Handle::Left ||
            handle == Handle::TopLeft ||
            handle == Handle::BottomLeft)
        {
            m_searchArea.setLeft(
                m_searchArea.right() - MIN_WIDTH + 1
                );
        }
        else
        {
            m_searchArea.setWidth(
                MIN_WIDTH
                );
        }
    }

    if (m_searchArea.height() < MIN_HEIGHT)
    {
        if (handle == Handle::Top ||
            handle == Handle::TopLeft ||
            handle == Handle::TopRight)
        {
            m_searchArea.setTop(
                m_searchArea.bottom() - MIN_HEIGHT + 1
                );
        }
        else
        {
            m_searchArea.setHeight(
                MIN_HEIGHT
                );
        }
    }
}

void CustomSearcherSearchArea::clampAreaToWindow()
{
    if (width() <= 0 ||
        height() <= 0)
    {
        return;
    }

    QRect area =
        m_searchArea.normalized();

    if (area.width() < MIN_WIDTH)
        area.setWidth(MIN_WIDTH);

    if (area.height() < MIN_HEIGHT)
        area.setHeight(MIN_HEIGHT);

    if (area.left() < 0)
        area.moveLeft(0);

    if (area.top() < 0)
        area.moveTop(0);

    if (area.right() >= width())
        area.moveRight(width() - 1);

    if (area.bottom() >= height())
        area.moveBottom(height() - 1);

    // Se la ROI è più grande della finestra,
    // la limitiamo alla finestra.

    if (area.width() > width())
    {
        area.setLeft(0);
        area.setRight(width() - 1);
    }

    if (area.height() > height())
    {
        area.setTop(0);
        area.setBottom(height() - 1);
    }

    m_searchArea =
        area;
}

void CustomSearcherSearchArea::keyPressEvent(
    QKeyEvent *event
    )
{
    if (!event)
        return;

    switch (event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        acceptArea();
        return;

    case Qt::Key_Escape:
        close();
        return;

    default:
        break;
    }

    QWidget::keyPressEvent(
        event
        );
}

void CustomSearcherSearchArea::acceptArea()
{
    m_searchArea =
        m_searchArea.normalized();

    clampAreaToWindow();

    m_accepted =
        true;

    emit accepted(
        m_searchArea
        );

    close();
}

void CustomSearcherSearchArea::closeEvent(
    QCloseEvent *event
    )
{
    if (!m_accepted)
        emit canceled();

    QWidget::closeEvent(
        event
        );
}