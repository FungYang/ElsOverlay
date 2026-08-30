#pragma once

#include <QImage>
#include <QRect>
#include <QScreen>
#include <QHash>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;


class ScreenCapture
{
public:

    // =====================================================
    // INIT
    // =====================================================

    static bool ensureInit();


    // =====================================================
    // REGIONI
    //
    // Ogni consumer registra solo le regioni che gli servono.
    //
    // Esempio:
    //
    // int id = registerRegion(rect);
    //
    // ...
    //
    // unregisterRegion(id);
    // =====================================================

    static int registerRegion(
        const QRect &rect
        );

    static void unregisterRegion(
        int regionId
        );


    static bool isRegionRegistered(
        int regionId
        );


    static QRect regionRect(
        int regionId
        );
    static bool updateRegion(
        int regionId,
        const QRect &rect
        );


    // =====================================================
    // FRAME CONDIVISO
    //
    // beginFrame()
    //      ↓
    // captureRegion(id)
    // captureRegion(id)
    // captureRegion(id)
    //      ↓
    // endFrame()
    //
    // AcquireNextFrame viene chiamato UNA volta.
    // =====================================================

    static bool beginFrame();

    static bool hasFrame();

    static QImage captureRegion(
        int regionId
        );

    static void endFrame();


    // =====================================================
    // API CLASSICA / COMPATIBILITÀ
    // =====================================================

    static QImage captureScreen(
        QScreen *screen
        );

    static QImage crop(
        const QImage &image,
        const QRect &area
        );

    static QImage captureRegionReliable(
        QScreen *screen,
        const QRect &area
        );


private:

    // =====================================================
    // DXGI
    // =====================================================

    static bool reinit();


    // =====================================================
    // STAGING
    // =====================================================

    static bool ensureStaging(
        const QSize &size
        );


    // =====================================================
    // ESTRAZIONE DAL FRAME CORRENTE
    // =====================================================

    static QImage captureRectFromCurrentFrame(
        const QRect &rect
        );


    // =====================================================
    // DXGI RESOURCES
    // =====================================================

    static ComPtr<ID3D11Device>
        s_device;

    static ComPtr<ID3D11DeviceContext>
        s_context;

    static ComPtr<IDXGIOutputDuplication>
        s_duplication;


    // Texture CPU-readable.
    //
    // Viene riutilizzata tra le catture.
    // Se arriva una regione più grande,
    // viene ricreata alla nuova dimensione.

    static ComPtr<ID3D11Texture2D>
        s_staging;


    // =====================================================
    // FRAME CORRENTE
    // =====================================================

    static ComPtr<ID3D11Texture2D>
        s_currentFrame;


    static bool
        s_frameAcquired;


    // =====================================================
    // GEOMETRIA
    // =====================================================

    static QSize
        s_stagingSize;


    static QRect
        s_desktopRect;


    static bool
        s_ready;


    // =====================================================
    // REGIONS
    // =====================================================

    static int
        s_nextRegionId;


    static QHash<int, QRect>
        s_regions;


    // =====================================================
    // CACHE
    //
    // Ultima immagine valida per ogni regione.
    // =====================================================

    static QHash<int, QImage>
        s_regionCache;
};