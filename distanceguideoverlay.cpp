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
            &DistanceGuideManager::groupsChanged,
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
    if(m_enabled == enabled)
    {
        if(m_enabled)
            rebuild();

        return;
    }


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
        !m_root ||
        !m_enabled)
    {
        return;
    }


    clear();


    const int opacity =
        m_manager->globalOpacity();


    const double windowOpacity =
        opacity / 255.0;


    // ==================================================
    // STANDALONE GUIDES
    // ==================================================

    const QList<DistanceGuideConfiguration>
        standaloneGuides =
        m_manager->guides();


    for(const DistanceGuideConfiguration &guide :
         standaloneGuides)
    {
        if(!m_manager->isGuideEffectivelyEnabled(
                guide
                ))
        {
            continue;
        }


        switch(guide.type)
        {

            // ==================================================
            // VERTICAL LINE
            // ==================================================

        case DistanceGuideType::VerticalLine:
        {
            DistanceGuideLine *line =
                new DistanceGuideLine(
                    m_manager->effectiveGuideColor(
                        guide
                        ),
                    this
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


            /*
             * La Line è una Qt::Tool,
             * quindi è una finestra indipendente.
             *
             * Registriamola direttamente nel root.
             */

            m_root->registerOverlay(
                line
                );


            line->show();
            line->raise();


            m_lines.append(
                line
                );


            break;
        }


            // ==================================================
            // RECTANGLE
            // ==================================================

        case DistanceGuideType::Rectangle:
        {
            DistanceGuideRectangle *rectangle =
                new DistanceGuideRectangle(
                    m_manager->effectiveGuideColor(
                        guide
                        ),
                    this
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


            /*
             * Anche il Rectangle è una Qt::Tool.
             */

            m_root->registerOverlay(
                rectangle
                );


            rectangle->show();
            rectangle->raise();


            m_rectangles.append(
                rectangle
                );


            break;
        }


            // ==================================================
            // CIRCLE
            // ==================================================

        case DistanceGuideType::Circle:
        {
            DistanceGuideCircle *circle =
                new DistanceGuideCircle(
                    m_manager->effectiveGuideColor(
                        guide
                        ),
                    this
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


            /*
             * IMPORTANTE:
             *
             * Registriamo anche il Circle standalone.
             */

            m_root->registerOverlay(
                circle
                );


            circle->show();
            circle->raise();


            m_circles.append(
                circle
                );


            break;
        }


        case DistanceGuideType::Group:
        {
            /*
             * Group non è una guida concreta.
             */
            break;
        }
        }
    }


    // ==================================================
    // GROUP GUIDES
    // ==================================================

    const QList<DistanceGuideGroup>
        groups =
        m_manager->groups();


    for(const DistanceGuideGroup &group :
         groups)
    {
        for(const DistanceGuideConfiguration &guide :
             group.guides)
        {
            if(!m_manager->isGuideEffectivelyEnabled(
                    guide
                    ))
            {
                continue;
            }


            switch(guide.type)
            {

                // ==================================================
                // VERTICAL LINE
                // ==================================================

            case DistanceGuideType::VerticalLine:
            {
                DistanceGuideLine *line =
                    new DistanceGuideLine(
                        m_manager->effectiveGuideColor(
                            guide
                            ),
                        this
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


                /*
                 * IMPORTANTE:
                 *
                 * Questa Line appartiene a un Group,
                 * ma è comunque una finestra indipendente.
                 *
                 * La registriamo direttamente.
                 */

                m_root->registerOverlay(
                    line
                    );


                line->show();
                line->raise();


                m_lines.append(
                    line
                    );


                break;
            }


                // ==================================================
                // RECTANGLE
                // ==================================================

            case DistanceGuideType::Rectangle:
            {
                DistanceGuideRectangle *rectangle =
                    new DistanceGuideRectangle(
                        m_manager->effectiveGuideColor(
                            guide
                            ),
                        this
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


                /*
                 * Rectangle appartenente al Group.
                 */

                m_root->registerOverlay(
                    rectangle
                    );


                rectangle->show();
                rectangle->raise();


                m_rectangles.append(
                    rectangle
                    );


                break;
            }


                // ==================================================
                // CIRCLE
                // ==================================================

            case DistanceGuideType::Circle:
            {
                DistanceGuideCircle *circle =
                    new DistanceGuideCircle(
                        m_manager->effectiveGuideColor(
                            guide
                            ),
                        this
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


                /*
                 * QUI è il punto fondamentale:
                 *
                 * ogni Circle del Group viene registrato
                 * individualmente nell'OverlayRoot.
                 *
                 * Se il Group contiene 20 Circle,
                 * OverlayRoot avrà 20 finestre registrate.
                 */

                m_root->registerOverlay(
                    circle
                    );


                circle->show();
                circle->raise();


                m_circles.append(
                    circle
                    );


                break;
            }


            case DistanceGuideType::Group:
            {
                /*
                 * Group non è una guida concreta.
                 */
                break;
            }
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
