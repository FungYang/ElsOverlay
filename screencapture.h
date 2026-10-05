#pragma once

#include <QImage>
#include <QRect>
#include <QScreen>
#include <QHash>
#include <QVector>
#include <QSize>

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
    // beginFrame(regionIds)
    //      ↓
    // captureRegion(id)
    // captureRegion(id)
    // captureRegion(id)
    //      ↓
    // endFrame()
    //
    // AcquireNextFrame viene chiamato UNA volta.
    //
    // Tutte le ROI richieste vengono copiate nell'atlas
    // GPU e poi lette con un solo Map().
    // =====================================================

    static bool beginFrame();

    static bool beginFrame(
        const QVector<int> &regionIds
        );

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
    // ATLAS
    // =====================================================

    static bool rebuildAtlas();

    static bool ensureAtlas(
        const QSize &size
        );


    // =====================================================
    // FRAME / GPU COPY
    // =====================================================

    static bool copyRegionsToAtlas(
        const QVector<int> &regionIds
        );


    // =====================================================
    // CPU MAPPING
    // =====================================================

    static bool mapAtlas();

    static void unmapAtlas();


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


    // Texture CPU-readable contenente l'atlas
    // delle ROI richieste nel frame corrente.

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
    // ATLAS
    // =====================================================

    static QSize
        s_stagingSize;


    static QHash<int, QRect>
        s_atlasRects;


    static bool
        s_atlasDirty;


    // =====================================================
    // MAPPING CORRENTE
    // =====================================================

    static bool
        s_atlasMapped;


    static D3D11_MAPPED_SUBRESOURCE
        s_mappedAtlas;


    // =====================================================
    // GEOMETRIA
    // =====================================================

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