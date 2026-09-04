
#include "skillbox.h"

#include <QPainter>
#include <QImage>
#include <QFont>


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


    setFixedSize(
        60,
        55
        );


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
    QPainter painter(
        this
        );


    painter.setRenderHint(
        QPainter::SmoothPixmapTransform,
        true
        );


    painter.setRenderHint(
        QPainter::Antialiasing,
        true
        );


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


    // =====================================================
    // DIMENSIONI SCALATE
    // =====================================================

    int imageX =
        qRound(
            5.0 * scale
            );


    int imageY =
        qRound(
            15.0 * scale
            );


    int imageWidth =
        qRound(
            50.0 * scale
            );


    int imageHeight =
        qRound(
            35.0 * scale
            );


    // =====================================================
    // IMAGE
    // =====================================================

    if(!drawImage.isNull())
    {
        painter.drawPixmap(
            imageX,
            imageY,
            imageWidth,
            imageHeight,
            drawImage
            );
    }


    // =====================================================
    // COOLDOWN / READY
    // =====================================================

    painter.setPen(
        Qt::white
        );


    QFont font;

    font.setPointSize(
        qMax(
            1,
            qRound(
                16.0 * scale
                )
            )
        );

    font.setBold(
        true
        );


    painter.setFont(
        font
        );


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
    {
        return;
    }


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
    {
        return;
    }


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


// =========================================================
// SCALE
// =========================================================

void SkillBox::setScale(
    double newScale
    )
{
    if(newScale <= 0.0)
    {
        newScale =
            1.0;
    }


    scale =
        newScale;


    int width =
        qMax(
            1,
            qRound(
                60.0 * scale
                )
            );


    int height =
        qMax(
            1,
            qRound(
                55.0 * scale
                )
            );


    setFixedSize(
        width,
        height
        );


    update();
}


double SkillBox::getScale() const
{
    return scale;
}


// =========================================================
// GETTERS
// =========================================================

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
