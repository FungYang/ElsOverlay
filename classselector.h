#ifndef CLASSSELECTOR_H
#define CLASSSELECTOR_H

#include <QWidget>
#include <QList>
#include <QPixmap>
#include "classdata.h"


class ClassSelector : public QWidget
{
    Q_OBJECT

public:

    explicit ClassSelector(QWidget *parent = nullptr);
    void open();


signals:

    void classSelected(ClassType type);
    void selectionCancelled();


protected:

    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;


private:

    struct ClassButton
    {
        QRect rect;
        QPixmap image;
        ClassType type;
    };


    QList<ClassButton> buttons;


    QRect cancelButton;


    void createButtons();

};

#endif