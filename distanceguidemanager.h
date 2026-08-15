#ifndef DISTANCEGUIDEMANAGER_H
#define DISTANCEGUIDEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QColor>

#include "distanceguideconfiguration.h"
#include "distanceguidegroup.h"


enum class MovementDirection
{
    None,
    Left,
    Right
};


class DistanceGuideManager :
                             public QObject
{
    Q_OBJECT


public:

    explicit DistanceGuideManager(
        QObject *parent = nullptr
        );


    // ==================================================
    // GROUPS
    // ==================================================

    QList<DistanceGuideGroup>
    groups() const;


    bool addGroup(
        const QString &name,
        const QColor &color
        );


    bool removeGroup(
        const QString &id
        );


    bool updateGroup(
        const DistanceGuideGroup &group
        );


    bool containsGroup(
        const QString &id
        ) const;


    DistanceGuideGroup group(
        const QString &id
        ) const;


    // ==================================================
    // GROUP GUIDE MANAGEMENT
    // ==================================================

    QList<DistanceGuideConfiguration>
    groupGuides(
        const QString &groupId
        ) const;


    bool addGroupGuide(
        const QString &groupId,
        const QString &name,
        DistanceGuideType type,
        const QColor &color
        );


    bool removeGroupGuide(
        const QString &groupId,
        const QString &guideId
        );


    bool updateGroupGuide(
        const QString &groupId,
        const DistanceGuideConfiguration &guide
        );


    bool setGroupGuideEnabled(
        const QString &groupId,
        const QString &guideId,
        bool enabled
        );


    bool containsGroupGuide(
        const QString &groupId,
        const QString &guideId
        ) const;


    DistanceGuideConfiguration groupGuide(
        const QString &groupId,
        const QString &guideId
        ) const;


    // ==================================================
    // GROUP ENABLED
    // ==================================================

    bool setGroupEnabled(
        const QString &id,
        bool enabled
        );


    // ==================================================
    // STANDALONE GUIDE / GROUP ASSOCIATION
    // ==================================================

    bool setGuideGroup(
        const QString &guideId,
        const QString &groupId
        );


    QColor effectiveGuideColor(
        const DistanceGuideConfiguration &guide
        ) const;


    // ==================================================
    // GUIDES
    // ==================================================

    QList<DistanceGuideConfiguration>
    guides() const;


    bool addGuide(
        const QString &name,
        const QColor &color
        );


    bool addRectangleGuide(
        const QString &name,
        const QColor &color
        );


    bool addCircleGuide(
        const QString &name,
        const QColor &color
        );



    bool removeGuide(
        const QString &id
        );


    bool updateGuide(
        const DistanceGuideConfiguration &guide
        );


    bool setGuideEnabled(
        const QString &id,
        bool enabled
        );


    bool setGuideDistance(
        const QString &id,
        int distance
        );


    bool setGuideSide(
        const QString &id,
        DistanceGuideSide side
        );


    bool contains(
        const QString &id
        ) const;


    // ==================================================
    // CHARACTER CENTER
    // ==================================================

    int characterCenter() const;


    bool characterCenterConfigured() const;


    void setCharacterCenter(
        int x
        );


    // ==================================================
    // CHARACTER MOVEMENT
    // ==================================================

    bool characterMoving() const;


    void setCharacterMoving(
        bool moving
        );


    // ==================================================
    // MOVEMENT DIRECTION
    // ==================================================

    MovementDirection movementDirection() const;


    void setMovementDirection(
        MovementDirection direction
        );


    // ==================================================
    // GLOBAL OPACITY
    // ==================================================

    int globalOpacity() const;


    void setGlobalOpacity(
        int opacity
        );


    // ==================================================
    // EFFECTIVE POSITION
    // ==================================================

    int effectivePositionX(
        const DistanceGuideConfiguration &guide
        ) const;


signals:

    // ==================================================
    // GUIDES
    // ==================================================

    void guidesChanged();


    // ==================================================
    // GROUPS
    // ==================================================

    void groupsChanged();


    // ==================================================
    // CHARACTER CENTER
    // ==================================================

    void characterCenterChanged(
        int x
        );


    // ==================================================
    // CHARACTER MOVEMENT
    // ==================================================

    void characterMovingChanged(
        bool moving
        );


    // ==================================================
    // MOVEMENT DIRECTION
    // ==================================================

    void movementDirectionChanged(
        MovementDirection direction
        );


    // ==================================================
    // GLOBAL OPACITY
    // ==================================================

    void globalOpacityChanged(
        int opacity
        );


private:

    // ==================================================
    // CONSTANTS
    // ==================================================

    static constexpr int MovementOffset =
        80;


    // ==================================================
    // PERSISTENCE
    // ==================================================

    void load();


    void save() const;


    QString createId() const;


    QString createGroupId() const;


    // ==================================================
    // GUIDES
    // ==================================================

    QList<DistanceGuideConfiguration>
        m_guides;


    // ==================================================
    // GROUPS
    // ==================================================

    QList<DistanceGuideGroup>
        m_groups;


    // ==================================================
    // GLOBAL STATE
    // ==================================================

    int m_characterCenter =
        0;


    bool m_characterCenterConfigured =
        false;


    // ==================================================
    // MOVEMENT STATE
    // ==================================================

    bool m_characterMoving =
        false;


    MovementDirection m_movementDirection =
        MovementDirection::None;


    // ==================================================
    // GLOBAL OPACITY
    // ==================================================

    int m_globalOpacity =
        255;
};


#endif