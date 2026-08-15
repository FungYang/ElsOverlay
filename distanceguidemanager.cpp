#include "distanceguidemanager.h"

#include <QSettings>


DistanceGuideManager::DistanceGuideManager(
    QObject *parent
    )
    : QObject(parent)
{
    load();
}


// ==================================================
// GUIDES
// ==================================================

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
    int index =
        0;

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


// ==================================================
// ADD LINE
// ==================================================

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


    // Valori non utilizzati dalla linea
    guide.positionY =
        0;

    guide.width =
        5;

    guide.height =
        0;


    m_guides.append(
        guide
        );


    save();

    emit guidesChanged();


    return true;
}


// ==================================================
// ADD RECTANGLE
// ==================================================

bool DistanceGuideManager::addRectangleGuide(
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
        DistanceGuideType::Rectangle;


    // Posizione verticale iniziale
    guide.positionY =
        100;

    // Dimensioni iniziali
    guide.width =
        200;

    guide.height =
        100;


    m_guides.append(
        guide
        );


    save();

    emit guidesChanged();


    return true;
}


// ==================================================
// REMOVE
// ==================================================

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


// ==================================================
// UPDATE
// ==================================================

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


// ==================================================
// ENABLE
// ==================================================

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


// ==================================================
// DISTANCE
// ==================================================

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


// ==================================================
// SIDE
// ==================================================

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


// ==================================================
// LOAD
// ==================================================

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


    m_globalOpacity =
        settings.value(
                    "DistanceGuides/globalOpacity",
                    255
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


    m_guides.clear();


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


        if(guide.id.isEmpty())
            continue;


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


        guide.positionY =
            settings.value(
                        prefix + "/positionY",
                        100
                        ).toInt();


        guide.width =
            settings.value(
                        prefix + "/width",
                        200
                        ).toInt();


        guide.height =
            settings.value(
                        prefix + "/height",
                        100
                        ).toInt();


        m_guides.append(
            guide
            );
    }
}


// ==================================================
// SAVE
// ==================================================

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
        "DistanceGuides/globalOpacity",
        m_globalOpacity
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


        settings.setValue(
            prefix + "/positionY",
            guide.positionY
            );


        settings.setValue(
            prefix + "/width",
            guide.width
            );


        settings.setValue(
            prefix + "/height",
            guide.height
            );
    }
}


// ==================================================
// CHARACTER CENTER
// ==================================================

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


// ==================================================
// CHARACTER MOVEMENT
// ==================================================

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


// ==================================================
// EFFECTIVE POSITION
// ==================================================

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


    DistanceGuideSide effectiveSide =
        guide.side;


    if(m_movementDirection ==
        MovementDirection::Left)
    {
        if(effectiveSide ==
            DistanceGuideSide::Left)
        {
            effectiveSide =
                DistanceGuideSide::Right;
        }
        else
        {
            effectiveSide =
                DistanceGuideSide::Left;
        }
    }


    if(effectiveSide ==
        DistanceGuideSide::Left)
    {
        return m_characterCenter -
               distance;
    }


    return m_characterCenter +
           distance;
}


// ==================================================
// MOVEMENT DIRECTION
// ==================================================

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


// ==================================================
// GLOBAL OPACITY
// ==================================================

int DistanceGuideManager::globalOpacity() const
{
    return m_globalOpacity;
}


void DistanceGuideManager::setGlobalOpacity(
    int opacity
    )
{
    opacity =
        qBound(
            0,
            opacity,
            255
            );


    if(m_globalOpacity == opacity)
        return;


    m_globalOpacity =
        opacity;


    save();


    emit globalOpacityChanged(
        opacity
        );

    emit guidesChanged();
}

bool DistanceGuideManager::addCircleGuide(
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
        DistanceGuideType::Circle;


    guide.positionY =
        100;


    guide.width =
        150;


    guide.height =
        150;


    m_guides.append(
        guide
        );


    save();


    emit guidesChanged();


    return true;
}