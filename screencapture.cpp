#include "screencapture.h"

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>

#include <cstring>


ComPtr<ID3D11Device>
    ScreenCapture::s_device;

ComPtr<ID3D11DeviceContext>
    ScreenCapture::s_context;

ComPtr<IDXGIOutputDuplication>
    ScreenCapture::s_duplication;

ComPtr<ID3D11Texture2D>
    ScreenCapture::s_staging;

ComPtr<ID3D11Texture2D>
    ScreenCapture::s_currentFrame;


bool
    ScreenCapture::s_frameAcquired = false;


QSize
    ScreenCapture::s_stagingSize;


QRect
    ScreenCapture::s_desktopRect;


bool
    ScreenCapture::s_ready = false;


int
    ScreenCapture::s_nextRegionId = 1;


QHash<int, QRect>
    ScreenCapture::s_regions;


QHash<int, QImage>
    ScreenCapture::s_regionCache;


// =========================================================
// INIT
// =========================================================

bool ScreenCapture::ensureInit()
{
    if(s_ready)
        return true;

    return reinit();
}


// =========================================================
// REINIT
// =========================================================

bool ScreenCapture::reinit()
{
    // Se per qualche motivo avevamo ancora un frame acquisito,
    // dobbiamo prima abbandonarlo.
    //
    // In condizioni normali non dovrebbe succedere, ma evitiamo
    // di lasciare lo stato interno incoerente.

    if(s_frameAcquired)
    {
        if(s_duplication)
        {
            s_duplication->ReleaseFrame();
        }

        s_frameAcquired = false;
    }


    s_currentFrame.Reset();
    s_staging.Reset();
    s_duplication.Reset();
    s_context.Reset();
    s_device.Reset();

    s_stagingSize = QSize();

    s_desktopRect = QRect();

    s_ready = false;


    QScreen *primaryScreen =
        QGuiApplication::primaryScreen();


    QRect targetGeometry =
        primaryScreen
            ? primaryScreen->geometry()
            : QRect();


    // qDebug()
    //     << "ScreenCapture: target geometry:"
    //     << targetGeometry;


    ComPtr<IDXGIFactory1> factory;


    HRESULT hr =
        CreateDXGIFactory1(
            __uuidof(IDXGIFactory1),
            reinterpret_cast<void **>(
                factory.GetAddressOf()
                )
            );


    if(FAILED(hr))
    {
        qDebug()
        << "ScreenCapture: CreateDXGIFactory1 failed:"
        << Qt::hex
        << hr;

        return false;
    }


    UINT adapterIndex = 0;

    ComPtr<IDXGIAdapter1> adapter;


    while(
        factory->EnumAdapters1(
            adapterIndex,
            adapter.ReleaseAndGetAddressOf()
            )
        != DXGI_ERROR_NOT_FOUND)
    {
        UINT outputIndex = 0;

        ComPtr<IDXGIOutput> output;


        while(
            adapter->EnumOutputs(
                outputIndex,
                output.ReleaseAndGetAddressOf()
                )
            != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC outputDesc;


            if(
                FAILED(
                    output->GetDesc(
                        &outputDesc
                        )
                    )
                )
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


            // Per ora manteniamo la stessa scelta
            // dell'implementazione precedente:
            //
            // catturiamo lo schermo primario.

            if(outputRect != targetGeometry)
            {
                ++outputIndex;

                continue;
            }


            qDebug()
                << "ScreenCapture: output trovato:"
                << QString::fromWCharArray(
                       outputDesc.DeviceName
                       )
                << "adapter"
                << adapterIndex
                << "output"
                << outputIndex;


            ComPtr<ID3D11Device>
                testDevice;

            ComPtr<ID3D11DeviceContext>
                testContext;

            D3D_FEATURE_LEVEL
                testLevel;


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
                qDebug()
                << "ScreenCapture:"
                << "D3D11CreateDevice failed:"
                << Qt::hex
                << devHr;

                return false;
            }


            ComPtr<IDXGIOutput1>
                output1;


            HRESULT asHr =
                output.As(
                    &output1
                    );


            if(FAILED(asHr))
            {
                qDebug()
                << "ScreenCapture:"
                << "QueryInterface IDXGIOutput1 failed:"
                << Qt::hex
                << asHr;

                return false;
            }


            ComPtr<IDXGIOutputDuplication>
                duplication;


            HRESULT dupHr =
                output1->DuplicateOutput(
                    testDevice.Get(),
                    duplication.GetAddressOf()
                    );


            if(FAILED(dupHr))
            {
                qDebug()
                << "ScreenCapture:"
                << "DuplicateOutput failed:"
                << Qt::hex
                << dupHr;

                return false;
            }


            s_device =
                testDevice;

            s_context =
                testContext;

            s_duplication =
                duplication;

            s_desktopRect =
                outputRect;

            s_ready =
                true;


            qDebug()
                << "ScreenCapture:"
                << "DXGI duplication attiva su"
                << QString::fromWCharArray(
                       outputDesc.DeviceName
                       );


            return true;
        }


        ++adapterIndex;
    }


    qDebug()
        << "ScreenCapture:"
        << "nessun output corrisponde allo schermo primario:"
        << targetGeometry;


    return false;
}


