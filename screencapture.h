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
    static QImage captureScreen(QScreen *screen);
    static QImage captureRegion(QScreen *screen, const QRect &area);
    static QImage crop(const QImage &image, const QRect &area);

private:
    static bool ensureInit();
    static bool ensureStaging(const QSize &size);
    static bool reinit();
    static QImage grabRegionInternal(const QRect &rect);

    static ComPtr<ID3D11Device> s_device;
    static ComPtr<ID3D11DeviceContext> s_context;
    static ComPtr<IDXGIOutputDuplication> s_duplication;
    static ComPtr<ID3D11Texture2D> s_staging;

    static QSize s_stagingSize;
    static QRect s_desktopRect;
    static bool s_ready;

    // Cache per-rect: se lo schermo non e' cambiato,
    // riusiamo l'ultima immagine catturata per quella regione.
    static QHash<QRect, QImage> s_cache;
};