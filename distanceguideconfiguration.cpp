#include "distanceguideconfiguration.h"


QString distanceGuideTypeToString(
    DistanceGuideType type
    )
{
    switch(type)
    {
    case DistanceGuideType::VerticalLine:
        return "VerticalLine";

    case DistanceGuideType::Rectangle:
        return "Rectangle";

    case DistanceGuideType::Circle:
        return "Circle";
    }


    return "VerticalLine";
}


DistanceGuideType distanceGuideTypeFromString(
    const QString &value
    )
{
    if(value == "Rectangle")
        return DistanceGuideType::Rectangle;


    if(value == "Circle")
        return DistanceGuideType::Circle;


    return DistanceGuideType::VerticalLine;
}


QString distanceGuideSideToString(
    DistanceGuideSide side
    )
{
    switch(side)
    {
    case DistanceGuideSide::Left:
        return "Left";

    case DistanceGuideSide::Right:
        return "Right";
    }


    return "Left";
}


DistanceGuideSide distanceGuideSideFromString(
    const QString &value
    )
{
    if(value == "Right")
    {
        return DistanceGuideSide::Right;
    }


    return DistanceGuideSide::Left;
}