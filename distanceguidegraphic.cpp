#include "distanceguidegraphic.h"


DistanceGuideGraphic::DistanceGuideGraphic(
    QWidget *parent
    )
    : QWidget(parent)
{
}


void DistanceGuideGraphic::setConfigurationMode(
    bool enabled
    )
{
    m_configurationMode =
        enabled;
}