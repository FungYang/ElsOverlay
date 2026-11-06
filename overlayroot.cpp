#include "overlayroot.h"

#include <QApplication>
#include <QFileInfo>
#include <QScreen>
#include <QTimer>

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
// BRING ALL TO FRONT (pulsante manuale)
// ============================================================

void OverlayRoot::bringAllToFront()
{
#ifdef Q_OS_WIN

    if(!m_overlaysVisible)
    {
        emit bringToFrontFinished(
            false,
            QStringLiteral(
                "Overlay nascosti con il tasto globale"
                )
            );

        return;
    }


    // Se si preme di nuovo il pulsante prima della fine, i passi
    // ancora in coda del click precedente si annullano.
    const quint32 generation =
        ++m_manualRaiseGeneration;


    // 1) Subito.
    raiseAllStrong();


    // 2) Di nuovo dopo poco: chi si riafferma TOPMOST subito dopo
    //    di noi (giochi, launcher, overlay di altre app) viene
    //    battuto una seconda volta.
    QTimer::singleShot(
        200,
        this,
        [this, generation]()
        {
            if(generation != m_manualRaiseGeneration)
            {
                return;
            }

            raiseAllStrong();
        }
        );


    // 3) Verifica: qualcosa sta ancora sopra ai nostri overlay?
    QTimer::singleShot(
        450,
        this,
        [this, generation]()
        {
            if(generation != m_manualRaiseGeneration)
            {
                return;
            }

            int checked = 0;

            const QString coveredBy =
                describeWindowCoveringOverlays(
                    &checked
                    );

            if(checked == 0)
            {
                emit bringToFrontFinished(
                    false,
                    QStringLiteral(
                        "Nessun overlay visibile"
                        )
                    );

                return;
            }

            emit bringToFrontFinished(
                coveredBy.isEmpty(),
                coveredBy
                );
        }
        );

#else

    raiseAll();

    emit bringToFrontFinished(
        true,
        QString()
        );

#endif
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


    // ========================================================
    // RILANCI RAVVICINATI
    //
    // L'evento di foreground arriva nel momento in cui cambia la
    // finestra attiva. Giochi, launcher e overlay di altre app
    // (Discord, OBS, Steam...) spesso si riaffermano TOPMOST DOPO,
    // di qualche decina o centinaio di ms (tipico dopo alt-tab o
    // cambi di fullscreen): in quel caso vincerebbero loro e
    // l'overlay resterebbe sotto fino al cambio successivo.
    //
    // Ripetiamo quindi il raise poco dopo. Sono 3 timer singoli
    // per ogni cambio di foreground, NON un timer periodico: a
    // riposo non c'e' nessun risveglio.
    //
    // Se arriva un altro cambio di foreground prima che i rilanci
    // scattino, quelli vecchi si annullano (generazione diversa) e
    // ci pensa il nuovo: i timer non si accumulano.
    // ========================================================

    const quint32 generation =
        ++m_raiseGeneration;

    for(const int delayMs : { 100, 400, 1500 })
    {
        QTimer::singleShot(
            delayMs,
            this,
            [this, generation]()
            {
                if(generation != m_raiseGeneration)
                {
                    return;
                }

                if(!m_overlaysVisible)
                {
                    return;
                }

                raiseNormalOverlays();

                raiseCustomSearcherOverlays();
            }
            );
    }
}


// ============================================================
// RILANCIO "FORTE" + VERIFICA (usati dal pulsante manuale)
// ============================================================

namespace
{

// Descrizione leggibile di una finestra:
//   eldorado.exe - "Titolo finestra" [ClasseFinestra]
QString describeWindow(
    HWND window
    )
{
    wchar_t title[256] = { 0 };
    GetWindowTextW(window, title, 256);

    wchar_t className[128] = { 0 };
    GetClassNameW(window, className, 128);

    QString executable;

    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);

