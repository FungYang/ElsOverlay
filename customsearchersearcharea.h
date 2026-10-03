#pragma once

#include <QRect>
#include <QWidget>

class QCloseEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class CustomSearcherSearchArea : public QWidget
{
    Q_OBJECT

public:
    explicit CustomSearcherSearchArea(
        const QRect &initialArea,
        QWidget *parent = nullptr
        );

    QRect searchArea() const;

signals:
    void accepted(const QRect &area);
    void canceled();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
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

    Handle hitTest(const QPoint &pos) const;

    void updateCursor(Handle handle);

    void applyResize(
        Handle handle,
        const QPoint &pos
        );

    void clampAreaToWindow();

    void acceptArea();

private:
    QRect m_searchArea;

    Handle m_activeHandle = Handle::None;

    QPoint m_dragStart;
    QRect m_areaAtDragStart;

    bool m_accepted = false;

    static constexpr int HANDLE_SIZE = 10;
    static constexpr int MIN_WIDTH = 32;
    static constexpr int MIN_HEIGHT = 32;
};