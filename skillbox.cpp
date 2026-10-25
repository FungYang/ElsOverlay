#include "skillbox.h"

#include <QPainter>
#include <QImage>
#include <QFont>
#include <QMouseEvent>


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


    // ========================================================
    // FINESTRA INDIPENDENTE
    //
    // Ogni box e' una finestra a se' (come i BuffBox): cosi' si
    // puo' trascinare ovunque senza essere vincolata agli altri.
    // ========================================================

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(
        Qt::WA_TranslucentBackground
        );

    // IMPORTANTE: niente WA_TransparentForMouseEvents.
    // Prima il mouse passava alla finestra madre (che si spostava
    // intera); ora il box deve riceverlo per potersi trascinare.
    // Il click-through globale e' gestito da OverlayRoot.
    setCursor(
        Qt::SizeAllCursor
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
    painter.save();

    painter.setOpacity(
        transparency / 255.0
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

    painter.restore();

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
                255,
                255,
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
    // Titolo disattivato dalla configurazione: nessun cooldown.
    if(!titleEnabled)
        return;


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
    // Un titolo disattivato non puo' avere il glow di selezione,
    // anche se e' quello "selezionato" in gioco.
    selected =
        value && titleEnabled;


    update();
}


void SkillBox::setTitleEnabled(
    bool enabled
    )
{
    if(titleEnabled == enabled)
        return;


    titleEnabled =
        enabled;


    if(!titleEnabled)
    {
        // Niente cooldown "fantasma" ne' glow residuo.
        resetCooldown();

        selected =
            false;
    }

    // Un titolo disattivato sparisce subito. Il contrario NON lo
    // mostra qui: ora il box e' una finestra a se' e deve comparire
    // solo se l'overlay (Buff Titles ON, overlay non nascosti) e'
    // visibile: ci pensa SkillOverlay::applyEnabledState().
    if(!titleEnabled)
    {
        hide();
    }


    update();
}


bool SkillBox::isTitleEnabled() const
{
    return titleEnabled;
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

void SkillBox::setTransparency(
    int value
    )
{
    transparency =
        qBound(
            0,
            value,
            255
            );

    update();
}


// =========================================================
// TRASCINAMENTO
// =========================================================

void SkillBox::mousePressEvent(
    QMouseEvent *event
    )
{
    if(event->button() == Qt::LeftButton)
    {
        dragging =
            true;

        dragOffset =
            event->globalPosition().toPoint()
            -
            frameGeometry().topLeft();

        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}


void SkillBox::mouseMoveEvent(
    QMouseEvent *event
    )
{
    if(dragging &&
        (event->buttons() & Qt::LeftButton))
    {
        move(
            event->globalPosition().toPoint()
            -
            dragOffset
            );

        event->accept();
        return;
    }

    QWidget::mouseMoveEvent(event);
}


void SkillBox::mouseReleaseEvent(
    QMouseEvent *event
    )
{
    if(dragging &&
        event->button() == Qt::LeftButton)
    {
        dragging =
            false;

        // La posizione si salva solo a fine trascinamento
        // (non a ogni pixel come faceva la finestra unica).
        emit dragFinished(
            pos()
            );

        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}
