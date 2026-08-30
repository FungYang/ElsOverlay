#pragma once

#include <QObject>
#include <QRect>
#include <QPixmap>

class BuffVisionCapture : public QObject
{
    Q_OBJECT

public:

    explicit BuffVisionCapture(
        QObject *parent = nullptr
        );

    ~BuffVisionCapture();


    // =====================================================
    // SETTINGS
    // =====================================================

    bool loadSettings();

    bool initDuplication();

    void setCropAreas(
        QRect crop1,
        QRect crop2
        );


    // =====================================================
    // SHARED FRAME
    //
    // beginCapture()
    //      -> cattura UN frame DXGI
    //
    // captureCrop1()
    //      -> estrae crop1 dal frame
    //
    // captureCrop2()
    //      -> estrae crop2 dal frame
    //
    // endCapture()
    //      -> rilascia il frame DXGI
    // =====================================================

    bool beginCapture();

    QPixmap captureCrop1();

    QPixmap captureCrop2();

    void endCapture();


    // =====================================================
    // REFERENCES
    // =====================================================

    void saveReference1();

    void saveReference2();


private:

    void unregisterRegions();

    void registerRegions();


private:

    QRect cropRect1;
    QRect cropRect2;

    int m_crop1RegionId = -1;
    int m_crop2RegionId = -1;

    bool m_frameActive = false;
};