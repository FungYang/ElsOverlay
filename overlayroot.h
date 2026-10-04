#pragma once

#include <QWidget>
#include <QList>
#include <QHash>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class SkillOverlay;
class BuffVisionOverlay;
class SpecialCooldownOverlay;
class CustomSearcherOverlay;


class OverlayRoot : public QWidget
{
    Q_OBJECT

public:

    explicit OverlayRoot(
        QWidget *parent = nullptr
        );

    ~OverlayRoot() override;


    // =========================================================
    // GENERIC OVERLAYS
    // =========================================================

    void registerOverlay(
        QWidget *overlay
        );


    // =========================================================
    // CLICKABILITY
    // =========================================================

    bool isClickable() const;

    void setClickable(
        bool enabled
        );


    // =========================================================
    // STACKING
    // =========================================================

    void raiseAll();


    // =========================================================
    // VISIBILITY
    // =========================================================

    void toggleVisibility();

    bool areOverlaysVisible() const;


    // =========================================================
    // TRANSPARENCY
    // =========================================================

    void setTransparency(
        int value
        );


    // =========================================================
    // SPECIFIC OVERLAYS
    // =========================================================

    void setSkillOverlay(
        SkillOverlay *overlay
        );

    void setBuffVisionOverlay(
        BuffVisionOverlay *overlay
        );

    void setSpecialCooldownOverlay(
        SpecialCooldownOverlay *overlay
        );


    // =========================================================
    // CUSTOM SEARCHER
    // =========================================================

    void registerCustomSearcherOverlay(
        CustomSearcherOverlay *overlay
        );

    void unregisterCustomSearcherOverlay(
        CustomSearcherOverlay *overlay
        );


private:

    // =========================================================
    // CLICKABILITY
    // =========================================================

    void applyClickableState(
        QWidget *overlay
        );


    // =========================================================
    // CUSTOM SEARCHER STACKING
    // =========================================================

    void raiseCustomSearcherOverlays();

    void raiseNormalOverlays();


#ifdef Q_OS_WIN

    // =========================================================
    // WINDOWS FOREGROUND EVENT
    // =========================================================

    static void CALLBACK winEventProc(
        HWINEVENTHOOK hook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD eventThread,
        DWORD eventTime
        );

    void handleForegroundChanged();

    static OverlayRoot *s_instance;

    HWINEVENTHOOK m_foregroundHook = nullptr;

#endif


private:

    // =========================================================
    // GENERIC OVERLAYS
    // =========================================================

    QList<QWidget *> overlays;

    QHash<QWidget *, bool> visibilityBeforeHide;


    // =========================================================
    // CUSTOM SEARCHER OVERLAYS
    // =========================================================

    QList<CustomSearcherOverlay *> m_customSearcherOverlays;


    // =========================================================
    // SPECIFIC OVERLAYS
    // =========================================================

    SkillOverlay *m_skillOverlay = nullptr;

    BuffVisionOverlay *m_buffVisionOverlay = nullptr;

    SpecialCooldownOverlay *m_specialCooldownOverlay = nullptr;


    // =========================================================
    // STATE
    // =========================================================

    bool m_overlaysVisible = true;

    bool m_clickable = false;

    int m_transparency = 255;
};