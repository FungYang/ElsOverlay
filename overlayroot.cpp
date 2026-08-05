#include "overlayroot.h"


OverlayRoot::OverlayRoot(QWidget *parent)
    : QWidget(parent)
{

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setAttribute(
        Qt::WA_TransparentForMouseEvents
        );


    resize(
        1920,
        1080
        );

}



void OverlayRoot::registerOverlay(QWidget *overlay)
{

    if(!overlay)
        return;


    overlays.append(
        overlay
        );

}



void OverlayRoot::raiseAll()
{

    raise();


    for(QWidget *w : overlays)
    {

        if(w && w->isVisible())
        {
            w->raise();
        }

    }

}