// =========================================================
// REGION REGISTRATION
// =========================================================

int ScreenCapture::registerRegion(
    const QRect &rect
    )
{
    if(rect.isNull() || rect.isEmpty())
    {
        qDebug()
        << "ScreenCapture:"
        << "tentativo di registrare rect invalido:"
        << rect;

        return -1;
    }


    const int id =
        s_nextRegionId++;


    s_regions.insert(
        id,
        rect
        );


    qDebug()
        << "ScreenCapture:"
        << "region registered:"
        << "id =" << id
        << "rect =" << rect;


    return id;
}


// =========================================================
// REGION UNREGISTER
// =========================================================

void ScreenCapture::unregisterRegion(
    int regionId
    )
{
    if(!s_regions.contains(regionId))
        return;


    // qDebug()
    //     << "ScreenCapture:"
    //     << "region unregistered:"
    //     << "id =" << regionId;


    s_regions.remove(
        regionId
        );


    s_regionCache.remove(
        regionId
        );
}


// =========================================================
// REGION EXISTS
// =========================================================

bool ScreenCapture::isRegionRegistered(
    int regionId
    )
{
    return s_regions.contains(
        regionId
        );
}


// =========================================================
// REGION RECT
// =========================================================

QRect ScreenCapture::regionRect(
    int regionId
    )
{
    return s_regions.value(
        regionId
        );
}


// =========================================================
// BEGIN FRAME
// =========================================================

bool ScreenCapture::beginFrame()
{
    if(s_frameAcquired)
    {
        qDebug()
        << "ScreenCapture:"
        << "beginFrame chiamato mentre un frame"
        << "e' gia' acquisito";

        return true;
    }


    if(!ensureInit())
        return false;


    if(s_regions.isEmpty())
    {
        qDebug()
        << "ScreenCapture:"
        << "beginFrame: nessuna regione registrata";

        return false;
    }


    ComPtr<IDXGIResource>
        desktopResource;


    DXGI_OUTDUPL_FRAME_INFO
        frameInfo = {};


    HRESULT hr =
        s_duplication->AcquireNextFrame(
            0,
            &frameInfo,
            desktopResource.GetAddressOf()
            );


    if(hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        qDebug()
        << "ScreenCapture:"
        << "DXGI WAIT_TIMEOUT";

        return false;
    }


    if(hr == DXGI_ERROR_ACCESS_LOST)
    {
        qDebug()
        << "ScreenCapture:"
        << "DXGI ACCESS_LOST - reinit";


        reinit();

        return false;
    }


    if(FAILED(hr))
    {
        qDebug()
        << "ScreenCapture:"
        << "AcquireNextFrame failed:"
        << Qt::hex
        << hr;

        return false;
    }


    ComPtr<ID3D11Texture2D>
        frameTexture;


    hr =
        desktopResource.As(
            &frameTexture
            );


    if(FAILED(hr))
    {
        qDebug()
        << "ScreenCapture:"
        << "frame resource non convertibile:"
        << Qt::hex
        << hr;


        s_duplication->ReleaseFrame();

        return false;
    }


    s_currentFrame =
        frameTexture;


    s_frameAcquired =
        true;


    qDebug()
        << "DXGI NEW FRAME:"
        << "active regions ="
        << s_regions.size();


    return true;
}


