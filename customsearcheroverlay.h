#pragma once

#include <QImage>
#include <QPoint>
#include <QRect>
#include <QWidget>

class QMouseEvent;
class QTimer;

class CustomSearcherOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit CustomSearcherOverlay(
        int templateId,
        const QImage &displayImage,
        QWidget *parent = nullptr
        );

    int templateId() const;

    void setScreenRect(const QRect &rect);
    QRect screenRect() const;

    void setVisibleBySearcher(bool visible);

    void startCooldown(int cooldownMs);
    void stopCooldown();

    bool isCoolingDown() const;
    void setTransparency(
        int value
        );

signals:
    void cooldownFinished(int templateId);

    // Emesso solo quando l'utente ha finito di spostare/ridimensionare.
    void geometryChangeFinished(
        int templateId,
        const QRect &rect
        );

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void updateCountdown();

private:
    enum class Interaction
    {
        None,
        Move,
        ResizeTopLeft,
        ResizeTopRight,
        ResizeBottomLeft,
        ResizeBottomRight,
        ResizeTop,
        ResizeBottom,
        ResizeLeft,
        ResizeRight
    };

    Interaction hitTest(const QPoint &pos) const;
    void applyResize(
        Interaction interaction,
        const QPoint &pos
        );

    void updateCursor(Interaction interaction);

private:
    int m_transparency = 255;
    int m_templateId = -1;

    QImage m_displayImage;
    QImage m_grayImage;

    QRect m_screenRect;

    QTimer *m_countdownTimer = nullptr;

    qint64 m_cooldownEndMs = 0;
    int m_lastDisplayedSecond = -1;

    bool m_visibleBySearcher = false;
    bool m_coolingDown = false;

    bool m_dragging = false;
    Interaction m_interaction = Interaction::None;

    QPoint m_dragStartGlobal;
    QRect m_rectAtDragStart;

    static constexpr int HANDLE_SIZE = 10;
    static constexpr int MIN_WIDTH = 16;
    static constexpr int MIN_HEIGHT = 16;
};