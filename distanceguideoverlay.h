#ifndef DISTANCEGUIDEOVERLAY_H
#define DISTANCEGUIDEOVERLAY_H

#include <QList>
#include <QWidget>

#include "distanceguiderectangle.h"
// #include "distanceguidecircle.h"

class DistanceGuideManager;
class DistanceGuideLine;
class OverlayRoot;
// class DistanceGuideCircle;


class DistanceGuideOverlay :
                             public QWidget
{
    Q_OBJECT

public:

    explicit DistanceGuideOverlay(
        DistanceGuideManager *manager,
        OverlayRoot *root
        );


public slots:

    void setEnabled(
        bool enabled
        );


private:

    void rebuild();

    void clear();


private:

    DistanceGuideManager *m_manager =
        nullptr;


    OverlayRoot *m_root =
        nullptr;


    // =========================
    // OGGETTI DISEGNO
    // =========================

    QList<DistanceGuideLine *>
        m_lines;


    QList<DistanceGuideRectangle *>
        m_rectangles;


    // QList<DistanceGuideCircle *>
    //     m_circles;


    bool m_enabled =
        false;
};


#endif