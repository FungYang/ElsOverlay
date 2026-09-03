#pragma once

#include <QWidget>
#include <QRect>
#include <QString>

class OverlayRoot;

class TranscendenceCaptureSetup : public QWidget
{
    Q_OBJECT

public:
    explicit TranscendenceCaptureSetup(
        OverlayRoot *parent = nullptr
        );

    QRect searchArea() const;
    QRect iconRect() const;

    void setSearchArea(const QRect &area);
    void setIconRect(const QRect &rect);
    void setCaptureMode(bool capturing);
    void showFeedback(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum class Handle
    {
        None,
        Move,
        MoveIcon,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Top,
        Bottom,
        Left,
        Right
    };

    Handle hitTest(const QPoint &pos) const;

    void updateCursor(Handle handle);
    void applyResize(
        Handle handle,
        const QPoint &pos
        );

    QRect m_searchArea;
    QRect m_iconRect;

    Handle m_activeHandle = Handle::None;

    QPoint m_dragStart;
    QRect m_areaAtDragStart;
    QPoint m_iconDragOffset;

    QString m_feedback;

    bool m_hideRectangles = false;

    static constexpr int HANDLE_SIZE = 10;
};