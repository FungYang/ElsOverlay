#ifndef DISTANCEGUIDEOVERLAY_H
#define DISTANCEGUIDEOVERLAY_H

#include <QWidget>
#include <QList>

class DistanceGuideManager;
class DistanceGuideLine;
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


    QList<DistanceGuideLine *> m_lines;


    bool m_enabled =
        false;
};


#endif