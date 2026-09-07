#ifndef OVERLAYROOT_H
#define OVERLAYROOT_H

#include <QList>
#include <QHash>
#include <QWidget>


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


public slots:

    void toggleVisibility();

    void setClickable(
        bool enabled
        );


private:

    QList<QWidget *> overlays;

    QHash<QWidget *, bool> visibilityBeforeHide;

    bool m_overlaysVisible = true;

    bool m_clickable = true;


private:

    void applyClickableState(
        QWidget *overlay
        );
};


#endif
