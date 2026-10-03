#pragma once

#include <QImage>
#include <QRect>
#include <QString>
#include <QPoint>
#include <QSize>
#include <QtGlobal>

struct CustomSearcherTemplate
{
    int id = -1;

    // Immagine usata dal MultiFinder.
    QImage templateImage;

    // Immagine mostrata dall'overlay quando viene trovato.
    QImage displayImage;

    // Cooldown individuale.
    int cooldownMs = 60000;

    // Posizione/dimensione dell'overlay associato.
    QRect overlayGeometry;

    // Stato runtime, NON necessariamente da salvare.
    qint64 cooldownUntilMs = 0;

    bool enabled = true;

    bool isOnCooldown(qint64 now) const
    {
        return cooldownUntilMs > now;
    }
};