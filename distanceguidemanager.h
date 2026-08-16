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
    Left,
    Right
};
enum class CharacterFacing
{
    Right,
    Left
};


class DistanceGuideManager : public QObject
{
    Q_OBJECT

public:

    explicit DistanceGuideManager(
        QObject *parent = nullptr
        );


    // ==================================================
    // GUIDES
    // ==================================================

    QList<DistanceGuideConfiguration>
    guides() const;


    bool contains(
        const QString &id
        ) const;


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


    // ==================================================
    // GROUPS
    // ==================================================

    QList<DistanceGuideGroup>
    groups() const;


    bool containsGroup(
        const QString &id
        ) const;


    DistanceGuideGroup
    group(
        const QString &id
        ) const;


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


    bool setGroupEnabled(
        const QString &id,
        bool enabled
        );


    // ==================================================
    // GROUP GUIDES
    // ==================================================

    QList<DistanceGuideConfiguration>
    groupGuides(
        const QString &groupId
        ) const;


    bool containsGroupGuide(
        const QString &groupId,
        const QString &guideId
        ) const;


    DistanceGuideConfiguration
    groupGuide(
        const QString &groupId,
        const QString &guideId
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


    // ==================================================
    // GUIDE -> GROUP
    // ==================================================

    bool setGuideGroup(
        const QString &guideId,
        const QString &groupId
        );


    // ==================================================
    // EFFECTIVE VALUES
    // ==================================================

    bool isGuideEffectivelyEnabled(
        const DistanceGuideConfiguration &guide
        ) const;


    QColor effectiveGuideColor(
        const DistanceGuideConfiguration &guide
        ) const;


    int effectiveGuideOpacity(
        const DistanceGuideConfiguration &guide
        ) const;


    int effectivePositionX(
        const DistanceGuideConfiguration &guide
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

    MovementDirection
    movementDirection() const;


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

    CharacterFacing characterFacing() const;

    void setCharacterFacing(
        CharacterFacing facing
        );


signals:

    void guidesChanged();


    void groupsChanged();


    void characterCenterChanged(
        int x
        );


    void characterMovingChanged(
        bool moving
        );


    void movementDirectionChanged(
        MovementDirection direction
        );


    void globalOpacityChanged(
        int opacity
        );


private:

    // ==================================================
    // IDENTIFIERS
    // ==================================================

    QString createId() const;


    QString createGroupId() const;


    // ==================================================
    // PERSISTENCE
    // ==================================================

    void load();


    void save() const;


private:

    CharacterFacing m_characterFacing =
        CharacterFacing::Right;

    // ==================================================
    // STANDALONE GUIDES
    // ==================================================

    QList<DistanceGuideConfiguration>
        m_guides;


    // ==================================================
    // GROUPS
    // ==================================================

    QList<DistanceGuideGroup>
        m_groups;


    // ==================================================
    // CHARACTER CENTER
    // ==================================================

    int m_characterCenter =
        0;


    bool m_characterCenterConfigured =
        false;


    // ==================================================
    // MOVEMENT
    // ==================================================

    bool m_characterMoving =
        false;


    MovementDirection m_movementDirection =
        MovementDirection::Right;


    // ==================================================
    // GLOBAL OPACITY
    // ==================================================

    int m_globalOpacity =
        255;


    // ==================================================
    // MOVEMENT OFFSET
    // ==================================================

    static constexpr int MovementOffset = 100;
};

#endif // DISTANCEGUIDEMANAGER_H