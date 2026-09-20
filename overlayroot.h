#ifndef OVERLAYROOT_H
#define OVERLAYROOT_H

#include <QList>
#include <QHash>
#include <QWidget>

class SkillOverlay;
class BuffVisionOverlay;
class SpecialCooldownOverlay;


    class OverlayRoot : public QWidget
{
    Q_OBJECT

public:

    explicit OverlayRoot(
        QWidget *parent = nullptr
        );


    void registerOverlay(
        QWidget *overlay
        );


    void raiseAll();


    bool isClickable() const;

    bool areOverlaysVisible() const;

    void setSkillOverlay(
        SkillOverlay *overlay
        );

    void setBuffVisionOverlay(
        BuffVisionOverlay *overlay
        );

    void setSpecialCooldownOverlay(
        SpecialCooldownOverlay *overlay
        );


public slots:

    void toggleVisibility();

    void setClickable(
        bool enabled
        );

    void setTransparency(
        int value
        );


private:

    SkillOverlay *m_skillOverlay = nullptr;

    BuffVisionOverlay *m_buffVisionOverlay = nullptr;

    SpecialCooldownOverlay *m_specialCooldownOverlay = nullptr;

    QList<QWidget *> overlays;

    QHash<QWidget *, bool> visibilityBeforeHide;

    bool m_overlaysVisible = true;

    bool m_clickable = true;

    int m_transparency = 255;


private:

    void applyClickableState(
        QWidget *overlay
        );
};


#endif
