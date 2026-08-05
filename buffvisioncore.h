#ifndef BUFFVISIONCORE_H
#define BUFFVISIONCORE_H

#include <QObject>
#include <QTimer>


class BuffVisionCore : public QObject
{
    Q_OBJECT

public:

    explicit BuffVisionCore(QObject *parent = nullptr);


    // avvia ascolto eventi
    void startTracking();

    void stopTracking();


    // reset completo
    void reset();


    // chiamata quando succede un'azione
    void registerAction();


    // stato buff 60 secondi
    bool isBuff60Active() const;

    void onVisionEvent();
    void onCrop1Event();
    void onCrop2Event();



signals:

    // buff corto 15 secondi
    void buff15Activated();

    void buff15Expired();

    void buff60Activated();

    void buff60Expired();



    // aggiornamento countdown
    void cooldownChanged(int value);



private:


    bool tracking = false;


    // contatore azioni per buff 60
    int actionCounter = 0;


    // stato buff lungo
    bool buff60Active = false;



    QTimer buff60Timer;

    QTimer buff15Timer;
public slots:

    void onActionDetected();



};

#endif