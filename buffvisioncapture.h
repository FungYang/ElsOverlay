#pragma once

#include <QObject>
#include <QRect>

class BuffVisionCapture : public QObject
{
    Q_OBJECT

public:
    explicit BuffVisionCapture(QObject *parent = nullptr);
    ~BuffVisionCapture();

    bool loadSettings();

    void setCropAreas(QRect crop1, QRect crop2);

    // NUOVO: accessor per il manager, che deve iscriversi
    // direttamente al CaptureCoordinator con questi id.
    int crop1RegionId() const { return m_crop1RegionId; }
    int crop2RegionId() const { return m_crop2RegionId; }

    // saveReference1/2 restano INVARIATI: sono catture singole,
    // una tantum, non cicliche — non toccano ScreenCapture::beginFrame,
    // usano captureRegionReliable() come già fanno oggi.
    void saveReference1();
    void saveReference2();

    // RIMOSSI: initDuplication(), beginCapture(), captureCrop1(),
    // captureCrop2(), endCapture(). Il ciclo cattura ora è
    // interamente gestito da CaptureCoordinator::tick(), che
    // chiama ScreenCapture::captureRegion() direttamente sugli
    // id restituiti da crop1RegionId()/crop2RegionId().

private:
    void unregisterRegions();
    void registerRegions();

private:
    QRect cropRect1;
    QRect cropRect2;

    int m_crop1RegionId = -1;
    int m_crop2RegionId = -1;
};