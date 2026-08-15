#ifndef BUFFOVERLAY_H
#define BUFFOVERLAY_H

#include <QWidget>
#include <QList>

#include "buffbox.h"
#include "classconfigurationmanager.h"


class BuffOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit BuffOverlay(
        QWidget *parent = nullptr
        );


    void clearBuffs();


    void loadConfiguration(
        const ClassConfiguration &configuration
        );



public slots:

    void handleKey(
        int key
        );
    void resetAll();
    void removePendingBoxes();


private:

    QList<BuffBox*> m_buffs;

};

#endif