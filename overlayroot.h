#ifndef OVERLAYROOT_H
#define OVERLAYROOT_H

#include <QWidget>
#include <QList>

class OverlayRoot : public QWidget
{
    Q_OBJECT

public:

    explicit OverlayRoot(
        QWidget *parent = nullptr
        );


    void registerOverlay(QWidget *overlay);


    void raiseAll();


private:

    QList<QWidget*> overlays;

};

#endif