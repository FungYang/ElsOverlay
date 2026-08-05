#ifndef BUFFVISIONDETECTOR_H
#define BUFFVISIONDETECTOR_H

#include <QObject>
#include <QPixmap>


enum class VisionState
{
    Unknown,
    State1,
    State2
};



class BuffVisionDetector : public QObject
{
    Q_OBJECT

public:

    explicit BuffVisionDetector(
        QObject *parent = nullptr
        );



    VisionState detect(
        const QPixmap &current,
        const QPixmap &ref1,
        const QPixmap &ref2,
        double *score1 = nullptr,
        double *score2 = nullptr
        );



    VisionState detectCrop1(
        const QPixmap &current
        );


    VisionState detectCrop2(
        const QPixmap &current
        );



    void loadReferences();


    bool referencesLoaded() const;



    double getCrop1State1Score() const
    {
        return lastCrop1State1Score;
    }

    double getCrop1State2Score() const
    {
        return lastCrop1State2Score;
    }

    double getCrop2State1Score() const
    {
        return lastCrop2State1Score;
    }

    double getCrop2State2Score() const
    {
        return lastCrop2State2Score;
    }



private:


    double compareImages(
        const QPixmap &a,
        const QPixmap &b
        ) const;



    QPixmap crop1Ref1;
    QPixmap crop1Ref2;


    QPixmap crop2Ref1;
    QPixmap crop2Ref2;



    double lastCrop1State1Score = 0.0;
    double lastCrop1State2Score = 0.0;


    double lastCrop2State1Score = 0.0;
    double lastCrop2State2Score = 0.0;



    bool loaded = false;

};

#endif