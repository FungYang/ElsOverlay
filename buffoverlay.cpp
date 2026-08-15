#include "buffoverlay.h"

#include <QtAlgorithms>


BuffOverlay::BuffOverlay(
    QWidget *parent
    )
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



void BuffOverlay::loadConfiguration(
    const ClassConfiguration &configuration
    )
{
    clearBuffs();


    for(const BuffConfiguration &buff :
         configuration.buffs)
    {

        BuffBox *box =
            new BuffBox(
                buff.key,
                buff.cooldown,
                this
                );


        box->move(
            buff.position
            );
        box->setConfigurationMode(false);

        box->show();


        m_buffs.append(
            box
            );

    }
}



void BuffOverlay::clearBuffs()
{
    qDeleteAll(
        m_buffs
        );


    m_buffs.clear();
}



void BuffOverlay::handleKey(
    int key
    )
{
    for(BuffBox *buff :
         m_buffs)
    {

        if(buff->key().unicode() == key)
        {

            buff->startCooldown();

            return;

        }

    }
}

void BuffOverlay::resetAll()
{
    for(BuffBox *buff : m_buffs)
    {
        buff->reset();
    }
}

void BuffOverlay::removePendingBoxes()
{
    for(int i = m_buffs.size() - 1; i >= 0; --i)
    {
        BuffBox *buff = m_buffs.at(i);

        if(!buff->isInConfigurationMode())
            continue;

        m_buffs.removeAt(i);

        buff->deleteLater();
    }
}