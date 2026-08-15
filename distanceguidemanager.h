#include "distanceguidemanager.h"

#include <QSettings>
#include <QtGlobal>


DistanceGuideManager::DistanceGuideManager(
    QObject *parent
    )
    : QObject(parent)
{
    load();
}


// ==================================================
// EFFECTIVE GUIDE STATE
// ==================================================

bool DistanceGuideManager::isGuideEffectivelyEnabled(
    const DistanceGuideConfiguration &guide
    ) const
{
    /*
     * La guida deve essere abilitata.
     */
    if(!guide.enabled)
        return false;


    /*
     * Una guida standalone non dipende da alcun gruppo.
     */
    if(guide.groupId.isEmpty())
        return true;


    /*
     * Una guida appartenente a un gruppo è visibile
     * solo se anche il gruppo è abilitato.
     */
    for(const DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != guide.groupId)
            continue;


        return group.enabled;
    }


    /*
     * Guida orfana: non viene disegnata.
     */
    return false;
}


// ==================================================
// EFFECTIVE GUIDE COLOR
// ==================================================

QColor DistanceGuideManager::effectiveGuideColor(
    const DistanceGuideConfiguration &guide
    ) const
{
    /*
     * Le guide di gruppo utilizzano il colore del gruppo.
     */
    if(!guide.groupId.isEmpty())
    {
        for(const DistanceGuideGroup &group :
             m_groups)
        {
            if(group.id != guide.groupId)
                continue;


            return group.color;
        }
    }


    /*
     * Standalone oppure gruppo inesistente:
     * utilizziamo il colore della guida.
     */
    return guide.color;
}


// ==================================================
// EFFECTIVE GUIDE OPACITY
// ==================================================

int DistanceGuideManager::effectiveGuideOpacity(
    const DistanceGuideConfiguration &guide
    ) const
{
    Q_UNUSED(guide);


    /*
     * L'opacità globale viene applicata a tutte le guide.
     *
     * La funzione esiste separatamente perché l'overlay
     * deve utilizzare un unico punto di accesso per
     * ottenere l'opacità effettiva.
     */
    return m_globalOpacity;
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


    /*
     * Quando il personaggio è in movimento viene
     * applicato l'offset di movimento.
     */
    if(m_characterMoving)
    {
        distance +=
            MovementOffset;
    }


    DistanceGuideSide effectiveSide =
        guide.side;


    /*
     * Durante il movimento verso sinistra invertiamo
     * il lato effettivo della guida.
     */
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
// GROUPS
// ==================================================

QList<DistanceGuideGroup>
DistanceGuideManager::groups() const
{
    return m_groups;
}


bool DistanceGuideManager::containsGroup(
    const QString &id
    ) const
{
    for(const DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id == id)
            return true;
    }


    return false;
}


DistanceGuideGroup
DistanceGuideManager::group(
    const QString &id
    ) const
{
    for(const DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id == id)
            return group;
    }


    return DistanceGuideGroup();
}


QString DistanceGuideManager::createGroupId() const
{
    int index =
        0;


    while(containsGroup(
        "group_" +
        QString::number(index)
        ))
    {
        ++index;
    }


    return "group_" +
           QString::number(index);
}


// ==================================================
// ADD GROUP
// ==================================================

bool DistanceGuideManager::addGroup(
    const QString &name,
    const QColor &color
    )
{
    if(name.trimmed().isEmpty())
        return false;


    DistanceGuideGroup newGroup;


    newGroup.id =
        createGroupId();

    newGroup.name =
        name.trimmed();

    newGroup.color =
        color;

    newGroup.enabled =
        true;

    newGroup.guides.clear();


    m_groups.append(
        newGroup
        );


    save();


    emit groupsChanged();


    return true;
}


// ==================================================
// REMOVE GROUP
// ==================================================

bool DistanceGuideManager::removeGroup(
    const QString &id
    )
{
    for(int i = 0;
         i < m_groups.size();
         ++i)
    {
        if(m_groups[i].id != id)
            continue;


        /*
         * Il gruppo è proprietario delle proprie guide.
         *
         * Eliminando il gruppo vengono quindi eliminate
         * anche tutte le sue guide.
         */
        m_groups.removeAt(i);


        save();


        emit groupsChanged();
        emit guidesChanged();


        return true;
    }


    return false;
}


