#pragma once

#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QTimer>
#include <array>

class AtmaZoneCaptureSetup : public QWidget
{
    Q_OBJECT

public:
    explicit AtmaZoneCaptureSetup(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

    QRect redZoneRect(int index) const;
    QRect blueZoneRect() const;

    void showFeedback(const QString &text);
    void captureAllReferences();

    static bool referencesExist();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    static constexpr int RED_COUNT = 6;
    static constexpr int RESIZE_MARGIN = 4;   // area sensibile attorno all'angolo
    static constexpr int MIN_ZONE_SIZE = 12;

    enum class ResizeCorner { None, TopLeft, TopRight, BottomLeft, BottomRight };

    std::array<QRect, RED_COUNT> m_redRects;
    QRect m_blueRect;

    enum class DragMode { None, MoveRed, MoveBlue, ResizeRed, ResizeBlue };

    DragMode m_dragMode = DragMode::None;
    int m_dragRedIndex = -1;
    ResizeCorner m_dragCorner = ResizeCorner::None; // angolo afferrato (vale per tutti i rossi se ResizeRed)
    QPoint m_lastMousePosition;

    QString feedbackText;
    QTimer feedbackTimer;

    QSize defaultZoneSize() const;

    ResizeCorner cornerAt(const QRect &zone, const QPoint &pos) const;
    void drawHandles(QPainter &p, const QRect &zone);

    void applyCornerResize(QRect &zone, ResizeCorner corner, const QPoint &delta);
    void resizeAllRed(ResizeCorner corner, const QPoint &delta);

    void keepInsideScreen(QRect &zone);
};