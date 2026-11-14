#include "screenmirroroverlay.h"

#include <QPainter>


ScreenMirrorOverlay::ScreenMirrorOverlay(
    QWidget *parent
    )
    : QWidget(parent)
{
    // Come gli altri overlay: finestra senza bordo, sempre in primo
    // piano, con sfondo trasparente.
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(
        Qt::WA_TranslucentBackground
        );
}


void ScreenMirrorOverlay::setRegion(
    const QRect &region
    )
{
    setGeometry(
        region
        );

    update();
}


void ScreenMirrorOverlay::setImage(
    const QImage &image
    )
{
    // RGB32: il canale alfa della cattura non deve rendere
    // trasparente (o semitrasparente) l'immagine mostrata.
    m_image =
        image.convertToFormat(
            QImage::Format_RGB32
            );

    // 1 pixel dell'immagine = 1 pixel dello schermo.
    m_image.setDevicePixelRatio(
        1.0
        );

    update();
}


void ScreenMirrorOverlay::clearImage()
{
    m_image =
        QImage();

    update();
}


const QImage &ScreenMirrorOverlay::image() const
{
    return m_image;
}


void ScreenMirrorOverlay::paintEvent(
    QPaintEvent *
    )
{
    if(m_image.isNull())
    {
        return;
    }

    QPainter painter(
        this
        );

    // Nessuna scalatura: l'angolo in alto a sinistra dell'immagine
    // coincide con quello della regione di destinazione. Cio' che
    // esce dalla finestra viene tagliato dal widget.
    painter.drawImage(
        QPoint(0, 0),
        m_image
        );
}
