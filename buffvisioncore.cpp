#include "buffvisioncore.h"



BuffVisionCore::BuffVisionCore(QObject *parent)
    : QObject(parent)
{


    // =========================
    // TIMER BUFF 60 SECONDI
    // =========================

    buff60Timer.setSingleShot(true);


    connect(
        &buff60Timer,
        &QTimer::timeout,
        this,
        [this]()
        {
            buff60Active = false;

            emit buff60Expired();
        }
        );



    // =========================
    // TIMER BUFF 15 SECONDI
    // =========================

    buff15Timer.setSingleShot(true);


    connect(
        &buff15Timer,
        &QTimer::timeout,
        this,
        [this]()
        {

            emit buff15Expired();

        }
        );

}



void BuffVisionCore::startTracking()
{


    tracking = true;

    atmaCooldownPaused = false;
    buff15RemainingMs = 0;
    buff60RemainingMs = 0;

    actionCounter = 0;


    buff60Active = false;


    buff60Timer.stop();

    buff15Timer.stop();

}



void BuffVisionCore::reset()
{

    tracking = false;

    atmaCooldownPaused = false;
    buff15RemainingMs = 0;
    buff60RemainingMs = 0;

    actionCounter = 0;


    buff60Active = false;


    buff60Timer.stop();

    buff15Timer.stop();


    emit buff60Expired();

    emit buff15Expired();

}


void BuffVisionCore::registerAction()
{

    if(!tracking)
        return;



    // =========================
    // BUFF 15 SECONDI
    // =========================

    if(!buff15Timer.isActive())
    {

        emit buff15Activated();


        buff15Timer.start(15000);

    }



    // =========================
    // BUFF 60 ATTIVO
    // BLOCCA IL CONTATORE
    // =========================

    if(buff60Active)
    {
        actionCounter = 0;
        return;
    }



    // =========================
    // CONTATORE BUFF 60
    // =========================

    actionCounter++;



    if(actionCounter >= 3)
    {
        actionCounter = 0;


        buff60Active = true;


        emit buff60Activated();



        buff60Timer.start(
            60000
            );

    }

}



bool BuffVisionCore::isBuff60Active() const
{

    return buff60Active;

}

void BuffVisionCore::onActionDetected()
{
    registerAction();
}

void BuffVisionCore::stopTracking()
{
    tracking = false;

    actionCounter = 0;

    buff60Timer.stop();

    buff15Timer.stop();

}

void BuffVisionCore::onVisionEvent()
{

    if(!tracking)
        return;



    // evento equivalente ad una azione rilevata
    registerAction();

}

void BuffVisionCore::onCrop1Event()
{
    // qDebug()
    // << "CROP 1 EVENT";

    // qui metteremo la logica del buff associato al crop 1

    onVisionEvent();
}



void BuffVisionCore::onCrop2Event()
{
    // qDebug()
    // << "CROP 2 EVENT";

    // qui metteremo la logica del buff associato al crop 2

    onVisionEvent();
}

void BuffVisionCore::pauseAtmaCooldowns()
{
    if (atmaCooldownPaused)
        return;

    atmaCooldownPaused = true;

    if (buff15Timer.isActive())
    {
        buff15RemainingMs = buff15Timer.remainingTime();
        buff15Timer.stop();
    }
    else
    {
        buff15RemainingMs = 0;
    }

    if (buff60Timer.isActive())
    {
        buff60RemainingMs = buff60Timer.remainingTime();
        buff60Timer.stop();
    }
    else
    {
        buff60RemainingMs = 0;
    }

}


void BuffVisionCore::resumeAtmaCooldowns()
{
    if (!atmaCooldownPaused)
        return;

    atmaCooldownPaused = false;

    if (buff15RemainingMs > 0)
    {
        buff15Timer.start(buff15RemainingMs);
    }

    if (buff60RemainingMs > 0)
    {
        buff60Timer.start(buff60RemainingMs);
    }

    buff15RemainingMs = 0;
    buff60RemainingMs = 0;
}

int BuffVisionCore::buff15RemainingTime() const
{
    if (atmaCooldownPaused)
        return buff15RemainingMs;

    if (buff15Timer.isActive())
        return buff15Timer.remainingTime();

    return 0;
}


int BuffVisionCore::buff60RemainingTime() const
{
    if (atmaCooldownPaused)
        return buff60RemainingMs;

    if (buff60Timer.isActive())
        return buff60Timer.remainingTime();

    return 0;
}

