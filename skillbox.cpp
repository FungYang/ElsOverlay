#include "skillbox.h"

#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QDebug>



SkillBox::SkillBox(
    const QString &imagePath,
    const QString &skillName,
    int cooldownTime,
    QWidget *parent
    )
    : QWidget(parent),
    skillName(skillName)
{

    image.load(imagePath);
    QImage gray = image.toImage().convertToFormat(QImage::Format_Grayscale8);

    grayImage = QPixmap::fromImage(gray);

    setFixedSize(60,55);
    cooldown = cooldownTime;
    currentCooldown = 0;
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

}



void SkillBox::paintEvent(QPaintEvent *)
{

    QPainter painter(this);


    // sfondo rettangolo
    painter.fillRect(
        rect(),
        QColor(30,30,30,180)
        );


    // immagine skill
    QPixmap drawImage = image;

    if(activeCooldown)
    {
        drawImage = grayImage;
    }


    if(!drawImage.isNull())
    {
        painter.drawPixmap(
            5,
            15,
            50,
            35,
            drawImage
            );
    }


    // numero cooldown
    painter.setPen(Qt::white);

    QFont font;
    font.setPointSize(16);
    font.setBold(true);

    painter.setFont(font);


    painter.drawText(
        rect(),
        Qt::AlignCenter,
        activeCooldown ? QString::number(currentCooldown) : "Ready"
        );

}

void SkillBox::startCooldown()
{
    if(activeCooldown)
    {
        return;
    }


    currentCooldown = cooldown;
    activeCooldown = true;

    update();
}


void SkillBox::resetCooldown()
{
    activeCooldown = false;
    currentCooldown = 0;

    update();
}



void SkillBox::tick()
{
    if(!activeCooldown)
        return;


    currentCooldown--;


    if(currentCooldown <= 0)
    {
        resetCooldown();
        return;
    }


    update();
}

