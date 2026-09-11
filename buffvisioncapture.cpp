#include "buffvisioncapture.h"
#include "buffvisionconfig.h"
#include "screencapture.h"
#include "capturecoordinator.h"

#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
#include <QDir>
#include <QDebug>

BuffVisionCapture::BuffVisionCapture(QObject *parent)
    : QObject(parent)
{
}

BuffVisionCapture::~BuffVisionCapture()
{
    // RIMOSSO: niente più m_frameActive/ScreenCapture::endFrame()
    // da gestire qui — il coordinator possiede il ciclo frame.
    unregisterRegions();
}

bool BuffVisionCapture::loadSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
        return false;

    const QSize resolution = screen->size();
    const QSize cropSize = BuffVisionConfig::cropSizeForScreen(resolution);

    const QString suffix =
        QString("_%1x%2").arg(resolution.width()).arg(resolution.height());

    cropRect1 = QRect(
        settings.value("BuffVision/Crop1X" + suffix, 0).toInt(),
        settings.value("BuffVision/Crop1Y" + suffix, 0).toInt(),
        cropSize.width(), cropSize.height()
        );

    cropRect2 = QRect(
        settings.value("BuffVision/Crop2X" + suffix, 0).toInt(),
        settings.value("BuffVision/Crop2Y" + suffix, 0).toInt(),
        cropSize.width(), cropSize.height()
        );

    registerRegions();

    return m_crop1RegionId >= 0 && m_crop2RegionId >= 0;
}

void BuffVisionCapture::unregisterRegions()
{
    if (m_crop1RegionId >= 0)
    {
        CaptureCoordinator::instance()->unregisterRegion(m_crop1RegionId);
        m_crop1RegionId = -1;
    }

    if (m_crop2RegionId >= 0)
    {
        CaptureCoordinator::instance()->unregisterRegion(m_crop2RegionId);
        m_crop2RegionId = -1;
    }
}

void BuffVisionCapture::registerRegions()
{
    unregisterRegions();

    if (cropRect1.isValid() && !cropRect1.isEmpty())
        m_crop1RegionId = CaptureCoordinator::instance()->registerRegion(cropRect1);

    if (cropRect2.isValid() && !cropRect2.isEmpty())
        m_crop2RegionId = CaptureCoordinator::instance()->registerRegion(cropRect2);
}

void BuffVisionCapture::setCropAreas(QRect crop1, QRect crop2)
{
    cropRect1 = crop1;
    cropRect2 = crop2;

    registerRegions();
}

// saveReference1() e saveReference2(): INVARIATE, identiche a
// quelle che mi hai mandato — usano ScreenCapture::captureRegionReliable()
// (cattura singola via QScreen::grabWindow), non toccano beginFrame/endFrame.
// Le riporto solo per completezza, corpo identico all'originale.

void BuffVisionCapture::saveReference1()
{
    QDir dir(QCoreApplication::applicationDirPath() + "/BuffVision");
    if (!dir.exists()) dir.mkpath(".");

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QImage img1 = ScreenCapture::captureRegionReliable(screen, cropRect1);
    QImage img2 = ScreenCapture::captureRegionReliable(screen, cropRect2);

    if (img1.isNull() || img2.isNull())
        return;

    img1.save(dir.filePath("Crop1_Ref1.png"));
    img2.save(dir.filePath("Crop2_Ref1.png"));
}

void BuffVisionCapture::saveReference2()
{
    QDir dir(QCoreApplication::applicationDirPath() + "/BuffVision");
    if (!dir.exists()) dir.mkpath(".");

    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QImage img1 = ScreenCapture::captureRegionReliable(screen, cropRect1);
    QImage img2 = ScreenCapture::captureRegionReliable(screen, cropRect2);

    if (img1.isNull() || img2.isNull())
        return;

    img1.save(dir.filePath("Crop1_Ref2.png"));
    img2.save(dir.filePath("Crop2_Ref2.png"));
}