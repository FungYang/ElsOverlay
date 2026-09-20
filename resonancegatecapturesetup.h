#pragma once

#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QTimer>

class ResonanceGateCaptureSetup : public QWidget
{
    Q_OBJECT

public:
    explicit ResonanceGateCaptureSetup(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

    QRect gateZoneRect() const;

    void showFeedback(const QString &text);
    void captureReference(); // tasto P: salva invariant.png

    static bool referenceExists();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    static constexpr int RESIZE_MARGIN = 8;
    static constexpr int MIN_ZONE_SIZE = 12;

    enum class ResizeCorner { None, TopLeft, TopRight, BottomLeft, BottomRight };
    enum class DragMode { None, Move, Resize };

    QRect m_zoneRect;
    DragMode m_dragMode = DragMode::None;
    ResizeCorner m_dragCorner = ResizeCorner::None;
    QPoint m_lastMousePosition;

    QString feedbackText;
    QTimer feedbackTimer;

    ResizeCorner cornerAt(const QRect &zone, const QPoint &pos) const;
    void applyCornerResize(QRect &zone, ResizeCorner corner, const QPoint &delta);
    void keepInsideScreen(QRect &zone);
};