#include "buffvisiondetector.h"

#include <QDebug>


BuffVisionDetector::BuffVisionDetector(
    QObject *parent
    )
    : QObject(parent)
{
}


// ============================================================
// LOAD MODEL
// ============================================================

bool BuffVisionDetector::loadModel(
    const QString &modelPath
    )
{
    loaded =
        digitDetector.loadModel(
            modelPath
            );


    qDebug()
        << "[BuffVisionDetector]"
        << "Model loaded ="
        << loaded;


    return loaded;
}


// ============================================================
// CROP 1
// ============================================================

int BuffVisionDetector::detectCrop1(
    const QPixmap &current
    )
{
    if(!loaded)
    {
        return 1000;
    }


    if(current.isNull())
    {
        return 1000;
    }


    return digitDetector.detect(
        current.toImage()
        );
}


// ============================================================
// CROP 2
// ============================================================

int BuffVisionDetector::detectCrop2(
    const QPixmap &current
    )
{
    if(!loaded)
    {
        return 1000;
    }


    if(current.isNull())
    {
        return 1000;
    }


    return digitDetector.detect(
        current.toImage()
        );
}


// ============================================================
// IS LOADED
// ============================================================

bool BuffVisionDetector::isLoaded() const
{
    return loaded;
}