// ==================================================
// UPDATE GROUP
// ==================================================

bool DistanceGuideManager::updateGroup(
    const DistanceGuideGroup &group
    )
{
    if(group.id.isEmpty())
        return false;


    if(group.name.trimmed().isEmpty())
        return false;


    for(DistanceGuideGroup &current :
         m_groups)
    {
        if(current.id != group.id)
            continue;


        current =
            group;


        current.name =
            current.name.trimmed();


        /*
         * Il contenitore determina la proprietà.
         */
        for(DistanceGuideConfiguration &guide :
             current.guides)
        {
            guide.groupId =
                current.id;
        }


        save();


        emit groupsChanged();
        emit guidesChanged();


        return true;
    }


    return false;
}


// ==================================================
// ENABLE GROUP
// ==================================================

bool DistanceGuideManager::setGroupEnabled(
    const QString &id,
    bool enabled
    )
{
    for(DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != id)
            continue;


        if(group.enabled == enabled)
            return true;


        group.enabled =
            enabled;


        save();


        emit groupsChanged();
        emit guidesChanged();


        return true;
    }


    return false;
}


// ==================================================
// GROUP GUIDES
// ==================================================

QList<DistanceGuideConfiguration>
DistanceGuideManager::groupGuides(
    const QString &groupId
    ) const
{
    for(const DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        return group.guides;
    }


    return {};
}


// ==================================================
// CONTAINS GROUP GUIDE
// ==================================================

bool DistanceGuideManager::containsGroupGuide(
    const QString &groupId,
    const QString &guideId
    ) const
{
    for(const DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        for(const DistanceGuideConfiguration &guide :
             group.guides)
        {
            if(guide.id == guideId)
                return true;
        }


        return false;
    }


    return false;
}


// ==================================================
// GET GROUP GUIDE
// ==================================================

DistanceGuideConfiguration
DistanceGuideManager::groupGuide(
    const QString &groupId,
    const QString &guideId
    ) const
{
    for(const DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        for(const DistanceGuideConfiguration &guide :
             group.guides)
        {
            if(guide.id == guideId)
                return guide;
        }


        break;
    }


    return DistanceGuideConfiguration();
}


// ==================================================
// ADD GROUP GUIDE
// ==================================================

bool DistanceGuideManager::addGroupGuide(
    const QString &groupId,
    const QString &name,
    DistanceGuideType type,
    const QColor &color
    )
{
    if(!containsGroup(groupId))
        return false;


    if(name.trimmed().isEmpty())
        return false;


    if(!m_characterCenterConfigured)
        return false;


    if(type == DistanceGuideType::Group)
        return false;


    for(DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        /*
         * Gli ID delle guide di gruppo sono locali
         * al gruppo.
         */
        int index =
            0;


        QString guideId;


        do
        {
            guideId =
                group.id +
                "_guide_" +
                QString::number(index);


            ++index;
        }
        while(containsGroupGuide(
            groupId,
            guideId
            ));


        DistanceGuideConfiguration guide;


        guide.id =
            guideId;

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
            type;

        guide.groupId =
            groupId;


        switch(type)
        {
        case DistanceGuideType::VerticalLine:
        {
            guide.positionY =
                0;

            guide.width =
                5;

            guide.height =
                0;

            break;
        }


        case DistanceGuideType::Rectangle:
        {
            guide.positionY =
                100;

            guide.width =
                200;

            guide.height =
                100;

            break;
        }


        case DistanceGuideType::Circle:
        {
            guide.positionY =
                100;

            guide.width =
                150;

            guide.height =
                150;

            break;
        }


        case DistanceGuideType::Group:
        {
            return false;
        }
        }


        /*
         * La guida viene inserita esclusivamente
         * nel gruppo proprietario.
         */
        group.guides.append(
            guide
            );


        save();


        emit groupsChanged();
        emit guidesChanged();


        return true;
    }


    return false;
}


// ==================================================
// REMOVE GROUP GUIDE
// ==================================================

bool DistanceGuideManager::removeGroupGuide(
    const QString &groupId,
    const QString &guideId
    )
{
    for(DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        for(int i = 0;
             i < group.guides.size();
             ++i)
        {
            if(group.guides[i].id != guideId)
                continue;


            group.guides.removeAt(i);


            save();


            emit groupsChanged();
            emit guidesChanged();


            return true;
        }


        return false;
    }


    return false;
}


