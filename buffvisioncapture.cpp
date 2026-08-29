#include "buffvisioncapture.h"
#include "buffvisionconfig.h"

#include <QSettings>
#include <QDir>
#include <QDebug>

BuffVisionCapture::BuffVisionCapture(QObject *parent)
    : QObject(parent)
{
}

BuffVisionCapture::~BuffVisionCapture()
{
    releaseDuplication();
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

// =========================================================
// SETUP D3D11 + DESKTOP DUPLICATION
// =========================================================

bool BuffVisionCapture::initDuplication()
{
    releaseDuplication();

    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr =
        D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            d3dDevice.GetAddressOf(),
            &featureLevel,
            d3dContext.GetAddressOf()
            );

    if(FAILED(hr))
    {
        qDebug() << "D3D11CreateDevice failed:" << Qt::hex << hr;
        return false;
    }

    ComPtr<IDXGIDevice> dxgiDevice;
    hr = d3dDevice.As(&dxgiDevice);
    if(FAILED(hr))
        return false;

    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
    if(FAILED(hr))
        return false;

    // Monitor primario = output 0.
    // Per setup multi-monitor andrebbe enumerato e trovato
    // l'output che corrisponde al monitor primario di Qt.
    ComPtr<IDXGIOutput> output;
    hr = adapter->EnumOutputs(0, output.GetAddressOf());
    if(FAILED(hr))
    {
        qDebug() << "EnumOutputs failed:" << Qt::hex << hr;
        return false;
    }

    ComPtr<IDXGIOutput1> output1;
    hr = output.As(&output1);
    if(FAILED(hr))
        return false;

    hr = output1->DuplicateOutput(
        d3dDevice.Get(),
        duplication.GetAddressOf()
        );

    if(FAILED(hr))
    {
        qDebug() << "DuplicateOutput failed:" << Qt::hex << hr;
        return false;
    }

    duplicationReady = true;
    return true;
}

bool BuffVisionCapture::reacquireDuplication()
{
    // Chiamato quando DXGI_ERROR_ACCESS_LOST:
    // succede ad es. dopo cambio risoluzione, UAC prompt,
    // switch utente, o passaggio a/da fullscreen esclusivo.
    duplicationReady = false;
    return initDuplication();
}

void BuffVisionCapture::releaseDuplication()
{
    stagingTexture.Reset();
    duplication.Reset();
    d3dContext.Reset();
    d3dDevice.Reset();
    duplicationReady = false;
    stagingSize = QSize();
}

bool BuffVisionCapture::ensureStagingTexture(const QSize &size)
{
    if(stagingTexture && stagingSize == size)
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
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    stagingTexture.Reset();

    HRESULT hr =
        d3dDevice->CreateTexture2D(
            &desc,
            nullptr,
            stagingTexture.GetAddressOf()
            );

    if(FAILED(hr))
    {
        qDebug() << "CreateTexture2D (staging) failed:" << Qt::hex << hr;
        stagingSize = QSize();
        return false;
    }

    stagingSize = size;
    return true;
}

// =========================================================
// CATTURA REGIONE
// =========================================================

QImage BuffVisionCapture::grabRegion(const QRect &rect, QImage &cache)
{
    if(rect.isNull() || rect.isEmpty())
        return QImage();

    if(!duplicationReady)
    {
        if(!initDuplication())
            return cache; // niente da fare, restituiamo l'ultima valida (se c'è)
    }

    ComPtr<IDXGIResource> desktopResource;
    DXGI_OUTDUPL_FRAME_INFO frameInfo = {};

    // Timeout 0: se non c'e' un frame nuovo, ritorniamo subito.
    HRESULT hr =
        duplication->AcquireNextFrame(
            0,
            &frameInfo,
            desktopResource.GetAddressOf()
            );

    if(hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        // Nulla e' cambiato sullo schermo dall'ultima cattura:
        // la cache e' ancora valida cosi' com'e'.
        return cache;
    }

    if(hr == DXGI_ERROR_ACCESS_LOST)
    {
        reacquireDuplication();
        return cache;
    }

    if(FAILED(hr))
    {
        qDebug() << "AcquireNextFrame failed:" << Qt::hex << hr;
        return cache;
    }

    ComPtr<ID3D11Texture2D> desktopTexture;
    hr = desktopResource.As(&desktopTexture);

    if(FAILED(hr))
    {
        duplication->ReleaseFrame();
        return cache;
    }

    if(!ensureStagingTexture(rect.size()))
    {
        duplication->ReleaseFrame();
        return cache;
    }

    D3D11_BOX srcBox;
    srcBox.left   = static_cast<UINT>(rect.left());
    srcBox.top    = static_cast<UINT>(rect.top());
    srcBox.front  = 0;
    srcBox.right  = static_cast<UINT>(rect.left() + rect.width());
    srcBox.bottom = static_cast<UINT>(rect.top() + rect.height());
    srcBox.back   = 1;

    d3dContext->CopySubresourceRegion(
        stagingTexture.Get(),
        0,
        0, 0, 0,
        desktopTexture.Get(),
        0,
        &srcBox
        );

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = d3dContext->Map(
        stagingTexture.Get(),
        0,
        D3D11_MAP_READ,
        0,
        &mapped
        );

    if(FAILED(hr))
    {
        duplication->ReleaseFrame();
        return cache;
    }

    // DXGI_FORMAT_B8G8R8A8_UNORM ha lo stesso layout in memoria
    // di QImage::Format_ARGB32 su little-endian: copia diretta.
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

    d3dContext->Unmap(stagingTexture.Get(), 0);

    duplication->ReleaseFrame();

    cache = image;
    return image;
}

QPixmap BuffVisionCapture::captureCrop1()
{
    if(cropRect1.isNull() || cropRect1.isEmpty())
    {
        qDebug() << "ERROR: Crop1 invalid" << cropRect1;
        return QPixmap();
    }

    QImage img = grabRegion(cropRect1, cachedCrop1);
    return QPixmap::fromImage(img);
}

QPixmap BuffVisionCapture::captureCrop2()
{
    if(cropRect2.isNull() || cropRect2.isEmpty())
        return QPixmap();

    QImage img = grabRegion(cropRect2, cachedCrop2);
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

void BuffVisionCapture::setCropAreas(QRect crop1, QRect crop2)
{
    cropRect1 = crop1;
    cropRect2 = crop2;

    // Le dimensioni di crop sono cambiate:
    // invalidiamo la cache e la staging texture.
    cachedCrop1 = QImage();
    cachedCrop2 = QImage();
    stagingSize = QSize();
}