#include "skillbox.h"

#include <QPainter>
#include <QImage>
#include <QFont>


SkillBox::SkillBox(
    const QString &imagePath,
    const QString &skillName,
    double cooldownTime,
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
        0.0;


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
    // GLOW SELEZIONE
    // =====================================================

    if(selected && !drawImage.isNull())
    {
        QRect imageRect(
            imageX,
            imageY,
            imageWidth,
            imageHeight
            );


        QColor glowColor(
            0,
            255,
            0,
            45
            );


        for(int i = 6;
             i >= 1;
             --i)
        {
            painter.setPen(
                QPen(
                    glowColor,
                    i * 2
                    )
                );


            painter.setBrush(
                Qt::NoBrush
                );


            painter.drawRoundedRect(
                imageRect.adjusted(
                    -i,
                    -i,
                    i,
                    i
                    ),
                4,
                4
                );
        }
    }


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

    if(!activeCooldown)
    {
        painter.setPen(
            Qt::white
            );
    }
    else if(currentCooldown > 3.0)
    {
        painter.setPen(
            QColor(
                0,
                0,
                255
                )
            );
    }
    else
    {
        painter.setPen(
            QColor(
                255,
                0,
                0
                )
            );
    }


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
            ? QString::number(
                  currentCooldown,
                  'f',
                  1
                  )
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


    if(cooldownPaused)
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
        0.0;


    update();
}


void SkillBox::tick()
{
    if(!activeCooldown)
    {
        return;
    }


    if(cooldownPaused)
    {
        return;
    }


    currentCooldown -= 0.1;


    if(currentCooldown <= 0.0)
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
    double cooldownTime
    )
{
    cooldown =
        qMax(
            0.0,
            cooldownTime
            );
}


void SkillBox::setSelected(
    bool value
    )
{
    selected =
        value;


    update();
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


double SkillBox::getCooldown() const
{
    return cooldown;
}


// =========================================================
// PAUSE / RESUME
// =========================================================

void SkillBox::pauseCooldown()
{
    cooldownPaused =
        true;
}


void SkillBox::resumeCooldown()
{
    cooldownPaused =
        false;
}