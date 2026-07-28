#include "classselector.h"

#include <QPainter>
#include <QMouseEvent>



ClassSelector::ClassSelector(QWidget *parent)
    : QWidget(parent)
{

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    resize(
        300,
        80
        );


    createButtons();

}



void ClassSelector::createButtons()
{

    int size = 60;


    int startX = 0;


    for(int i = 0; i < 4; i++)
    {

        ClassButton button;


        button.rect =
            QRect(
                startX + i * size,
                10,
                size,
                size
                );


        switch(i)
        {

        case 0:
            button.type = ClassType::ES;
            button.image = QPixmap("images/ES.png");
            break;


        case 1:
            button.type = ClassType::FL;
            button.image = QPixmap("images/FL.png");
            break;


        case 2:
            button.type = ClassType::BQ;
            button.image = QPixmap("images/BQ.png");
            break;


        case 3:
            button.type = ClassType::AD;
            button.image = QPixmap("images/AD.png");
            break;

        }


        buttons.append(
            button
            );

    }


    cancelButton =
        QRect(
            240,
            10,
            size,
            size
            );

}



void ClassSelector::paintEvent(QPaintEvent *)
{

    QPainter painter(this);


    painter.setRenderHint(
        QPainter::Antialiasing
        );


    // Pulsanti classi

    for(const auto &button : buttons)
    {

        painter.setBrush(
            Qt::NoBrush
            );


        painter.setPen(
            QPen(
                Qt::white,
                2
                )
            );


        painter.drawRect(
            button.rect
            );


        painter.drawPixmap(
            button.rect.x() + 2,
            button.rect.y() + 2,
            55,
            55,
            button.image
            );

    }



    // Pulsante annulla

    painter.setBrush(
        QColor(120,120,120,120)
        );


    painter.setPen(
        QPen(
            Qt::white,
            2
            )
        );


    painter.drawRect(
        cancelButton
        );


    painter.setPen(
        QColor(220,40,40)
        );


    painter.setFont(
        QFont(
            "Arial",
            30,
            QFont::Bold
            )
        );


    painter.drawText(
        cancelButton,
        Qt::AlignCenter,
        "X"
        );

}



void ClassSelector::mousePressEvent(
    QMouseEvent *event
    )
{

    if(event->button() != Qt::LeftButton)
        return;


    QPoint pos =
        event->pos();



    for(const auto &button : buttons)
    {

        if(button.rect.contains(pos))
        {

            hide();


            emit classSelected(
                button.type
                );


            return;

        }

    }



    if(cancelButton.contains(pos))
    {

        hide();


        emit selectionCancelled();

    }

}



void ClassSelector::open()
{

    show();

    raise();

    activateWindow();

}