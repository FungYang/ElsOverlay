#include "distanceguideoverlay.h"

#include "distanceguidemanager.h"
#include "distanceguideconfiguration.h"
#include "distanceguidegroup.h"
#include "distanceguideline.h"
#include "distanceguiderectangle.h"
#include "distanceguidecircle.h"
#include "overlayroot.h"

#include <QtGlobal>
#include <QtAlgorithms>


DistanceGuideOverlay::DistanceGuideOverlay(
    DistanceGuideManager *manager,
    OverlayRoot *root
    )
    : QWidget(root),
    m_manager(manager),
    m_root(root)
{
    setAttribute(
        Qt::WA_TransparentForMouseEvents
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Tool |
        Qt::WindowStaysOnTopHint
        );


    if(m_root)
    {
        setGeometry(
            m_root->rect()
            );
    }


    if(m_manager)
    {
        connect(
            m_manager,
            &DistanceGuideManager::guidesChanged,
            this,
            [this]()
            {
                if(!m_enabled)
                    return;


                rebuild();
            }
            );


        connect(
            m_manager,
            &DistanceGuideManager::globalOpacityChanged,
            this,
            [this](int opacity)
            {
                const double windowOpacity =
                    opacity / 255.0;


                for(DistanceGuideLine *line :
                     m_lines)
                {
                    if(!line)
                        continue;


                    line->setOpacity(
                        opacity
                        );


                    line->setWindowOpacity(
                        windowOpacity
                        );
                }


                for(DistanceGuideRectangle *rectangle :
                     m_rectangles)
                {
                    if(!rectangle)
                        continue;


                    rectangle->setWindowOpacity(
                        windowOpacity
                        );
                }


                for(DistanceGuideCircle *circle :
                     m_circles)
                {
                    if(!circle)
                        continue;


                    circle->setWindowOpacity(
                        windowOpacity
                        );
                }
            }
            );
    }
}


// ==================================================
// ENABLE
// ==================================================

void DistanceGuideOverlay::setEnabled(
    bool enabled
    )
{
    m_enabled =
        enabled;


    clear();


    if(!m_enabled)
        return;


    rebuild();
}


// ==================================================
// REBUILD
// ==================================================

void DistanceGuideOverlay::rebuild()
{
    if(!m_manager ||
        !m_root)
    {
        return;
    }


    clear();


    const QList<DistanceGuideConfiguration>
        guides =
        m_manager->guides();


    const int opacity =
        m_manager->globalOpacity();


    const double windowOpacity =
        opacity / 255.0;


    for(const DistanceGuideConfiguration &guide :
         guides)
    {
        // ------------------------------------------
        // GUIDA DISABILITATA
        // ------------------------------------------

        if(!guide.enabled)
            continue;


        // ------------------------------------------
        // GRUPPO
        // ------------------------------------------

        if(!guide.groupId.isEmpty())
        {
            const DistanceGuideGroup group =
                m_manager->group(
                    guide.groupId
                    );


            if(group.id.isEmpty())
                continue;


            if(!group.enabled)
                continue;
        }


        // ------------------------------------------
        // TIPO GUIDA
        // ------------------------------------------

        switch(guide.type)
        {

            // ==================================================
            // LINEA VERTICALE
            // ==================================================

        case DistanceGuideType::VerticalLine:
        {
            DistanceGuideLine *line =
                new DistanceGuideLine(
                    m_manager->effectiveGuideColor(
                        guide
                        ),
                    m_root
                    );


            line->setConfigurationMode(
                false
                );


            line->setOpacity(
                opacity
                );


            line->setWindowOpacity(
                windowOpacity
                );


            const int x =
                m_manager->effectivePositionX(
                    guide
                    );


            line->setGeometry(
                x,
                0,
                5,
                m_root->height()
                );


            line->show();
            line->raise();


            m_lines.append(
                line
                );


            break;
        }


            // ==================================================
            // RETTANGOLO
            // ==================================================

        case DistanceGuideType::Rectangle:
        {
            DistanceGuideRectangle *rectangle =
                new DistanceGuideRectangle(
                    m_manager->effectiveGuideColor(
                        guide
                        ),
                    m_root
                    );


            rectangle->setConfigurationMode(
                false
                );


            rectangle->setWindowOpacity(
                windowOpacity
                );


            const int centerX =
                m_manager->effectivePositionX(
                    guide
                    );


            const int width =
                qMax(
                    20,
                    guide.width
                    );


            const int height =
                qMax(
                    20,
                    guide.height
                    );


            const int x =
                centerX -
                width / 2;


            const int y =
                guide.positionY;


            rectangle->setGeometry(
                x,
                y,
                width,
                height
                );


            rectangle->show();
            rectangle->raise();


            m_rectangles.append(
                rectangle
                );


            break;
        }


            // ==================================================
            // CERCHIO
            // ==================================================

        case DistanceGuideType::Circle:
        {
            DistanceGuideCircle *circle =
                new DistanceGuideCircle(
                    m_manager->effectiveGuideColor(
                        guide
                        ),
                    m_root
                    );


            circle->setConfigurationMode(
                false
                );


            circle->setWindowOpacity(
                windowOpacity
                );


            const int centerX =
                m_manager->effectivePositionX(
                    guide
                    );


            const int centerY =
                guide.positionY;


            const int size =
                qMax(
                    20,
                    guide.width
                    );


            circle->setGeometry(
                0,
                0,
                size,
                size
                );


            circle->move(
                centerX -
                    size / 2,
                centerY -
                    size / 2
                );


            circle->show();
            circle->raise();


            m_circles.append(
                circle
                );


            break;
        }
        }
    }
}


// ==================================================
// CLEAR
// ==================================================

void DistanceGuideOverlay::clear()
{
    qDeleteAll(
        m_lines
        );


    m_lines.clear();


    qDeleteAll(
        m_rectangles
        );


    m_rectangles.clear();


    qDeleteAll(
        m_circles
        );


    m_circles.clear();
}