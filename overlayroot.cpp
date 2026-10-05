#include "overlayroot.h"

#include <QApplication>
#include <QScreen>

#include <windows.h>

#include "skilloverlay.h"
#include "buffvisionoverlay.h"
#include "specialcooldownoverlay.h"
#include "customsearcheroverlay.h"


#ifdef Q_OS_WIN

OverlayRoot *OverlayRoot::s_instance = nullptr;

#endif


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

    if(QScreen *screen = QApplication::primaryScreen())
    {
        setGeometry(
            screen->geometry()
            );
    }


    show();

    raise();


#ifdef Q_OS_WIN

    // ========================================================
    // WINDOWS FOREGROUND EVENT
    //
    // Nessun timer periodico.
    //
    // Riceviamo un evento quando cambia la finestra
    // in foreground e, in quel momento, riportiamo
    // il Custom Searcher sopra le altre finestre TOPMOST.
    // ========================================================

    if(!s_instance)
    {
        s_instance = this;

        m_foregroundHook =
            SetWinEventHook(
                EVENT_SYSTEM_FOREGROUND,
                EVENT_SYSTEM_FOREGROUND,
                nullptr,
                &OverlayRoot::winEventProc,
                0,
                0,
                WINEVENT_OUTOFCONTEXT |
                    WINEVENT_SKIPOWNPROCESS
                );
    }

#endif
}


// ============================================================
// DESTRUCTOR
// ============================================================

OverlayRoot::~OverlayRoot()
{
#ifdef Q_OS_WIN

    if(m_foregroundHook)
    {
        UnhookWinEvent(
            m_foregroundHook
            );

        m_foregroundHook = nullptr;
    }


    if(s_instance == this)
    {
        s_instance = nullptr;
    }

#endif
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


    // ========================================================
    // OVERLAY NORMALI
    // ========================================================

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


    // ========================================================
    // CUSTOM SEARCHER
    //
    // Sempre per ultimi.
    // ========================================================

    raiseCustomSearcherOverlays();
}


// ============================================================
// RAISE CUSTOM SEARCHER OVERLAYS
// ============================================================

void OverlayRoot::raiseCustomSearcherOverlays()
{
    if(!m_overlaysVisible)
    {
        return;
    }


#ifdef Q_OS_WIN

    for(CustomSearcherOverlay *overlay :
         m_customSearcherOverlays)
    {
        if(!overlay)
        {
            continue;
        }


        if(!overlay->isVisible())
        {
            continue;
        }


        HWND hwnd =
            reinterpret_cast<HWND>(
                overlay->winId()
                );


        if(!hwnd)
        {
            continue;
        }


        // ====================================================
        // WINDOWS TOPMOST
        //
        // Non attiva la finestra.
        // Non prende il focus.
        // Non modifica posizione o dimensione.
        // ====================================================

        SetWindowPos(
            hwnd,
            HWND_TOPMOST,
            0,
            0,
            0,
            0,
            SWP_NOMOVE |
                SWP_NOSIZE |
                SWP_NOACTIVATE
            );
    }

#else

    for(CustomSearcherOverlay *overlay :
         m_customSearcherOverlays)
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

#endif
}


// ============================================================
// WINDOWS FOREGROUND EVENT
// ============================================================

#ifdef Q_OS_WIN

void CALLBACK OverlayRoot::winEventProc(
    HWINEVENTHOOK,
    DWORD event,
    HWND,
    LONG,
    LONG,
    DWORD,
    DWORD
    )
{
    if(event != EVENT_SYSTEM_FOREGROUND)
    {
        return;
    }


    if(!s_instance)
    {
        return;
    }


    s_instance->handleForegroundChanged();
}


void OverlayRoot::handleForegroundChanged()
{
    if(!m_overlaysVisible)
    {
        return;
    }

    // Prima riportiamo sopra gli overlay normali.
    raiseNormalOverlays();

    // Infine riportiamo sopra i Custom Searcher.
    raiseCustomSearcherOverlays();
}

#endif


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


    // ========================================================
    // CUSTOM SEARCHER
    //
    // Dopo il ripristino, riportiamolo sopra.
    // ========================================================

    raiseCustomSearcherOverlays();
}


// ============================================================
// SET TRANSPARENCY
// ============================================================

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


    for(CustomSearcherOverlay *overlay :
         m_customSearcherOverlays)
    {
        if(!overlay)
        {
            continue;
        }


        overlay->setTransparency(
            m_transparency
            );
    }
}


// ============================================================
// ARE OVERLAYS VISIBLE
// ============================================================

bool OverlayRoot::areOverlaysVisible() const
{
    return m_overlaysVisible;
}


// ============================================================
// SET SKILL OVERLAY
// ============================================================

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


// ============================================================
// SET BUFF VISION OVERLAY
// ============================================================

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


// ============================================================
// SET SPECIAL COOLDOWN OVERLAY
// ============================================================

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


// ============================================================
// REGISTER CUSTOM SEARCHER OVERLAY
// ============================================================

void OverlayRoot::registerCustomSearcherOverlay(
    CustomSearcherOverlay *overlay)
{
    if(!overlay)
        return;

    if(!m_customSearcherOverlays.contains(overlay))
    {
        m_customSearcherOverlays.append(overlay);

        connect(
            overlay,
            &QObject::destroyed,
            this,
            [this, overlay]()
            {
                m_customSearcherOverlays.removeAll(overlay);
            });
    }

    overlay->setTransparency(m_transparency);

    registerOverlay(overlay);

    // Lo stato corrente di OverlayRoot deve essere sempre
    // quello applicato al nuovo overlay.
    applyClickableState(overlay);

    if(m_overlaysVisible && overlay->isVisible())
        raiseCustomSearcherOverlays();
}

// ============================================================
// UNREGISTER CUSTOM SEARCHER OVERLAY
// ============================================================

void OverlayRoot::unregisterCustomSearcherOverlay(
    CustomSearcherOverlay *overlay
    )
{
    if(!overlay)
    {
        return;
    }


    m_customSearcherOverlays.removeAll(
        overlay
        );


    overlays.removeAll(
        overlay
        );


    visibilityBeforeHide.remove(
        overlay
        );
}

void OverlayRoot::raiseNormalOverlays()
{
    if(!m_overlaysVisible)
    {
        return;
    }

#ifdef Q_OS_WIN

    for(QWidget *overlay : overlays)
    {
        if(!overlay)
        {
            continue;
        }

        if(!overlay->isVisible())
        {
            continue;
        }

        bool isCustomSearcher = false;

        for(CustomSearcherOverlay *custom :
             m_customSearcherOverlays)
        {
            if(custom == overlay)
            {
                isCustomSearcher = true;
                break;
            }
        }

        if(isCustomSearcher)
        {
            continue;
        }

        HWND hwnd =
            reinterpret_cast<HWND>(
                overlay->winId()
                );

        if(!hwnd)
        {
            continue;
        }

        SetWindowPos(
            hwnd,
            HWND_TOPMOST,
            0,
            0,
            0,
            0,
            SWP_NOMOVE |
                SWP_NOSIZE |
                SWP_NOACTIVATE
            );
    }

#else

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

#endif
}