// =========================================================
// HAS FRAME
// =========================================================

bool ScreenCapture::hasFrame()
{
    return s_frameAcquired;
}


// =========================================================
// ENSURE STAGING
// =========================================================

bool ScreenCapture::ensureStaging(
    const QSize &size
    )
{
    if(size.isEmpty())
        return false;


    // La staging esistente viene riutilizzata
    // anche se è più grande della regione richiesta.

    if(
        s_staging &&
        s_stagingSize.width() >= size.width() &&
        s_stagingSize.height() >= size.height()
        )
    {
        return true;
    }


    D3D11_TEXTURE2D_DESC desc = {};


    desc.Width =
        static_cast<UINT>(
            size.width()
            );


    desc.Height =
        static_cast<UINT>(
            size.height()
            );


    desc.MipLevels =
        1;

    desc.ArraySize =
        1;


    desc.Format =
        DXGI_FORMAT_B8G8R8A8_UNORM;


    desc.SampleDesc.Count =
        1;


    desc.Usage =
        D3D11_USAGE_STAGING;


    desc.CPUAccessFlags =
        D3D11_CPU_ACCESS_READ;


    s_staging.Reset();


    HRESULT hr =
        s_device->CreateTexture2D(
            &desc,
            nullptr,
            s_staging.GetAddressOf()
            );


    if(FAILED(hr))
    {
        qDebug()
        << "ScreenCapture:"
        << "CreateTexture2D staging failed:"
        << Qt::hex
        << hr;


        s_stagingSize =
            QSize();


        return false;
    }


    s_stagingSize =
        size;


    qDebug()
        << "ScreenCapture:"
        << "staging resized:"
        << size;


    return true;
}


// =========================================================
// CAPTURE RECT FROM CURRENT FRAME
// =========================================================

QImage ScreenCapture::captureRectFromCurrentFrame(
    const QRect &rect
    )
{
    if(!s_frameAcquired)
        return QImage();


    if(!s_currentFrame)
        return QImage();


    if(rect.isNull() || rect.isEmpty())
        return QImage();


    // Il rettangolo deve appartenere
    // al desktop catturato.

    QRect validRect =
        rect.intersected(
            s_desktopRect
            );


    if(validRect.isEmpty())
        return QImage();


    if(
        validRect.size()
        !=
        rect.size()
        )
    {
        qDebug()
        << "ScreenCapture:"
        << "rect fuori dal desktop:"
        << rect
        << "desktop:"
        << s_desktopRect;

        return QImage();
    }


    if(
        !ensureStaging(
            rect.size()
            )
        )
    {
        return QImage();
    }


    D3D11_BOX srcBox = {};


    srcBox.left =
        static_cast<UINT>(
            rect.left()
            );


    srcBox.top =
        static_cast<UINT>(
            rect.top()
            );


    srcBox.front =
        0;


    srcBox.right =
        static_cast<UINT>(
            rect.left() +
            rect.width()
            );


    srcBox.bottom =
        static_cast<UINT>(
            rect.top() +
            rect.height()
            );


    srcBox.back =
        1;


    s_context->CopySubresourceRegion(
        s_staging.Get(),
        0,

        0,
        0,
        0,

        s_currentFrame.Get(),
        0,

        &srcBox
        );


    D3D11_MAPPED_SUBRESOURCE mapped = {};


    HRESULT hr =
        s_context->Map(
            s_staging.Get(),
            0,
            D3D11_MAP_READ,
            0,
            &mapped
            );


    if(FAILED(hr))
    {
        qDebug()
        << "ScreenCapture:"
        << "Map failed:"
        << Qt::hex
        << hr
        << "rect:"
        << rect;

        return QImage();
    }


    QImage image(
        rect.width(),
        rect.height(),
        QImage::Format_ARGB32
        );


    const uchar *src =
        static_cast<const uchar *>(
            mapped.pData
            );


    for(int y = 0;
         y < rect.height();
         ++y)
    {
        std::memcpy(
            image.scanLine(y),

            src +
                y * mapped.RowPitch,

            static_cast<size_t>(
                rect.width()
                ) * 4
            );
    }


    s_context->Unmap(
        s_staging.Get(),
        0
        );


    return image;
}