// ==================================================
// UPDATE GROUP GUIDE
// ==================================================

bool DistanceGuideManager::updateGroupGuide(
    const QString &groupId,
    const DistanceGuideConfiguration &guide
    )
{
    if(guide.id.isEmpty())
        return false;


    if(guide.name.trimmed().isEmpty())
        return false;


    if(guide.type == DistanceGuideType::Group)
        return false;


    for(DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        for(DistanceGuideConfiguration &current :
             group.guides)
        {
            if(current.id != guide.id)
                continue;


            current =
                guide;


            current.name =
                current.name.trimmed();


            /*
             * La proprietà è determinata dal gruppo
             * che contiene la guida.
             */
            current.groupId =
                groupId;


            save();


            emit groupsChanged();
            emit guidesChanged();


            return true;
        }


        return false;
    }


    return false;
}


// ==================================================
// ENABLE GROUP GUIDE
// ==================================================

bool DistanceGuideManager::setGroupGuideEnabled(
    const QString &groupId,
    const QString &guideId,
    bool enabled
    )
{
    for(DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        for(DistanceGuideConfiguration &guide :
             group.guides)
        {
            if(guide.id != guideId)
                continue;


            if(guide.enabled == enabled)
                return true;


            guide.enabled =
                enabled;


            save();


            emit groupsChanged();
            emit guidesChanged();


            return true;
        }


        return false;
    }


    return false;
}


// ==================================================
// STANDALONE GUIDES
// ==================================================

QList<DistanceGuideConfiguration>
DistanceGuideManager::guides() const
{
    /*
     * SOLO guide standalone.
     *
     * Le guide di gruppo vivono esclusivamente
     * dentro DistanceGuideGroup::guides.
     */
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
// ADD STANDALONE LINE
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

    guide.positionY =
        0;

    guide.width =
        5;

    guide.height =
        0;

    /*
     * Una guida presente in m_guides è sempre standalone.
     */
    guide.groupId.clear();


    m_guides.append(
        guide
        );


    save();


    emit guidesChanged();


    return true;
}


// ==================================================
// ADD STANDALONE RECTANGLE
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

    guide.positionY =
        100;

    guide.width =
        200;

    guide.height =
        100;

    guide.groupId.clear();


    m_guides.append(
        guide
        );


    save();


    emit guidesChanged();


    return true;
}


// ==================================================
// ADD STANDALONE CIRCLE
// ==================================================

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

    guide.groupId.clear();


    m_guides.append(
        guide
        );


    save();


    emit guidesChanged();


    return true;
}


// ==================================================
// REMOVE STANDALONE GUIDE
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
// UPDATE STANDALONE GUIDE
// ==================================================

bool DistanceGuideManager::updateGuide(
    const DistanceGuideConfiguration &guide
    )
{
    if(guide.id.isEmpty())
        return false;


    if(guide.name.trimmed().isEmpty())
        return false;


    /*
     * updateGuide è riservato alle guide standalone.
     */
    if(!guide.groupId.isEmpty())
        return false;


    if(guide.type == DistanceGuideType::Group)
        return false;


    for(DistanceGuideConfiguration &current :
         m_guides)
    {
        if(current.id != guide.id)
            continue;


        current =
            guide;


        current.name =
            current.name.trimmed();


        /*
         * Una guida presente in m_guides è sempre
         * standalone.
         */
        current.groupId.clear();


        save();


        emit guidesChanged();


        return true;
    }


    return false;
}


// ==================================================
// ENABLE STANDALONE GUIDE
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


        if(guide.enabled == enabled)
            return true;


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
// STANDALONE GUIDE -> GROUP
// ==================================================

