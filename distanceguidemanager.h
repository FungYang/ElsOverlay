#ifndef DISTANCEGUIDEMANAGER_H
#define DISTANCEGUIDEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>

#include "distanceguideconfiguration.h"


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
    //
    // 0   = completamente trasparente
    // 255 = completamente opaco
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


    // ==================================================
    // GUIDES
    // ==================================================

    QList<DistanceGuideConfiguration>
        m_guides;


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