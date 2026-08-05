#include "buffvisiondetector.h"

#include <QImage>
#include <QColor>
#include <QDebug>
#include <QFile>



BuffVisionDetector::BuffVisionDetector(
    QObject *parent
    )
    : QObject(parent)
{

}



void BuffVisionDetector::loadReferences()
{

    crop1Ref1.load(
        "BuffVision/Crop1_Ref1.png"
        );


    crop1Ref2.load(
        "BuffVision/Crop1_Ref2.png"
        );


    crop2Ref1.load(
        "BuffVision/Crop2_Ref1.png"
        );


    crop2Ref2.load(
        "BuffVision/Crop2_Ref2.png"
        );



    loaded =
        !crop1Ref1.isNull()
        &&
        !crop1Ref2.isNull()
        &&
        !crop2Ref1.isNull()
        &&
        !crop2Ref2.isNull();



    // qDebug()
    //     << "References loaded:"
    //     << loaded;

}



bool BuffVisionDetector::referencesLoaded() const
{

    return loaded;

}



VisionState BuffVisionDetector::detect(
    const QPixmap &current,
    const QPixmap &ref1,
    const QPixmap &ref2,
    double *score1,
    double *score2
    )
{

    if(current.isNull() ||
        ref1.isNull() ||
        ref2.isNull())
    {

        if(score1)
            *score1 = 0.0;


        if(score2)
            *score2 = 0.0;


        return VisionState::Unknown;

    }



    double s1 =
        compareImages(
            current,
            ref1
            );


    double s2 =
        compareImages(
            current,
            ref2
            );



    if(score1)
        *score1 = s1;


    if(score2)
        *score2 = s2;



    constexpr double confidence = 0.80;



    if(s1 >= confidence &&
        s1 > s2)
    {
        return VisionState::State1;
    }



    if(s2 >= confidence &&
        s2 > s1)
    {
        return VisionState::State2;
    }



    return VisionState::Unknown;

}



VisionState BuffVisionDetector::detectCrop1(
    const QPixmap &current
    )
{

    return detect(
        current,
        crop1Ref1,
        crop1Ref2,
        &lastCrop1State1Score,
        &lastCrop1State2Score
        );

}



VisionState BuffVisionDetector::detectCrop2(
    const QPixmap &current
    )
{

    return detect(
        current,
        crop2Ref1,
        crop2Ref2,
        &lastCrop2State1Score,
        &lastCrop2State2Score
        );

}



double BuffVisionDetector::compareImages(
    const QPixmap &a,
    const QPixmap &b
    ) const
{

    if(a.isNull() ||
        b.isNull())
    {
        return 0.0;
    }



    QImage imgA =
        a.toImage()
            .convertToFormat(
                QImage::Format_RGB32
                );


    QImage imgB =
        b.toImage()
            .convertToFormat(
                QImage::Format_RGB32
                );



    if(imgA.size() != imgB.size())
    {

        qDebug()
        << "SIZE MISMATCH"
        << "Current:"
        << imgA.size()
        << "Reference:"
        << imgB.size();


        return 0.0;

    }



    long long totalDiff = 0;



    int pixels =
        imgA.width() *
        imgA.height();



    for(int y = 0;
         y < imgA.height();
         y++)
    {

        for(int x = 0;
             x < imgA.width();
             x++)
        {

            QColor ca =
                imgA.pixelColor(
                    x,
                    y
                    );


            QColor cb =
                imgB.pixelColor(
                    x,
                    y
                    );



            totalDiff +=
                abs(ca.red() -
                    cb.red());


            totalDiff +=
                abs(ca.green() -
                    cb.green());


            totalDiff +=
                abs(ca.blue() -
                    cb.blue());

        }

    }



    double maxDiff =
        pixels *
        255.0 *
        3.0;



    return 1.0 -
           ((double)totalDiff / maxDiff);

}