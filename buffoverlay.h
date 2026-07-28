#ifndef BUFFOVERLAY_H
#define BUFFOVERLAY_H

#include <QWidget>
#include <QList>

#include "buffbox.h"
#include "classdata.h"


class BuffOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit BuffOverlay(QWidget *parent = nullptr);


    void clearBuffs();


    void loadBuffs(
        const QString& className,
        const QList<BuffData>& buffs
        );

public slots:

    void handleKey(int key);
    void confirmAll();


private:

    QList<BuffBox*> m_buffs;

};

#endif