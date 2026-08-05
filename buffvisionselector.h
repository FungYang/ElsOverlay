#ifndef BUFFVISIONSELECTOR_H
#define BUFFVISIONSELECTOR_H

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>


class BuffVisionSelector : public QWidget
{
    Q_OBJECT

public:

    explicit BuffVisionSelector(
        QWidget *parent = nullptr
        );


    void resetSelector();
    void showSelector();



signals:

    void enabled();

    void disabled();



protected:

    void mousePressEvent(
        QMouseEvent *event
        ) override;


    void paintEvent(
        QPaintEvent *event
        ) override;



private:

    QPixmap flowImage;


    QRect flowRect;

    QRect disableRect;


};

#endif