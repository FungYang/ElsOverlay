#include "distanceguidemanager.h"

#include <QSettings>


DistanceGuideManager::DistanceGuideManager(
    QObject *parent
    )
    : QObject(parent)
{
    load();
}



QList<DistanceGuideConfiguration>
DistanceGuideManager::guides() const
{
    return m_guides;
}



bool DistanceGuideManager::contains(
    const QString &id
    ) const
{
    for(const DistanceGuideConfiguration &guide :
         m_guides)
    {
        if(guide.id == id)
            return true;
    }


    return false;
}



QString DistanceGuideManager::createId() const
{
    int index = 0;


    while(contains(
        "guide_" +
        QString::number(index)
        ))
    {
        ++index;
    }


    return "guide_" +
           QString::number(index);
}



bool DistanceGuideManager::addGuide(
    const QString &name,
    const QColor &color
    )
{
    if(name.trimmed().isEmpty())
        return false;


    if(!m_characterCenterConfigured)
        return false;


    DistanceGuideConfiguration guide;


    guide.id =
        createId();


    guide.name =
        name.trimmed();


    guide.color =
        color;


    guide.enabled =
        true;


    guide.distance =
        0;


    guide.side =
        DistanceGuideSide::Left;


    guide.type =
        DistanceGuideType::VerticalLine;


    m_guides.append(
        guide
        );


    save();


    emit guidesChanged();


    return true;
}



bool DistanceGuideManager::removeGuide(
    const QString &id
    )
{
    for(int i = 0;
         i < m_guides.size();
         ++i)
    {
        if(m_guides[i].id != id)
            continue;


        m_guides.removeAt(i);


        save();


        emit guidesChanged();


        return true;
    }


    return false;
}



bool DistanceGuideManager::updateGuide(
    const DistanceGuideConfiguration &guide
    )
{
    for(DistanceGuideConfiguration &current :
         m_guides)
    {
        if(current.id != guide.id)
            continue;


        current =
            guide;


        save();


        emit guidesChanged();


        return true;
    }


    return false;
}



bool DistanceGuideManager::setGuideEnabled(
    const QString &id,
    bool enabled
    )
{
    for(DistanceGuideConfiguration &guide :
         m_guides)
    {
        if(guide.id != id)
            continue;


        guide.enabled =
            enabled;


        save();


        emit guidesChanged();


        return true;
    }


    return false;
}



bool DistanceGuideManager::setGuideDistance(
    const QString &id,
    int distance
    )
{
    if(distance < 0)
        return false;


    for(DistanceGuideConfiguration &guide :
         m_guides)
    {
        if(guide.id != id)
            continue;


        guide.distance =
            distance;


        save();


        emit guidesChanged();


        return true;
    }


    return false;
}



bool DistanceGuideManager::setGuideSide(
    const QString &id,
    DistanceGuideSide side
    )
{
    for(DistanceGuideConfiguration &guide :
         m_guides)
    {
        if(guide.id != id)
            continue;


        guide.side =
            side;


        save();


        emit guidesChanged();


        return true;
    }


    return false;
}



void DistanceGuideManager::load()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );

    m_characterCenter =
        settings.value(
                    "DistanceGuides/characterCenter",
                    0
                    ).toInt();


    m_characterCenterConfigured =
        settings.value(
                    "DistanceGuides/characterCenterConfigured",
                    false
                    ).toBool();


    const int count =
        settings.value(
                    "DistanceGuides/count",
                    0
                    ).toInt();


    for(int i = 0;
         i < count;
         ++i)
    {
        const QString prefix =
            "DistanceGuides/" +
            QString::number(i);


        DistanceGuideConfiguration guide;


        guide.id =
            settings.value(
                        prefix + "/id"
                        ).toString();


        guide.name =
            settings.value(
                        prefix + "/name"
                        ).toString();


        guide.color =
            QColor(
                settings.value(
                            prefix + "/color",
                            "#ffffff"
                            ).toString()
                );


        guide.enabled =
            settings.value(
                        prefix + "/enabled",
                        true
                        ).toBool();


        guide.distance =
            settings.value(
                        prefix + "/distance",
                        0
                        ).toInt();


        guide.side =
            distanceGuideSideFromString(
                settings.value(
                            prefix + "/side",
                            "Left"
                            ).toString()
                );


        guide.type =
            distanceGuideTypeFromString(
                settings.value(
                            prefix + "/type",
                            "VerticalLine"
                            ).toString()
                );


        if(guide.id.isEmpty())
            continue;


        m_guides.append(
            guide
            );
    }
}



void DistanceGuideManager::save() const
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    settings.remove(
        "DistanceGuides"
        );


    settings.setValue(
        "DistanceGuides/characterCenter",
        m_characterCenter
        );


    settings.setValue(
        "DistanceGuides/characterCenterConfigured",
        m_characterCenterConfigured
        );


    settings.setValue(
        "DistanceGuides/count",
        m_guides.size()
        );


    for(int i = 0;
         i < m_guides.size();
         ++i)
    {
        const DistanceGuideConfiguration &guide =
            m_guides[i];


        const QString prefix =
            "DistanceGuides/" +
            QString::number(i);


        settings.setValue(
            prefix + "/id",
            guide.id
            );


        settings.setValue(
            prefix + "/name",
            guide.name
            );


        settings.setValue(
            prefix + "/color",
            guide.color.name()
            );


        settings.setValue(
            prefix + "/enabled",
            guide.enabled
            );


        settings.setValue(
            prefix + "/distance",
            guide.distance
            );


        settings.setValue(
            prefix + "/side",
            distanceGuideSideToString(
                guide.side
                )
            );


        settings.setValue(
            prefix + "/type",
            distanceGuideTypeToString(
                guide.type
                )
            );
    }
}



int DistanceGuideManager::characterCenter() const
{
    return m_characterCenter;
}



bool DistanceGuideManager::characterCenterConfigured() const
{
    return m_characterCenterConfigured;
}



void DistanceGuideManager::setCharacterCenter(
    int x
    )
{
    m_characterCenter =
        x;


    m_characterCenterConfigured =
        true;


    save();


    emit characterCenterChanged(
        x
        );


    emit guidesChanged();
}


bool DistanceGuideManager::characterMoving() const
{
    return m_characterMoving;
}



void DistanceGuideManager::setCharacterMoving(
    bool moving
    )
{
    if(m_characterMoving == moving)
        return;


    m_characterMoving =
        moving;


    emit characterMovingChanged(
        moving
        );


    emit guidesChanged();
}



int DistanceGuideManager::effectivePositionX(
    const DistanceGuideConfiguration &guide
    ) const
{
    int distance =
        guide.distance;


    if(m_characterMoving)
    {
        distance +=
            MovementOffset;
    }


    if(m_movementDirection ==
        MovementDirection::Left)
    {
        return m_characterCenter -
               distance;
    }


    if(m_movementDirection ==
        MovementDirection::Right)
    {
        return m_characterCenter +
               distance;
    }


    // Nessuna direzione ancora impostata:
    // usa quella configurata nella guida.

    if(guide.side ==
        DistanceGuideSide::Left)
    {
        return m_characterCenter -
               distance;
    }


    return m_characterCenter +
           distance;
}

MovementDirection
DistanceGuideManager::movementDirection() const
{
    return m_movementDirection;
}



void DistanceGuideManager::setMovementDirection(
    MovementDirection direction
    )
{
    if(m_movementDirection == direction)
        return;


    m_movementDirection =
        direction;


    emit movementDirectionChanged(
        direction
        );


    emit guidesChanged();
}