#include "overlayroot.h"

#include <QGuiApplication>
#include <QScreen>


OverlayRoot::OverlayRoot(
    QWidget *parent
    )
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


    QScreen *screen =
        QGuiApplication::primaryScreen();


    if(screen)
    {
        setGeometry(
            screen->geometry()
            );
    }


    m_overlaysVisible =
        true;
}


// ============================================================
// REGISTER OVERLAY
// ============================================================

void OverlayRoot::registerOverlay(
    QWidget *overlay
    )
{
    if(!overlay)
    {
        return;
    }


    if(overlays.contains(
            overlay
            ))
    {
        return;
    }


    overlays.append(
        overlay
        );


    /*
     * Se il global hide è già attivo,
     * una nuova finestra registrata deve partire nascosta.
     *
     * Questo è importante per le Distance Guides,
     * perché Line / Rectangle / Circle sono Qt::Tool
     * e vengono create dinamicamente durante rebuild().
     */

    if(!m_overlaysVisible)
    {
        visibilityBeforeHide.insert(
            overlay,
            false
            );

        overlay->hide();
    }


    /*
     * Se l'overlay viene distrutto, eliminiamolo
     * automaticamente dalle nostre liste.
     */

    connect(
        overlay,
        &QObject::destroyed,
        this,
        [this, overlay]()
        {
            overlays.removeAll(
                overlay
                );

            visibilityBeforeHide.remove(
                overlay
                );
        }
        );
}


// ============================================================
// RAISE ALL
// ============================================================

void OverlayRoot::raiseAll()
{
    if(!m_overlaysVisible)
    {
        return;
    }


    /*
     * OverlayRoot deve rimanere sopra.
     */

    raise();


    /*
     * Portiamo sopra anche gli overlay registrati.
     */

    for(QWidget *overlay : overlays)
    {
        if(!overlay)
        {
            continue;
        }


        if(overlay->isVisible())
        {
            overlay->raise();
        }
    }
}


// ============================================================
// TOGGLE VISIBILITY
// ============================================================

void OverlayRoot::toggleVisibility()
{
    /*
     * ========================================================
     * HIDE
     * ========================================================
     */

    if(m_overlaysVisible)
    {
        m_overlaysVisible =
            false;


        visibilityBeforeHide.clear();


        /*
         * Salviamo lo stato reale di ogni overlay.
         */

        for(QWidget *overlay : overlays)
        {
            if(!overlay)
            {
                continue;
            }


            visibilityBeforeHide.insert(
                overlay,
                overlay->isVisible()
                );


            overlay->hide();
        }


        /*
         * Nascondiamo anche il root.
         */

        hide();


        return;
    }


    /*
     * ========================================================
     * SHOW
     * ========================================================
     */

    m_overlaysVisible =
        true;


    /*
     * Prima mostriamo il root.
     */

    show();


    raise();


    /*
     * Ripristiniamo esattamente la situazione precedente.
     */

    for(QWidget *overlay : overlays)
    {
        if(!overlay)
        {
            continue;
        }


        const bool wasVisible =
            visibilityBeforeHide.value(
                overlay,
                false
                );


        if(wasVisible)
        {
            overlay->show();
            overlay->raise();
        }
    }


    visibilityBeforeHide.clear();
}
