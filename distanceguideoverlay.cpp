#include "distanceguideoverlay.h"

#include "distanceguidemanager.h"
#include "distanceguideconfiguration.h"
#include "distanceguideline.h"
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
            &DistanceGuideManager::characterCenterChanged,
            this,
            [this](int)
            {
                if(!m_enabled)
                    return;

                rebuild();
            }
            );


        connect(
            m_manager,
            &DistanceGuideManager::characterMovingChanged,
            this,
            [this](bool)
            {
                if(!m_enabled)
                    return;

                rebuild();
            }
            );

        connect(
            m_manager,
            &DistanceGuideManager::guidesChanged,
            this,
            [this]()
            {
                if(!m_enabled)
                    return;

                clear();
                rebuild();
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


    for(const DistanceGuideConfiguration &guide :
         guides)
    {
        if(!guide.enabled)
            continue;


        switch(guide.type)
        {
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



            line->setGeometry(
                m_manager->effectivePositionX(guide),
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
        }
    }
}


void DistanceGuideOverlay::clear()
{
    qDeleteAll(
        m_lines
        );


    m_lines.clear();
}