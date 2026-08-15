#include "distanceguideoverlay.h"

#include "distanceguidemanager.h"
#include "distanceguideconfiguration.h"
#include "distanceguideline.h"
#include "distanceguiderectangle.h"
#include "overlayroot.h"


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
                    if(line)
                    {
                        line->setOpacity(
                            opacity
                            );

                        line->setWindowOpacity(
                            windowOpacity
                            );
                    }
                }


                for(DistanceGuideRectangle *rectangle :
                     m_rectangles)
                {
                    if(rectangle)
                    {
                        rectangle->setWindowOpacity(
                            windowOpacity
                            );
                    }
                }
            }
            );
    }
}



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
        if(!guide.enabled)
            continue;


        switch(guide.type)
        {
            // ==================================================
            // LINEA
            // ==================================================

        case DistanceGuideType::VerticalLine:
        {
            DistanceGuideLine *line =
                new DistanceGuideLine(
                    guide.color,
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
                    guide.color,
                    m_root
                    );


            rectangle->setConfigurationMode(
                false
                );


            rectangle->setWindowOpacity(
                opacity / 255.0
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
            // Lo colleghiamo nel prossimo passaggio.

            break;
        }
        }
    }
}



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
}