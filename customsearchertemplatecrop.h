#pragma once

#include <QImage>
#include <QRect>
#include <QSize>
#include <QWidget>

class QCloseEvent;

class CustomSearcherTemplateCrop : public QWidget
{
    Q_OBJECT

public:

    explicit CustomSearcherTemplateCrop(
        const QImage &source,
        const QSize &initialCropSize = QSize(28, 28),
        QWidget *parent = nullptr
        );

    QRect cropRect() const;

    QImage croppedImage() const;

signals:

    void accepted(
        const QImage &image
        );

    void canceled();

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
    void closeEvent(QCloseEvent *event) override;

private:

    void increaseCropSize();
    void decreaseCropSize();

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

    void clampCrop();

    void acceptCrop();

private:

    bool m_accepted = false;
    QImage m_source;

    QRect m_cropRect;

    QPoint m_mousePressSource;
    QPoint m_cropAtMousePress;

    bool m_dragging = false;

    static constexpr int PIXEL_SIZE = 3;
    static constexpr int IMAGE_MARGIN = 20;
};