    if(HANDLE process =
            OpenProcess(
                PROCESS_QUERY_LIMITED_INFORMATION,
                FALSE,
                processId
                ))
    {
        wchar_t path[1024] = { 0 };
        DWORD size = 1024;

        if(QueryFullProcessImageNameW(
               process,
               0,
               path,
               &size
               ))
        {
            // Separatori sempre '/', cosi' fileName() funziona
            // identico su qualunque piattaforma.
            executable =
                QFileInfo(
                    QString::fromWCharArray(
                        path,
                        static_cast<int>(size)
                        ).replace(
                            '\\',
                            '/'
                            )
                    ).fileName();
        }

        CloseHandle(process);
    }

    const QString titleText =
        QString::fromWCharArray(title).trimmed();

    return QString("%1 - \"%2\" [%3]")
        .arg(
            executable.isEmpty()
                ? QStringLiteral("processo sconosciuto")
                : executable,
            titleText.isEmpty()
                ? QStringLiteral("senza titolo")
                : titleText,
            QString::fromWCharArray(className)
            );
}

}


void OverlayRoot::raiseAllStrong()
{
    if(!m_overlaysVisible)
    {
        return;
    }


    auto isCustomSearcher =
        [this](const QWidget *widget)
    {
        for(const CustomSearcherOverlay *custom :
             m_customSearcherOverlays)
        {
            if(custom == widget)
            {
                return true;
            }
        }

        return false;
    };


    // Toglie e rimette il flag topmost: la finestra riparte dalla
    // cima della fascia topmost (sopra anche ad altre finestre
    // topmost), cosa che HWND_TOPMOST ripetuto su una finestra gia'
    // topmost non garantisce.
    auto reassertTopmost =
        [](QWidget *widget)
    {
        if(!widget || !widget->isVisible())
        {
            return;
        }

        HWND hwnd =
            reinterpret_cast<HWND>(
                widget->winId()
                );

        if(!hwnd)
        {
            return;
        }

        const UINT flags =
            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOACTIVATE;

        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, flags);
        SetWindowPos(hwnd, HWND_TOPMOST,   0, 0, 0, 0, flags);
    };


    // Prima gli overlay normali...
    for(QWidget *overlay : overlays)
    {
        if(overlay && !isCustomSearcher(overlay))
        {
            reassertTopmost(overlay);
        }
    }

    // ...per ultimi i Custom Searcher (restano sopra a tutto).
    for(CustomSearcherOverlay *custom :
         m_customSearcherOverlays)
    {
        reassertTopmost(custom);
    }
}


QString OverlayRoot::describeWindowCoveringOverlays(
    int *checked
    ) const
{
    if(checked)
    {
        *checked = 0;
    }

    const DWORD ourProcessId =
        GetCurrentProcessId();


    auto coveringWindow =
        [&](const QWidget *widget) -> QString
    {
        if(!widget || !widget->isVisible())
        {
            return QString();
        }

        HWND mine =
            reinterpret_cast<HWND>(
                widget->winId()
                );

        if(!mine)
        {
            return QString();
        }

        if(checked)
        {
            ++(*checked);
        }

        RECT mineRect;

        if(!GetWindowRect(mine, &mineRect))
        {
            return QString();
        }

        // GW_HWNDPREV = la finestra SOPRA nello z-order.
        for(HWND other = GetWindow(mine, GW_HWNDPREV);
             other;
             other = GetWindow(other, GW_HWNDPREV))
        {
            if(!IsWindowVisible(other))
            {
                continue;
            }

            DWORD otherProcessId = 0;

            GetWindowThreadProcessId(
                other,
                &otherProcessId
                );

            if(otherProcessId == ourProcessId)
            {
                continue;
            }

            RECT otherRect;
            RECT intersection;

            if(!GetWindowRect(other, &otherRect))
            {
                continue;
            }

            if(!IntersectRect(
                   &intersection,
                   &otherRect,
                   &mineRect
                   ))
            {
                continue;
            }

            return describeWindow(other);
        }

        return QString();
    };


    for(const QWidget *overlay : overlays)
    {
        const QString covering =
            coveringWindow(overlay);

        if(!covering.isEmpty())
        {
            return covering;
        }
    }

    return QString();
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
