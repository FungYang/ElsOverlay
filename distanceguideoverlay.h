#ifndef DISTANCEGUIDEOVERLAY_H
#define DISTANCEGUIDEOVERLAY_H

#include <QList>
#include <QWidget>


class DistanceGuideManager;
class DistanceGuideLine;
class DistanceGuideRectangle;
class DistanceGuideCircle;
class OverlayRoot;


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


    // ==================================================
    // OGGETTI DISEGNO
    // ==================================================

    QList<DistanceGuideLine *>
        m_lines;


    QList<DistanceGuideRectangle *>
        m_rectangles;


    QList<DistanceGuideCircle *>
        m_circles;


    bool m_enabled =
        false;
};


#endif