bool DistanceGuideManager::setGuideGroup(
    const QString &guideId,
    const QString &groupId
    )
{
    /*
     * groupId vuoto:
     *
     * non effettuiamo un detach automatico.
     *
     * Se la guida è già standalone, l'operazione è
     * semplicemente confermata.
     */
    if(groupId.isEmpty())
    {
        for(const DistanceGuideConfiguration &guide :
             m_guides)
        {
            if(guide.id == guideId)
                return true;
        }


        /*
         * Una guida di gruppo non viene trasformata
         * automaticamente in standalone.
         */
        return false;
    }


    /*
     * Il gruppo deve esistere.
     */
    if(!containsGroup(groupId))
        return false;


    /*
     * Cerchiamo esclusivamente una guida standalone.
     */
    int standaloneIndex =
        -1;


    for(int i = 0;
         i < m_guides.size();
         ++i)
    {
        if(m_guides[i].id != guideId)
            continue;


        standaloneIndex =
            i;


        break;
    }


    /*
     * Non è standalone.
     *
     * Se è già nel gruppo richiesto, l'operazione
     * è già soddisfatta.
     */
    if(standaloneIndex < 0)
    {
        if(containsGroupGuide(
                groupId,
                guideId
                ))
        {
            return true;
        }


        return false;
    }


    DistanceGuideConfiguration guide =
        m_guides[standaloneIndex];


    /*
     * L'ID standalone potrebbe essere già utilizzato
     * dal gruppo.
     */
    if(containsGroupGuide(
            groupId,
            guide.id
            ))
    {
        int index =
            0;


        QString newId;


        do
        {
            newId =
                groupId +
                "_guide_" +
                QString::number(index);


            ++index;
        }
        while(containsGroupGuide(
            groupId,
            newId
            ));


        guide.id =
            newId;
    }


    /*
     * Il contenitore cambia.
     */
    guide.groupId =
        groupId;


    for(DistanceGuideGroup &group :
         m_groups)
    {
        if(group.id != groupId)
            continue;


        group.guides.append(
            guide
            );


        /*
         * Rimuoviamo la vecchia istanza standalone.
         */
        m_guides.removeAt(
            standaloneIndex
            );


        save();


        emit groupsChanged();
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


    // ==================================================
    // GLOBAL STATE
    // ==================================================

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


    m_globalOpacity =
        qBound(
            0,
            m_globalOpacity,
            255
            );


    m_characterCenterConfigured =
        settings.value(
                    "DistanceGuides/characterCenterConfigured",
                    false
                    ).toBool();


    // ==================================================
    // GROUPS
    // ==================================================

    m_groups.clear();


    const int groupCount =
        settings.value(
                    "DistanceGuides/groupCount",
                    0
                    ).toInt();


    for(int i = 0;
         i < groupCount;
         ++i)
    {
        const QString prefix =
            "DistanceGuides/Groups/" +
            QString::number(i);


        DistanceGuideGroup group;


        group.id =
            settings.value(
                        prefix + "/id"
                        ).toString();


        if(group.id.isEmpty())
            continue;


        group.name =
            settings.value(
                        prefix + "/name"
                        ).toString();


        group.color =
            QColor(
                settings.value(
                            prefix + "/color",
                            "#ffffff"
                            ).toString()
                );


        group.enabled =
            settings.value(
                        prefix + "/enabled",
                        true
                        ).toBool();


        group.guides.clear();


        // ==================================================
        // GROUP OWNED GUIDES
        // ==================================================

        const int guideCount =
            settings.value(
                        prefix + "/guideCount",
                        0
                        ).toInt();


        for(int g = 0;
             g < guideCount;
             ++g)
        {
            const QString guidePrefix =
                prefix +
                "/Guides/" +
                QString::number(g);


            DistanceGuideConfiguration guide;


            guide.id =
                settings.value(
                            guidePrefix + "/id"
                            ).toString();


            if(guide.id.isEmpty())
                continue;


            guide.name =
                settings.value(
                            guidePrefix + "/name"
                            ).toString();


            guide.color =
                QColor(
                    settings.value(
                                guidePrefix + "/color",
                                "#ffffff"
                                ).toString()
                    );


            guide.enabled =
                settings.value(
                            guidePrefix + "/enabled",
                            true
                            ).toBool();


            guide.distance =
                settings.value(
                            guidePrefix + "/distance",
                            0
                            ).toInt();


            if(guide.distance < 0)
                guide.distance = 0;


            guide.side =
                distanceGuideSideFromString(
                    settings.value(
                                guidePrefix + "/side",
                                "Left"
                                ).toString()
                    );


            guide.type =
                distanceGuideTypeFromString(
                    settings.value(
                                guidePrefix + "/type",
                                "VerticalLine"
                                ).toString()
                    );


            guide.positionY =
                settings.value(
                            guidePrefix + "/positionY",
                            100
                            ).toInt();


            guide.width =
                settings.value(
                            guidePrefix + "/width",
                            200
                            ).toInt();


            guide.height =
                settings.value(
                            guidePrefix + "/height",
                            100
                            ).toInt();


            /*
             * Il contenitore è la fonte di verità.
             */
            guide.groupId =
                group.id;


            group.guides.append(
                guide
                );
        }


        m_groups.append(
            group
            );
    }


    // ==================================================
    // STANDALONE GUIDES
    // ==================================================

    m_guides.clear();


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


        if(guide.distance < 0)
            guide.distance = 0;


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


        /*
         * ==================================================
         * LEGACY MIGRATION
         * ==================================================
         *
         * I vecchi file potevano avere una guida dentro
         * la lista standalone con un groupId.
         *
         * Se il gruppo esiste, la trasferiamo realmente
         * dentro il gruppo.
         */

        const QString legacyGroupId =
            settings.value(
                        prefix + "/groupId",
                        QString()
                        ).toString();


        guide.groupId.clear();


        if(!legacyGroupId.isEmpty() &&
            containsGroup(legacyGroupId))
        {
            guide.groupId =
                legacyGroupId;


            bool alreadyExists =
                containsGroupGuide(
                    legacyGroupId,
                    guide.id
                    );


            if(!alreadyExists)
            {
                for(DistanceGuideGroup &group :
                     m_groups)
                {
                    if(group.id != legacyGroupId)
                        continue;


                    group.guides.append(
                        guide
                        );


                    break;
                }
            }


            /*
             * Non la inseriamo in m_guides.
             */
            continue;
        }


        /*
         * Vera guida standalone.
         */
        guide.groupId.clear();


        m_guides.append(
            guide
            );
    }


    /*
     * Normalizza immediatamente il file dopo una
     * eventuale migrazione.
     */
    save();
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


    /*
     * Ricostruiamo completamente la sezione.
     */
    settings.remove(
        "DistanceGuides"
        );


    // ==================================================
    // GLOBAL STATE
    // ==================================================

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


    // ==================================================
    // GROUPS
    // ==================================================

    settings.setValue(
        "DistanceGuides/groupCount",
        m_groups.size()
        );


    for(int i = 0;
         i < m_groups.size();
         ++i)
    {
        const DistanceGuideGroup &group =
            m_groups[i];


        const QString prefix =
            "DistanceGuides/Groups/" +
            QString::number(i);


        settings.setValue(
            prefix + "/id",
            group.id
            );


        settings.setValue(
            prefix + "/name",
            group.name
            );


        settings.setValue(
            prefix + "/color",
            group.color.name()
            );


        settings.setValue(
            prefix + "/enabled",
            group.enabled
            );


        // ==================================================
        // GROUP OWNED GUIDES
        // ==================================================

        settings.setValue(
            prefix + "/guideCount",
            group.guides.size()
            );


        for(int g = 0;
             g < group.guides.size();
             ++g)
        {
            const DistanceGuideConfiguration &guide =
                group.guides[g];


            const QString guidePrefix =
                prefix +
                "/Guides/" +
                QString::number(g);


            settings.setValue(
                guidePrefix + "/id",
                guide.id
                );


            settings.setValue(
                guidePrefix + "/name",
                guide.name
                );


            settings.setValue(
                guidePrefix + "/color",
                guide.color.name()
                );


            settings.setValue(
                guidePrefix + "/enabled",
                guide.enabled
                );


            settings.setValue(
                guidePrefix + "/distance",
                guide.distance
                );


            settings.setValue(
                guidePrefix + "/side",
                distanceGuideSideToString(
                    guide.side
                    )
                );


            settings.setValue(
                guidePrefix + "/type",
                distanceGuideTypeToString(
                    guide.type
                    )
                );


            settings.setValue(
                guidePrefix + "/positionY",
                guide.positionY
                );


            settings.setValue(
                guidePrefix + "/width",
                guide.width
                );


            settings.setValue(
                guidePrefix + "/height",
                guide.height
                );


            /*
             * Ridondante ma coerente.
             */
            settings.setValue(
                guidePrefix + "/groupId",
                group.id
                );
        }
    }


    // ==================================================
    // STANDALONE GUIDES
    // ==================================================

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


        /*
         * Una guida standalone non ha gruppo.
         */
        settings.setValue(
            prefix + "/groupId",
            QString()
            );
    }


    settings.sync();
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