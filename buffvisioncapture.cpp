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


// =========================================================
// DESTRUCTOR
// =========================================================

BuffVisionCapture::~BuffVisionCapture()
{
    if(m_frameActive)
    {
        ScreenCapture::endFrame();
        m_frameActive = false;
    }

    unregisterRegions();
}


// =========================================================
// SETTINGS
// =========================================================

bool BuffVisionCapture::loadSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
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


    // Le regioni vengono aggiornate solamente qui.
    registerRegions();


    qDebug()
        << "BuffVision settings:"
        << "crop1 =" << cropRect1
        << "regionId =" << m_crop1RegionId
        << "| crop2 =" << cropRect2
        << "regionId =" << m_crop2RegionId;


    return
        m_crop1RegionId >= 0 &&
        m_crop2RegionId >= 0;
}


// =========================================================
// REGION REGISTRATION
// =========================================================

void BuffVisionCapture::unregisterRegions()
{
    if(m_crop1RegionId >= 0)
    {
        ScreenCapture::unregisterRegion(
            m_crop1RegionId
            );

        m_crop1RegionId = -1;
    }


    if(m_crop2RegionId >= 0)
    {
        ScreenCapture::unregisterRegion(
            m_crop2RegionId
            );

        m_crop2RegionId = -1;
    }
}


void BuffVisionCapture::registerRegions()
{
    // Prima rimuoviamo eventuali regioni precedenti.
    unregisterRegions();


    if(
        cropRect1.isValid() &&
        !cropRect1.isEmpty()
        )
    {
        m_crop1RegionId =
            ScreenCapture::registerRegion(
                cropRect1
                );
    }


    if(
        cropRect2.isValid() &&
        !cropRect2.isEmpty()
        )
    {
        m_crop2RegionId =
            ScreenCapture::registerRegion(
                cropRect2
                );
    }
}


// =========================================================
// DXGI INIT
// =========================================================

bool BuffVisionCapture::initDuplication()
{
    // ScreenCapture inizializza la Desktop Duplication
    // lazy, al primo beginFrame().
    //
    // Non inizializziamo DXGI nel costruttore.

    return true;
}


// =========================================================
// BEGIN CAPTURE
// =========================================================

bool BuffVisionCapture::beginCapture()
{
    if(m_frameActive)
    {
        qDebug()
        << "BuffVisionCapture:"
        << "beginCapture chiamato mentre un frame"
        << "e' gia' attivo";

        return true;
    }


    if(
        m_crop1RegionId < 0 ||
        m_crop2RegionId < 0
        )
    {
        qDebug()
        << "BuffVisionCapture:"
        << "regioni non valide";

        return false;
    }


    if(!ScreenCapture::beginFrame())
    {
        return false;
    }


    m_frameActive = true;


    return true;
}


// =========================================================
// CROP 1
// =========================================================

QPixmap BuffVisionCapture::captureCrop1()
{
    if(!m_frameActive)
    {
        qDebug()
        << "BuffVisionCapture:"
        << "captureCrop1 chiamata senza beginCapture()";

        return QPixmap();
    }


    if(m_crop1RegionId < 0)
        return QPixmap();


    QImage img =
        ScreenCapture::captureRegion(
            m_crop1RegionId
            );


    qDebug()
        << "CROP1 CAPTURE:"
        << "null =" << img.isNull()
        << "size =" << img.size()
        << "rect =" << cropRect1;


    return QPixmap::fromImage(
        img
        );
}


// =========================================================
// CROP 2
// =========================================================

QPixmap BuffVisionCapture::captureCrop2()
{
    if(!m_frameActive)
    {
        qDebug()
        << "BuffVisionCapture:"
        << "captureCrop2 chiamata senza beginCapture()";

        return QPixmap();
    }


    if(m_crop2RegionId < 0)
        return QPixmap();


    QImage img =
        ScreenCapture::captureRegion(
            m_crop2RegionId
            );


    qDebug()
        << "CROP2 CAPTURE:"
        << "null =" << img.isNull()
        << "size =" << img.size()
        << "rect =" << cropRect2;


    return QPixmap::fromImage(
        img
        );
}


// =========================================================
// END CAPTURE
// =========================================================

void BuffVisionCapture::endCapture()
{
    if(!m_frameActive)
        return;


    ScreenCapture::endFrame();


    m_frameActive = false;
}


// =========================================================
// SET CROP AREAS
// =========================================================

void BuffVisionCapture::setCropAreas(
    QRect crop1,
    QRect crop2
    )
{
    // Se per qualche motivo arriva una nuova configurazione
    // mentre un frame e' attivo, chiudiamo prima il frame.
    if(m_frameActive)
    {
        endCapture();
    }


    cropRect1 = crop1;
    cropRect2 = crop2;


    registerRegions();


    qDebug()
        << "BuffVision crop areas updated:"
        << "crop1 =" << cropRect1
        << "id =" << m_crop1RegionId
        << "| crop2 =" << cropRect2
        << "id =" << m_crop2RegionId;
}


// =========================================================
// SAVE REFERENCE 1
// =========================================================

void BuffVisionCapture::saveReference1()
{
    QDir dir(
        QCoreApplication::applicationDirPath() +
        "/BuffVision"
        );


    if(!dir.exists())
    {
        dir.mkpath(".");
    }


    QScreen *screen =
        QGuiApplication::primaryScreen();


    if(!screen)
        return;


    QImage img1 =
        ScreenCapture::captureRegionReliable(
            screen,
            cropRect1
            );


    QImage img2 =
        ScreenCapture::captureRegionReliable(
            screen,
            cropRect2
            );


    if(
        img1.isNull() ||
        img2.isNull()
        )
    {
        qDebug()
        << "BuffVision:"
        << "saveReference1: cattura fallita";

        return;
    }


    img1.save(
        dir.filePath(
            "Crop1_Ref1.png"
            )
        );


    img2.save(
        dir.filePath(
            "Crop2_Ref1.png"
            )
        );


    qDebug()
        << "BuffVision:"
        << "Reference 1 salvata";
}


// =========================================================
// SAVE REFERENCE 2
// =========================================================

void BuffVisionCapture::saveReference2()
{
    QDir dir(
        QCoreApplication::applicationDirPath() +
        "/BuffVision"
        );


    if(!dir.exists())
    {
        dir.mkpath(".");
    }


    QScreen *screen =
        QGuiApplication::primaryScreen();


    if(!screen)
        return;


    QImage img1 =
        ScreenCapture::captureRegionReliable(
            screen,
            cropRect1
            );


    QImage img2 =
        ScreenCapture::captureRegionReliable(
            screen,
            cropRect2
            );


    if(
        img1.isNull() ||
        img2.isNull()
        )
    {
        qDebug()
        << "BuffVision:"
        << "saveReference2: cattura fallita";

        return;
    }


    img1.save(
        dir.filePath(
            "Crop1_Ref2.png"
            )
        );


    img2.save(
        dir.filePath(
            "Crop2_Ref2.png"
            )
        );


    qDebug()
        << "BuffVision:"
        << "Reference 2 salvata";
}