// =========================================================
// CAPTURE REGION
// =========================================================

QImage ScreenCapture::captureRegion(
    int regionId
    )
{
    if(!s_regions.contains(regionId))
    {
        qDebug()
        << "ScreenCapture:"
        << "region ID non registrato:"
        << regionId;

        return QImage();
    }


    if(!s_frameAcquired)
    {
        qDebug()
        << "ScreenCapture:"
        << "captureRegion chiamata senza beginFrame:"
        << regionId;

        return s_regionCache.value(
            regionId
            );
    }


    const QRect rect =
        s_regions.value(
            regionId
            );


    QImage image =
        captureRectFromCurrentFrame(
            rect
            );


    if(!image.isNull())
    {
        s_regionCache.insert(
            regionId,
            image
            );

        qDebug()
            << "ScreenCapture:"
            << "region captured:"
            << "id =" << regionId
            << "rect =" << rect
            << "size =" << image.size();


        return image;
    }


    // Se la cattura fallisce, manteniamo
    // l'ultima immagine valida.

    return s_regionCache.value(
        regionId
        );
}


// =========================================================
// END FRAME
// =========================================================

void ScreenCapture::endFrame()
{
    if(!s_frameAcquired)
        return;


    if(s_duplication)
    {
        s_duplication->ReleaseFrame();
    }


    s_currentFrame.Reset();

    s_frameAcquired =
        false;


    qDebug()
        << "ScreenCapture:"
        << "DXGI frame released";
}


// =========================================================
// COMPATIBILITY: CAPTURE SCREEN
// =========================================================

QImage ScreenCapture::captureScreen(
    QScreen *screen
    )
{
    Q_UNUSED(screen);


    if(!ensureInit())
        return QImage();


    const int tempId =
        registerRegion(
            s_desktopRect
            );


    if(tempId < 0)
        return QImage();


    QImage result;


    if(beginFrame())
    {
        result =
            captureRegion(
                tempId
                );

        endFrame();
    }
    else
    {
        result =
            s_regionCache.value(
                tempId
                );
    }


    unregisterRegion(
        tempId
        );


    return result;
}


// =========================================================
// CROP
// =========================================================

QImage ScreenCapture::crop(
    const QImage &image,
    const QRect &area
    )
{
    if(image.isNull())
        return QImage();


    QRect validArea =
        area.intersected(
            image.rect()
            );


    if(validArea.isEmpty())
        return QImage();


    return image.copy(
        validArea
        );
}


// =========================================================
// RELIABLE FALLBACK
// =========================================================

QImage ScreenCapture::captureRegionReliable(
    QScreen *screen,
    const QRect &area
    )
{
    if(!screen)
        return QImage();


    if(area.isNull() || area.isEmpty())
        return QImage();


    QPixmap pixmap =
        screen->grabWindow(
            0,
            area.x(),
            area.y(),
            area.width(),
            area.height()
            );


    return pixmap
        .toImage()
        .convertToFormat(
            QImage::Format_ARGB32
            );
}

// =========================================================
// REGION UPDATE
// =========================================================

bool ScreenCapture::updateRegion(
    int regionId,
    const QRect &rect
    )
{
    if (rect.isNull() || rect.isEmpty())
    {
        qDebug()
        << "ScreenCapture:"
        << "tentativo di aggiornare con rect invalido:"
        << rect;

        return false;
    }

    if (!s_regions.contains(regionId))
    {
        qDebug()
        << "ScreenCapture:"
        << "updateRegion: ID non registrato:"
        << regionId;

        return false;
    }

    const QRect oldRect =
        s_regions.value(regionId);

    s_regions[regionId] = rect;

    // La cache appartiene alla vecchia posizione.
    // Va quindi eliminata.
    s_regionCache.remove(regionId);

    qDebug()
        << "ScreenCapture:"
        << "region updated:"
        << "id =" << regionId
        << "old =" << oldRect
        << "new =" << rect;

    return true;
}