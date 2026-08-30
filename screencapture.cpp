#include "screencapture.h"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

ComPtr<ID3D11Device> ScreenCapture::s_device;
ComPtr<ID3D11DeviceContext> ScreenCapture::s_context;
ComPtr<IDXGIOutputDuplication> ScreenCapture::s_duplication;
ComPtr<ID3D11Texture2D> ScreenCapture::s_staging;


QSize ScreenCapture::s_stagingSize;
QRect ScreenCapture::s_desktopRect;
bool ScreenCapture::s_ready = false;

QHash<QRect, QImage> ScreenCapture::s_cache;

// =========================================================
// INIT
// =========================================================

bool ScreenCapture::ensureInit()
{
    if(s_ready)
        return true;

    return reinit();
}

bool ScreenCapture::reinit()
{
    s_ready = false;
    s_staging.Reset();
    s_duplication.Reset();
    s_context.Reset();
    s_device.Reset();
    s_stagingSize = QSize();
    s_cache.clear();


    QScreen *primaryScreen =
        QGuiApplication::primaryScreen();

    QRect targetGeometry =
        primaryScreen
            ? primaryScreen->geometry()
            : QRect();

    qDebug() << "ScreenCapture: target geometry (Qt primary screen):"
             << targetGeometry;


    ComPtr<IDXGIFactory1> factory;

    HRESULT hr =
        CreateDXGIFactory1(
            __uuidof(IDXGIFactory1),
            reinterpret_cast<void **>(factory.GetAddressOf())
            );

    if(FAILED(hr))
    {
        qDebug() << "ScreenCapture: CreateDXGIFactory1 failed:" << Qt::hex << hr;
        return false;
    }


    UINT adapterIndex = 0;
    ComPtr<IDXGIAdapter1> adapter;

    while(factory->EnumAdapters1(
               adapterIndex,
               adapter.ReleaseAndGetAddressOf()
               ) != DXGI_ERROR_NOT_FOUND)
    {
        UINT outputIndex = 0;
        ComPtr<IDXGIOutput> output;

        while(adapter->EnumOutputs(
                   outputIndex,
                   output.ReleaseAndGetAddressOf()
                   ) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC outputDesc;

            if(FAILED(output->GetDesc(&outputDesc)))
            {
                ++outputIndex;
                continue;
            }

            QRect outputRect(
                outputDesc.DesktopCoordinates.left,
                outputDesc.DesktopCoordinates.top,
                outputDesc.DesktopCoordinates.right -
                    outputDesc.DesktopCoordinates.left,
                outputDesc.DesktopCoordinates.bottom -
                    outputDesc.DesktopCoordinates.top
                );


            // =========================
            // ACCETTIAMO SOLO L'OUTPUT
            // CHE CORRISPONDE ALLO
            // SCHERMO PRIMARIO
            // =========================

            if(outputRect != targetGeometry)
            {
                ++outputIndex;
                continue;
            }


            qDebug() << "ScreenCapture: output candidato trovato:"
                     << QString::fromWCharArray(outputDesc.DeviceName)
                     << "adapter" << adapterIndex
                     << "output" << outputIndex;


            ComPtr<ID3D11Device> testDevice;
            ComPtr<ID3D11DeviceContext> testContext;
            D3D_FEATURE_LEVEL testLevel;

            HRESULT devHr =
                D3D11CreateDevice(
                    adapter.Get(),
                    D3D_DRIVER_TYPE_UNKNOWN,
                    nullptr,
                    0,
                    nullptr,
                    0,
                    D3D11_SDK_VERSION,
                    testDevice.GetAddressOf(),
                    &testLevel,
                    testContext.GetAddressOf()
                    );

            if(FAILED(devHr))
            {
                qDebug() << "ScreenCapture: D3D11CreateDevice failed:"
                         << Qt::hex << devHr;

                return false;
            }


            ComPtr<IDXGIOutput1> output1;
            HRESULT asHr = output.As(&output1);

            if(FAILED(asHr))
            {
                qDebug() << "ScreenCapture: QueryInterface IDXGIOutput1 failed:"
                         << Qt::hex << asHr;

                return false;
            }


            HRESULT dupHr =
                output1->DuplicateOutput(
                    testDevice.Get(),
                    s_duplication.GetAddressOf()
                    );

            if(FAILED(dupHr))
            {
                qDebug() << "ScreenCapture: DuplicateOutput failed sul monitor"
                         << "primario:" << Qt::hex << dupHr
                         << "- probabile conflitto con un altro processo"
                         << "(ShadowPlay, Xbox Game Bar, Discord, OBS, "
                         << "un'altra istanza dell'app...)";

                return false;
            }


            s_device = testDevice;
            s_context = testContext;
            s_desktopRect = outputRect;

            s_ready = true;

            qDebug() << "ScreenCapture: duplication attiva su"
                     << QString::fromWCharArray(outputDesc.DeviceName);

            return true;
        }

        ++adapterIndex;
    }


    qDebug() << "ScreenCapture: nessun output corrisponde"
             << "allo schermo primario:" << targetGeometry;

    return false;
}

