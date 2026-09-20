#include "overlayroot.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QScreen>
#include <windows.h>
#include "skilloverlay.h"
#include "buffvisionoverlay.h"
#include "specialcooldownoverlay.h"


// ============================================================
// CONSTRUCTOR
// ============================================================

OverlayRoot::OverlayRoot(
    QWidget *parent
    )
    : QWidget(parent)
{
    setWindowFlags(
        Qt::FramelessWindowHint |
        Qt::Tool |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(
        Qt::WA_TransparentForMouseEvents,
        true
        );

    setAttribute(
        Qt::WA_TranslucentBackground,
        true
        );

    setGeometry(
        QApplication::primaryScreen()->geometry()
        );

    show();

    raise();
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


    if(overlays.contains(overlay))
    {
        return;
    }


    overlays.append(
        overlay
        );


    // ========================================================
    // CLICKABILITÀ
    // ========================================================

    applyClickableState(
        overlay
        );


    // ========================================================
    // RIMOZIONE AUTOMATICA
    // ========================================================

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


    // ========================================================
    // SE GLOBAL HIDE È ATTIVO
    // ========================================================

    if(!m_overlaysVisible)
    {
        visibilityBeforeHide.insert(
            overlay,
            false
            );

        overlay->hide();

        return;
    }
}


// ============================================================
// APPLY CLICKABLE STATE
// ============================================================


void OverlayRoot::applyClickableState(
    QWidget *overlay
    )
{
    if(!overlay)
    {
        return;
    }


#ifdef Q_OS_WIN

    HWND hwnd =
        reinterpret_cast<HWND>(
            overlay->winId()
            );


    if(!hwnd)
    {
        return;
    }


    LONG_PTR exStyle =
        GetWindowLongPtr(
            hwnd,
            GWL_EXSTYLE
            );


    if(!m_clickable)
    {
        // ====================================================
        // CLICK-THROUGH
        // ====================================================

        exStyle |= WS_EX_TRANSPARENT;


        SetWindowLongPtr(
            hwnd,
            GWL_EXSTYLE,
            exStyle
            );
    }
    else
    {
        // ====================================================
        // CLICKABILE
        // ====================================================

        exStyle &= ~WS_EX_TRANSPARENT;


        SetWindowLongPtr(
            hwnd,
            GWL_EXSTYLE,
            exStyle
            );
    }


    // ========================================================
    // FORZA WINDOWS A RILEGGERE LO STILE
    // ========================================================

    SetWindowPos(
        hwnd,
        nullptr,
        0,
        0,
        0,
        0,
        SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOZORDER |
            SWP_NOACTIVATE |
            SWP_FRAMECHANGED
        );


#else

    overlay->setAttribute(
        Qt::WA_TransparentForMouseEvents,
        !m_clickable
        );

#endif
}




// ============================================================
// IS CLICKABLE
// ============================================================

bool OverlayRoot::isClickable() const
{
    return m_clickable;
}


// ============================================================
// SET CLICKABLE
// ============================================================

void OverlayRoot::setClickable(
    bool enabled
    )
{
    if(m_clickable == enabled)
    {
        return;
    }


    m_clickable = enabled;


    for(QWidget *overlay : overlays)
    {
        if(!overlay)
        {
            continue;
        }


        applyClickableState(
            overlay
            );
    }
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


    raise();


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
    // ========================================================
    // HIDE
    // ========================================================

    if(m_overlaysVisible)
    {
        visibilityBeforeHide.clear();


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


        hide();


        m_overlaysVisible = false;

        return;
    }


    // ========================================================
    // SHOW
    // ========================================================

    show();

    raise();


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


    m_overlaysVisible = true;
}

void OverlayRoot::setTransparency(
    int value
    )
{
    m_transparency =
        qBound(
            0,
            value,
            255
            );


    if(m_skillOverlay)
    {
        m_skillOverlay->setTransparency(
            m_transparency
            );
    }


    if(m_buffVisionOverlay)
    {
        m_buffVisionOverlay->setTransparency(
            m_transparency
            );
    }


    if(m_specialCooldownOverlay)
    {
        m_specialCooldownOverlay->setTransparency(
            m_transparency
            );
    }
}

bool OverlayRoot::areOverlaysVisible() const
{
    return m_overlaysVisible;
}


void OverlayRoot::setSkillOverlay(
    SkillOverlay *overlay
    )
{
    m_skillOverlay =
        overlay;

    if(m_skillOverlay)
    {
        m_skillOverlay->setTransparency(
            m_transparency
            );
    }
}


void OverlayRoot::setBuffVisionOverlay(
    BuffVisionOverlay *overlay
    )
{
    m_buffVisionOverlay =
        overlay;

    if(m_buffVisionOverlay)
    {
        m_buffVisionOverlay->setTransparency(
            m_transparency
            );
    }
}


void OverlayRoot::setSpecialCooldownOverlay(
    SpecialCooldownOverlay *overlay
    )
{
    m_specialCooldownOverlay =
        overlay;

    if(m_specialCooldownOverlay)
    {
        m_specialCooldownOverlay->setTransparency(
            m_transparency
            );
    }
}


