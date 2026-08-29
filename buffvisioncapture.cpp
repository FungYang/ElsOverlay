#include "buffvisioncapture.h"

#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
#include <QDir>
#include "buffvisionconfig.h"



BuffVisionCapture::BuffVisionCapture(
    QObject *parent
    )
    : QObject(parent)
{

}



bool BuffVisionCapture::loadSettings()
{

    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );



    cropRect1 =
        QRect(
            settings.value(
                        "BuffVision/Crop1X",
                        0
                        ).toInt(),

            settings.value(
                        "BuffVision/Crop1Y",
                        0
                        ).toInt(),

            BuffVisionConfig::CROP_WIDTH,
            BuffVisionConfig::CROP_HEIGHT
            );



    cropRect2 =
        QRect(
            settings.value(
                        "BuffVision/Crop2X",
                        0
                        ).toInt(),

            settings.value(
                        "BuffVision/Crop2Y",
                        0
                        ).toInt(),

        BuffVisionConfig::CROP_WIDTH,
        BuffVisionConfig::CROP_HEIGHT
            );



    return true;

}



QPixmap BuffVisionCapture::grabScreen(const QRect &rect)
{
    QScreen *screen =
        QGuiApplication::primaryScreen();
    if(!screen)
        return QPixmap();
    return screen->grabWindow(
        0,
        rect.x(),
        rect.y(),
        rect.width(),
        rect.height()
        );
}

QPixmap BuffVisionCapture::captureCrop1()
{
    if(cropRect1.isNull() || cropRect1.isEmpty())
    {
        qDebug()
        << "ERROR: Crop1 invalid"
        << cropRect1;
        return QPixmap();
    }
    return grabScreen(cropRect1);
}

QPixmap BuffVisionCapture::captureCrop2()
{
    return grabScreen(cropRect2);
}

void BuffVisionCapture::saveReference1()
{
    QDir dir("BuffVision");

    if(!dir.exists())
    {
        dir.mkpath(".");
    }
    QPixmap crop1 =
        captureCrop1();


    QPixmap crop2 =
        captureCrop2();



    if(crop1.isNull() ||
        crop2.isNull())
    {
        return;
    }



    crop1.save(
        "BuffVision/Crop1_Ref1.png"
        );


    crop2.save(
        "BuffVision/Crop2_Ref1.png"
        );

}

void BuffVisionCapture::saveReference2()
{

    QPixmap crop1 =
        captureCrop1();


    QPixmap crop2 =
        captureCrop2();



    if(crop1.isNull() ||
        crop2.isNull())
    {
        return;
    }



    crop1.save(
        "BuffVision/Crop1_Ref2.png"
        );

    crop2.save(
        "BuffVision/Crop2_Ref2.png"
        );

}

void BuffVisionCapture::setCropAreas(
    QRect crop1,
    QRect crop2
    )
{
    cropRect1 = crop1;
    cropRect2 = crop2;
}