bool ScreenCapture::ensureStaging(const QSize &size)
{
    if(s_staging && s_stagingSize == size)
        return true;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = size.width();
    desc.Height = size.height();
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    s_staging.Reset();

    HRESULT hr =
        s_device->CreateTexture2D(
            &desc,
            nullptr,
            s_staging.GetAddressOf()
            );

    if(FAILED(hr))
    {
        qDebug() << "ScreenCapture: CreateTexture2D (staging) failed:" << Qt::hex << hr;
        s_stagingSize = QSize();
        return false;
    }

    s_stagingSize = size;
    return true;
}

// =========================================================
// CATTURA
// =========================================================

QImage ScreenCapture::grabRegionInternal(const QRect &rect)
{
    if(rect.isNull() || rect.isEmpty())
        return QImage();

    if(!ensureInit())
        return s_cache.value(rect);

    ComPtr<IDXGIResource> desktopResource;
    DXGI_OUTDUPL_FRAME_INFO frameInfo = {};

    HRESULT hr =
        s_duplication->AcquireNextFrame(
            0,
            &frameInfo,
            desktopResource.GetAddressOf()
            );

    if(hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        // Nulla e' cambiato: riusiamo la cache per questo rect,
        // se esiste gia'.
        return s_cache.value(rect);
    }

    if(hr == DXGI_ERROR_ACCESS_LOST)
    {
        reinit();
        return s_cache.value(rect);
    }

    if(FAILED(hr))
    {
        qDebug() << "ScreenCapture: AcquireNextFrame failed:" << Qt::hex << hr;
        return s_cache.value(rect);
    }

    ComPtr<ID3D11Texture2D> desktopTexture;
    hr = desktopResource.As(&desktopTexture);

    if(FAILED(hr))
    {
        s_duplication->ReleaseFrame();
        return s_cache.value(rect);
    }

    if(!ensureStaging(rect.size()))
    {
        s_duplication->ReleaseFrame();
        return s_cache.value(rect);
    }

    D3D11_BOX srcBox;
    srcBox.left   = static_cast<UINT>(rect.left());
    srcBox.top    = static_cast<UINT>(rect.top());
    srcBox.front  = 0;
    srcBox.right  = static_cast<UINT>(rect.left() + rect.width());
    srcBox.bottom = static_cast<UINT>(rect.top() + rect.height());
    srcBox.back   = 1;

    s_context->CopySubresourceRegion(
        s_staging.Get(),
        0,
        0, 0, 0,
        desktopTexture.Get(),
        0,
        &srcBox
        );

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = s_context->Map(
        s_staging.Get(),
        0,
        D3D11_MAP_READ,
        0,
        &mapped
        );

    if(FAILED(hr))
    {
        s_duplication->ReleaseFrame();
        return s_cache.value(rect);
    }

    QImage image(
        rect.width(),
        rect.height(),
        QImage::Format_ARGB32
        );

    const uchar *src = static_cast<const uchar *>(mapped.pData);

    for(int y = 0; y < rect.height(); y++)
    {
        memcpy(
            image.scanLine(y),
            src + y * mapped.RowPitch,
            static_cast<size_t>(rect.width()) * 4
            );
    }

    s_context->Unmap(s_staging.Get(), 0);
    s_duplication->ReleaseFrame();

    s_cache.insert(rect, image);

    return image;
}

// =========================================================
// API PUBBLICA
// =========================================================

QImage ScreenCapture::captureScreen(QScreen *screen)
{
    if(!screen)
        return QImage();

    if(!ensureInit())
        return QImage();

    return grabRegionInternal(s_desktopRect);
}

QImage ScreenCapture::captureRegion(QScreen *screen, const QRect &area)
{
    Q_UNUSED(screen);

    return grabRegionInternal(area);
}

QImage ScreenCapture::crop(const QImage &image, const QRect &area)
{
    if(image.isNull())
        return QImage();

    QRect validArea =
        area.intersected(
            image.rect()
            );

    return image.copy(validArea);
}