#include "buffvisiondebug.h"

#include <QVBoxLayout>
#include <QMouseEvent>



BuffVisionDebug::BuffVisionDebug(
    QWidget *parent
    )
    : QWidget(parent)
{

    setWindowTitle(
        "BuffVision Debug"
        );


    setFixedSize(
        250,
        300
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setAttribute(
        Qt::WA_TransparentForMouseEvents,
        false
        );



    label =
        new QLabel(
            this
            );


    label->setAlignment(
        Qt::AlignCenter
        );


    QFont font;

    font.setPointSize(16);
    font.setBold(true);

    label->setFont(
        font
        );



    QVBoxLayout *layout =
        new QVBoxLayout(
            this
            );


    layout->addWidget(
        label
        );


    setLayout(
        layout
        );

}




void BuffVisionDebug::setScores(
    double crop1State1,
    double crop1State2,
    double crop2State1,
    double crop2State2
    )
{

    label->setText(
        QString(
            "CROP 1\n"
            "Ref1: %1\n"
            "Ref2: %2\n\n"
            "CROP 2\n"
            "Ref1: %3\n"
            "Ref2: %4"
            )
            .arg(crop1State1, 0, 'f', 3)
            .arg(crop1State2, 0, 'f', 3)
            .arg(crop2State1, 0, 'f', 3)
            .arg(crop2State2, 0, 'f', 3)
        );

}




void BuffVisionDebug::mousePressEvent(
    QMouseEvent *event
    )
{

    if(event->button() == Qt::LeftButton)
    {

        dragPosition =
            event->globalPosition().toPoint()
            - frameGeometry().topLeft();


        event->accept();

    }

}



void BuffVisionDebug::mouseMoveEvent(
    QMouseEvent *event
    )
{

    if(event->buttons() & Qt::LeftButton)
    {

        move(
            event->globalPosition().toPoint()
            - dragPosition
            );


        event->accept();

    }

}