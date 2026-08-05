#ifndef BUFFVISIONOVERLAY_H
#define BUFFVISIONOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QPoint>
#include <QSettings>
#include <QMouseEvent>

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
    void setScores(double s1, double s2);


protected:

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;


private:

    QPoint dragPosition;
    BuffVisionCore *core;
    double score1 = 0;
    double score2 = 0;

    QPixmap buffImage;
    QPixmap grayBuffImage;

    int refreshCounter = 0;


    // stato grafico del buff lungo
    bool buff60Active = false;


    // countdown visualizzato del buff corto
    int cooldown15 = 0;


    // aggiorna timer grafico
    QTimer timer;



    QPixmap createGrayImage(
        const QPixmap &source
        );


    void updateBuffColor();



};

#endif