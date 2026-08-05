#pragma once

#include <QWidget>
#include <QLabel>


class BuffVisionDebug : public QWidget
{
    Q_OBJECT

public:

    explicit BuffVisionDebug(
        QWidget *parent = nullptr
        );


public slots:

    void setScores(
        double crop1State1,
        double crop1State2,
        double crop2State1,
        double crop2State2
        );


protected:

    void mousePressEvent(
        QMouseEvent *event
        ) override;


    void mouseMoveEvent(
        QMouseEvent *event
        ) override;


private:

    QLabel *label;

    QPoint dragPosition;

};