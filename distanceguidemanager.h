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

class DistanceGuideManager : public QObject
{
    Q_OBJECT

public:


    MovementDirection movementDirection() const;

    void setMovementDirection(
        MovementDirection direction
        );

    explicit DistanceGuideManager(
        QObject *parent = nullptr
        );


    // =========================
    // GUIDES
    // =========================

    QList<DistanceGuideConfiguration>
    guides() const;


    bool addGuide(
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


    // =========================
    // CHARACTER CENTER
    // =========================

    int characterCenter() const;


    bool characterCenterConfigured() const;


    void setCharacterCenter(
        int x
        );


    // =========================
    // CHARACTER MOVEMENT
    // =========================

    bool characterMoving() const;


    void setCharacterMoving(
        bool moving
        );


    // =========================
    // EFFECTIVE POSITION
    // =========================

    int effectivePositionX(
        const DistanceGuideConfiguration &guide
        ) const;


signals:

    void guidesChanged();


    void characterCenterChanged(
        int x
        );


    void characterMovingChanged(
        bool moving
        );

    void movementDirectionChanged(
        MovementDirection direction
        );


private:

    static constexpr int MovementOffset =
        80;


    void load();


    void save() const;


    QString createId() const;


    QList<DistanceGuideConfiguration>
        m_guides;


    int m_characterCenter =
        0;


    bool m_characterCenterConfigured =
        false;


    bool m_characterMoving =
        false;

    MovementDirection m_movementDirection =
        MovementDirection::None;
};


#endif