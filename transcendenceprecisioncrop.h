#pragma once

#include <QWidget>
#include <QImage>
#include <QRect>

class TranscendencePrecisionCrop : public QWidget
{
    Q_OBJECT

public:
    explicit TranscendencePrecisionCrop(
        const QImage &source,
        QWidget *parent = nullptr
        );

    QRect cropRect() const;
    QImage croppedImage() const;

signals:
    void accepted(const QImage &image);
    void canceled();

public slots:
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void clampCrop();
    void acceptCrop();
    void updateFromMouse(const QPoint &pos);

    QImage m_source;
    QRect m_cropRect;

    int m_zoom = 3;
    int m_pixelSize = 3;

    QPoint m_mousePressSource;
    QPoint m_cropAtMousePress;
    bool m_dragging = false;
};
