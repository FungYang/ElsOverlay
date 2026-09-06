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


public slots:

    void toggleVisibility();


private:

    QList<QWidget *> overlays;

    QHash<QWidget *, bool> visibilityBeforeHide;

    bool m_overlaysVisible = true;
};


#endif
