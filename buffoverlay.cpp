
#include "buffoverlay.h"

#include "overlayroot.h"

#include <QtAlgorithms>


BuffOverlay::BuffOverlay(
    OverlayRoot *root,
    QWidget *parent
    )
    : QWidget(parent),
    m_root(root)
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


// ============================================================
// LOAD CONFIGURATION
// ============================================================

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


        // ====================================================
        // DIMENSIONE SALVATA
        // ====================================================

        box->resize(
            buff.size
            );


        // ====================================================
        // POSIZIONE SALVATA
        // ====================================================

        box->move(
            buff.position
            );


        // ====================================================
        // MODALITÀ NORMALE
        // ====================================================

        box->setConfigurationMode(
            false
            );


        // ====================================================
        // REGISTRA NELL'OVERLAY ROOT
        // ====================================================

        /*
         * BuffBox utilizza Qt::Tool e quindi è una
         * finestra indipendente.
         *
         * Deve essere registrato direttamente nell'OverlayRoot
         * affinché il DELETE globale possa nasconderlo.
         */

        if(m_root)
        {
            m_root->registerOverlay(
                box
                );
        }


        box->show();


        m_buffs.append(
            box
            );
    }
}


// ============================================================
// CLEAR BUFFS
// ============================================================

void BuffOverlay::clearBuffs()
{
    qDeleteAll(
        m_buffs
        );


    m_buffs.clear();
}


// ============================================================
// HANDLE KEY
// ============================================================

void BuffOverlay::handleKey(
    int key
    )
{
    for(BuffBox *buff :
         m_buffs)
    {
        if(buff->key() == key)
        {
            buff->startCooldown();

            return;
        }
    }
}


// ============================================================
// RESET ALL
// ============================================================

void BuffOverlay::resetAll()
{
    for(BuffBox *buff :
         m_buffs)
    {
        buff->reset();
    }
}


// ============================================================
// REMOVE PENDING BOXES
// ============================================================

void BuffOverlay::removePendingBoxes()
{
    for(int i = m_buffs.size() - 1;
         i >= 0;
         --i)
    {
        BuffBox *buff =
            m_buffs.at(i);


        if(!buff->isInConfigurationMode())
            continue;


        m_buffs.removeAt(
            i
            );


        buff->deleteLater();
    }
}
