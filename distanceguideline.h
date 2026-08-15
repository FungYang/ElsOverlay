#ifndef DISTANCEGUIDELINE_H
#define DISTANCEGUIDELINE_H

#include <QColor>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>


class DistanceGuideLine :
                          public QWidget
{
    Q_OBJECT


public:

    explicit DistanceGuideLine(
        const QColor &color,
        QWidget *parent = nullptr
        );


    void setConfigurationMode(
        bool enabled
        );


    int configurationPositionX() const;


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

    QColor m_color;


    bool m_configurationMode =
        false;


    bool m_dragging =
        false;


    int m_dragOffsetX =
        0;
};


#endif