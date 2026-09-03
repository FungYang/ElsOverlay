#include "skillbox.h"

#include <QPainter>
#include <QImage>


SkillBox::SkillBox(
    const QString &imagePath,
    const QString &skillName,
    int cooldownTime,
    QWidget *parent
    )
    : QWidget(parent),
    skillName(skillName),
    imagePath(imagePath)
{
    image.load(imagePath);

    QImage gray =
        image.toImage().convertToFormat(
            QImage::Format_Grayscale8
            );

    grayImage =
        QPixmap::fromImage(gray);


    setFixedSize(60,55);

    cooldown =
        cooldownTime;

    currentCooldown =
        0;


    setAttribute(
        Qt::WA_TranslucentBackground
        );

    setAttribute(
        Qt::WA_TransparentForMouseEvents
        );
}


// =========================================================
// PAINT
// =========================================================

void SkillBox::paintEvent(
    QPaintEvent *
    )
{
    QPainter painter(this);


    painter.fillRect(
        rect(),
        QColor(30,30,30,180)
        );


    QPixmap drawImage =
        image;


    if(activeCooldown)
    {
        drawImage =
            grayImage;
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


    painter.setPen(
        Qt::white
        );


    QFont font;

    font.setPointSize(16);
    font.setBold(true);

    painter.setFont(font);


    painter.drawText(
        rect(),
        Qt::AlignCenter,
        activeCooldown
            ? QString::number(currentCooldown)
            : "Ready"
        );
}


// =========================================================
// COOLDOWN
// =========================================================

void SkillBox::startCooldown()
{
    if(activeCooldown)
        return;


    currentCooldown =
        cooldown;

    activeCooldown =
        true;


    update();
}


void SkillBox::resetCooldown()
{
    activeCooldown =
        false;

    currentCooldown =
        0;

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


// =========================================================
// CONFIGURATION
// =========================================================

void SkillBox::setImage(
    const QString &newImagePath
    )
{
    imagePath =
        newImagePath;


    image.load(
        imagePath
        );


    QImage gray =
        image.toImage().convertToFormat(
            QImage::Format_Grayscale8
            );


    grayImage =
        QPixmap::fromImage(
            gray
            );


    update();
}


void SkillBox::setSkillName(
    const QString &name
    )
{
    skillName =
        name;

    update();
}


void SkillBox::setCooldown(
    int cooldownTime
    )
{
    cooldown =
        qMax(
            1,
            cooldownTime
            );
}


QString SkillBox::getSkillName() const
{
    return skillName;
}


QString SkillBox::getImagePath() const
{
    return imagePath;
}


int SkillBox::getCooldown() const
{
    return cooldown;
}