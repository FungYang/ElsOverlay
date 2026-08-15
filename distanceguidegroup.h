#ifndef DISTANCEGUIDEGROUP_H
#define DISTANCEGUIDEGROUP_H

#include <QColor>
#include <QString>
#include <QList>

#include "distanceguideconfiguration.h"


struct DistanceGuideGroup
{
    QString id;

    QString name;

    QColor color;

    bool enabled =
        true;

    // ==================================================
    // GUIDE PROPRIE DEL GRUPPO
    // ==================================================

    QList<DistanceGuideConfiguration> guides;
};


#endif