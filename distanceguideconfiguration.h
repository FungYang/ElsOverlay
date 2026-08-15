#ifndef DISTANCEGUIDECONFIGURATION_H
#define DISTANCEGUIDECONFIGURATION_H

#include <QColor>
#include <QString>


enum class DistanceGuideType
{
    VerticalLine,
    Rectangle,
    Circle
};


enum class DistanceGuideSide
{
    Left,
    Right
};


struct DistanceGuideConfiguration
{
    QString id;

    QString name;

    QColor color;

    bool enabled =
        true;

    int distance =
        0;

    DistanceGuideSide side =
        DistanceGuideSide::Left;

    DistanceGuideType type =
        DistanceGuideType::VerticalLine;
};


QString distanceGuideTypeToString(
    DistanceGuideType type
    );


DistanceGuideType distanceGuideTypeFromString(
    const QString &value
    );


QString distanceGuideSideToString(
    DistanceGuideSide side
    );


DistanceGuideSide distanceGuideSideFromString(
    const QString &value
    );


#endif