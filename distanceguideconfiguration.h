#ifndef DISTANCEGUIDECONFIGURATION_H
#define DISTANCEGUIDECONFIGURATION_H

#include <QColor>
#include <QString>


enum class DistanceGuideType
{
    VerticalLine,
    Rectangle,
    Circle,
    Group
};


enum class DistanceGuideSide
{
    Left,
    Right
};


struct DistanceGuideGroupConfiguration
{
    QString id;

    QString name;

    QColor color =
        Qt::white;
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

    int positionY =
        0;

    int width =
        200;

    int height =
        100;

    DistanceGuideSide side =
        DistanceGuideSide::Left;

    DistanceGuideType type =
        DistanceGuideType::VerticalLine;

    // Empty = oggetto standalone.
    // Valorizzato = oggetto appartenente a un gruppo.
    QString groupId;
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