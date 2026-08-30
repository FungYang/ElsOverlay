#pragma once

#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QString>

class OverlayRoot;

class TranscendenceCaptureSetup : public QWidget
{
    Q_OBJECT

public:
    explicit TranscendenceCaptureSetup(
        OverlayRoot *parent
        );

    QRect searchArea() const;
    QRect iconRect() const;

    void setSearchArea(
        const QRect &area
        );

    void setIconRect(
        const QRect &rect
        );

    void setCaptureMode(
        bool capturing
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

private:

    enum class Handle
    {
        None,
        Move,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        MoveIcon
    };

    static constexpr int HANDLE_SIZE = 10;

    Handle hitTest(
        const QPoint &pos
        ) const;

    void updateCursor(
        Handle handle
        );

    void applyResize(
        Handle handle,
        const QPoint &pos
        );


    QRect m_searchArea;
    QRect m_iconRect;

    Handle m_activeHandle =
        Handle::None;

    QPoint m_dragStart;
    QRect m_areaAtDragStart;
    QPoint m_iconDragOffset;

    bool m_hideRectangles = false;

    QString m_feedback;
};