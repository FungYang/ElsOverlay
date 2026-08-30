#include "buffvisioncapture.h"
#include "buffvisionconfig.h"
#include "screencapture.h"

#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
#include <QDir>
#include <QDebug>


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
            settings.value("BuffVision/Crop1X", 0).toInt(),
            settings.value("BuffVision/Crop1Y", 0).toInt(),
            BuffVisionConfig::CROP_WIDTH,
            BuffVisionConfig::CROP_HEIGHT
            );

    cropRect2 =
        QRect(
            settings.value("BuffVision/Crop2X", 0).toInt(),
            settings.value("BuffVision/Crop2Y", 0).toInt(),
            BuffVisionConfig::CROP_WIDTH,
            BuffVisionConfig::CROP_HEIGHT
            );

    return true;
}


bool BuffVisionCapture::initDuplication()
{
    // ScreenCapture gestisce l'inizializzazione di
    // D3D11/Desktop Duplication in modo lazy e condiviso
    // tra tutti i moduli dell'applicazione: qui non serve
    // piu' fare nulla, la funzione resta solo per non dover
    // toccare i chiamanti esistenti.

    return true;
}


QPixmap BuffVisionCapture::captureCrop1()
{
    if(cropRect1.isNull() || cropRect1.isEmpty())
    {
        qDebug() << "ERROR: Crop1 invalid" << cropRect1;
        return QPixmap();
    }

    QScreen *screen =
        QGuiApplication::primaryScreen();

    QImage img =
        ScreenCapture::captureRegion(
            screen,
            cropRect1
            );

    return QPixmap::fromImage(img);
}


QPixmap BuffVisionCapture::captureCrop2()
{
    if(cropRect2.isNull() || cropRect2.isEmpty())
        return QPixmap();

    QScreen *screen =
        QGuiApplication::primaryScreen();

    QImage img =
        ScreenCapture::captureRegion(
            screen,
            cropRect2
            );

    return QPixmap::fromImage(img);
}


void BuffVisionCapture::saveReference1()
{
    QDir dir("BuffVision");

    if(!dir.exists())
        dir.mkpath(".");

    QPixmap crop1 = captureCrop1();
    QPixmap crop2 = captureCrop2();

    if(crop1.isNull() || crop2.isNull())
        return;

    crop1.save("BuffVision/Crop1_Ref1.png");
    crop2.save("BuffVision/Crop2_Ref1.png");
}


void BuffVisionCapture::saveReference2()
{
    QPixmap crop1 = captureCrop1();
    QPixmap crop2 = captureCrop2();

    if(crop1.isNull() || crop2.isNull())
        return;

    crop1.save("BuffVision/Crop1_Ref2.png");
    crop2.save("BuffVision/Crop2_Ref2.png");
}


void BuffVisionCapture::setCropAreas(
    QRect crop1,
    QRect crop2
    )
{
    cropRect1 = crop1;
    cropRect2 = crop2;
}