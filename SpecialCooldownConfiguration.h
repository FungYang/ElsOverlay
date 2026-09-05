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
};


#endif
