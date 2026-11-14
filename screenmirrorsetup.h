#pragma once

// ============================================================
// SCREEN MIRROR - CONFIGURAZIONE
//
// Finestra a tutto schermo con DUE rettangoli:
//   - azzurro: regione SORGENTE (quella che viene catturata)
//   - verde:   regione di DESTINAZIONE (dove viene mostrata)
// Ognuno si sposta trascinandolo e si ridimensiona dai bordi e
// dagli angoli, come l'area di ricerca della Trascendenza.
// ============================================================

#include <QPoint>
#include <QRect>
#include <QString>
#include <QWidget>

class ScreenMirrorSetup : public QWidget
{
    Q_OBJECT

public:

    explicit ScreenMirrorSetup(
        QWidget *parent = nullptr
        );

    QRect sourceRect() const;
    QRect destRect() const;

    void setRects(
        const QRect &source,
        const QRect &dest
        );

    void showFeedback(
        const QString &text
        );


protected:

    void paintEvent(
        QPaintEvent *event
        ) override;

    void mousePressEvent(
        QMouseEvent *event
        ) override;

    void mouseMoveEvent(
        QMouseEvent *event
        ) override;

    void mouseReleaseEvent(
        QMouseEvent *event
        ) override;

    void keyPressEvent(
        QKeyEvent *event
        ) override;


private:

    enum class Box
    {
        None,
        Source,
        Dest
    };

    enum class Handle
    {
        None,
        Move,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Bottom,
        Left,
        Right
    };

    struct Hit
    {
        Box box = Box::None;
        Handle handle = Handle::None;
    };

    Hit hitTest(
        const QPoint &pos
        ) const;

    Handle handleAt(
        const QRect &rect,
        const QPoint &pos
        ) const;

    void updateCursor(
        Handle handle
        );

    QRect &rectOf(
        Box box
        );

    const QRect &rectOf(
        Box box
        ) const;

    void applyResize(
        Handle handle,
        const QPoint &pos
        );

    void paintBox(
        QPainter &painter,
        const QRect &rect,
        const QColor &color,
        const QString &label
        );

    // Rettangolo valido: almeno MIN_SIZE per lato e dentro lo schermo.
    QRect sanitized(
        const QRect &rect
        ) const;


    QRect m_source;
    QRect m_dest;

    Box m_activeBox = Box::None;
    Handle m_activeHandle = Handle::None;

    QPoint m_dragStart;
    QRect m_rectAtDragStart;

    QString m_feedback;

    static constexpr int HANDLE_SIZE = 10;
    static constexpr int MIN_SIZE = 8;
};
