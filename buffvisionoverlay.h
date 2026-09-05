
#ifndef BUFFVISIONOVERLAY_H
#define BUFFVISIONOVERLAY_H

#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QTimer>
#include <QWidget>

#include "buffvisioncore.h"


    class BuffVisionOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit BuffVisionOverlay(
        BuffVisionCore *core,
        QWidget *parent = nullptr
        );

    void resetOverlay();

    void setScores(
        double s1,
        double s2
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

    void enterEvent(
        QEnterEvent *event
        ) override;

    void leaveEvent(
        QEvent *event
        ) override;


private:

    enum class ResizeCorner
    {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };


    QPoint dragPosition;

    BuffVisionCore *core;


    double score1 = 0;

    double score2 = 0;


    QPixmap buffImage;

    QPixmap grayBuffImage;


    int refreshCounter = 0;


    bool buff60Active = false;


    int cooldown15 = 0;


    QTimer timer;


    // ========================================================
    // RESIZE
    // ========================================================

    bool dragging = false;

    bool resizing = false;


    ResizeCorner resizeCorner =
        ResizeCorner::None;


    QPoint resizeStartGlobal;

    QPoint resizeStartPosition;

    QSize resizeStartSize;


    static constexpr int ResizeMargin = 14;

    static constexpr int HandleSize = 8;

    static constexpr int MinimumSize = 30;


    ResizeCorner resizeCornerAt(
        const QPoint &position
        ) const;


    void updateResizeCursor(
        const QPoint &position
        );


    void resizeFromCorner(
        const QPoint &globalPosition
        );


    void saveGeometry();


    QPixmap createGrayImage(
        const QPixmap &source
        );


    void updateBuffColor();

};

#endif
