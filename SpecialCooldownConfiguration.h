#ifndef SPECIALCOOLDOWNCONFIGURATION_H
#define SPECIALCOOLDOWNCONFIGURATION_H

#include <QPoint>
#include <QSize>
#include <QString>
#include <QList>


struct SpecialCooldownConfiguration
{
    int key = 0;

    QList<int> acceptedKeys;

    QString imagePath;

    int cooldown = 0;

    QPoint position;

    QSize size = QSize(
        42,
        42
        );

    bool visible = true;

    // LOOP: premendo il tasto il cooldown parte e, appena arriva a 0,
    // RICOMINCIA da solo (contatore ciclico) finche' non si preme il
    // tasto Reset. Ripremere il tasto lo riallinea (riparte da capo).
    // Default false: le vecchie configurazioni non cambiano.
    bool loop = false;
};


#endif
