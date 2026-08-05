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



QPixmap BuffVisionCapture::grabScreen()
{

    QScreen *screen =
        QGuiApplication::primaryScreen();



    if(!screen)
        return QPixmap();



    return screen->grabWindow(
        0
        );

}



QPixmap BuffVisionCapture::captureCrop1()
{

    QPixmap screen =
        grabScreen();

    QPixmap crop = screen.copy(cropRect1);
    if(cropRect1.isNull() || cropRect1.isEmpty())
    {
        qDebug()
        << "ERROR: Crop1 invalid"
        << cropRect1;

        return QPixmap();
        qDebug()
            << "SCREEN:"
            << screen.size()
            << "CROP RECT:"
            << cropRect1
            << "RESULT:"
            << crop.size();
    }


    return screen.copy(
        cropRect1
        );

}



QPixmap BuffVisionCapture::captureCrop2()
{

    QPixmap screen =
        grabScreen();



    return screen.copy(
        cropRect2
        );

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