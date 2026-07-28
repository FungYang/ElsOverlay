#include "buffoverlay.h"

#include <QtAlgorithms>


BuffOverlay::BuffOverlay(QWidget *parent)
    : QWidget(parent)
{

    setAttribute(
        Qt::WA_TranslucentBackground
        );


    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TransparentForMouseEvents,
        false
        );


    resize(
        500,
        100
        );

}



void BuffOverlay::loadBuffs(
    const QString& className,
    const QList<BuffData>& buffs
    )
{

    qDeleteAll(m_buffs);

    m_buffs.clear();


    int x = 10;


    for(const auto &buff : buffs)
    {

        BuffBox *box =
            new BuffBox(
                buff.key,
                buff.cooldowns,
                className,
                QString(buff.key),
                nullptr
                );


        if(!box->hasSavedPosition())
        {
            box->move(
                200 + x,
                200
                );
        }


        box->show();


        m_buffs.append(
            box
            );


        x += 70;

    }

}


void BuffOverlay::clearBuffs()
{
    qDeleteAll(m_buffs);
    m_buffs.clear();
}



void BuffOverlay::handleKey(int key)
{

    for(BuffBox *buff : m_buffs)
    {

        if(buff->key().unicode() == key)
        {

            buff->startCooldown();

            return;

        }

    }

}

void BuffOverlay::confirmAll()
{
    for(BuffBox *buff : m_buffs)
    {
        buff->confirmPlacement();
    }
}