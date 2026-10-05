#include "screencapture.h"

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>

#ifdef QT_DEBUG
#include <QElapsedTimer>
#endif

#include <cstring>


namespace
{

#ifdef QT_DEBUG

struct CaptureProfiler
{
    qint64 frames = 0;

    qint64 acquireNs = 0;
    qint64 copyNs = 0;
    qint64 mapNs = 0;
    qint64 cpuCopyNs = 0;
    qint64 captureNs = 0;

    qint64 regionCount = 0;
    qint64 copyCount = 0;
    qint64 mapCount = 0;


    void reset()
    {
        frames = 0;

        acquireNs = 0;
        copyNs = 0;
        mapNs = 0;
        cpuCopyNs = 0;
        captureNs = 0;

        regionCount = 0;
        copyCount = 0;
        mapCount = 0;
    }


    void print()
    {
        if(frames <= 0)
            return;


        const double divisor =
            static_cast<double>(frames) *
            1000000.0;


        qDebug()
            << "[ScreenCapture profiler]"
            << "frames =" << frames

            << "regions/frame ="
            << (
                   static_cast<double>(
                       regionCount
                       ) /
                   static_cast<double>(
                       frames
                       )
                   )

            << "avg acquire ="
            << (
                   static_cast<double>(
                       acquireNs
                       ) /
                   divisor
                   )
            << "ms"

            << "avg CopySubresourceRegion ="
            << (
                   static_cast<double>(
                       copyNs
                       ) /
                   divisor
                   )
            << "ms"

            << "avg Map ="
            << (
                   static_cast<double>(
                       mapNs
                       ) /
                   divisor
                   )
            << "ms"

            << "avg CPU copy ="
            << (
                   static_cast<double>(
                       cpuCopyNs
                       ) /
                   divisor
                   )
            << "ms"

            << "avg captureRegion ="
            << (
                   static_cast<double>(
                       captureNs
                       ) /
                   divisor
                   )
            << "ms"

            << "Map/frame ="
            << (
                   static_cast<double>(
                       mapCount
                       ) /
                   static_cast<double>(
                       frames
                       )
                   );

        reset();
    }
};


CaptureProfiler g_captureProfiler;

#endif // QT_DEBUG

}


// =========================================================
// STATIC MEMBERS
// =========================================================

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


QHash<int, QRect>
    ScreenCapture::s_atlasRects;


bool
    ScreenCapture::s_atlasDirty = true;


bool
    ScreenCapture::s_atlasMapped = false;


D3D11_MAPPED_SUBRESOURCE
    ScreenCapture::s_mappedAtlas = {};


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
    if(s_frameAcquired)
    {
        if(s_atlasMapped)
        {
            unmapAtlas();
        }

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

    s_atlasRects.clear();
    s_atlasDirty = true;

    s_atlasMapped = false;
    s_mappedAtlas = {};

    s_desktopRect = QRect();

    s_ready = false;


    QScreen *primaryScreen =
        QGuiApplication::primaryScreen();


    QRect targetGeometry =
        primaryScreen
            ? primaryScreen->geometry()
            : QRect();


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


            if(outputRect != targetGeometry)
            {
                ++outputIndex;

                continue;
            }


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


#ifdef QT_DEBUG
            qDebug()
                << "TRANSCENDENCE DPI CHECK:"
                << "Qt screen->geometry() ="
                << targetGeometry
                << " DXGI DesktopCoordinates ="
                << outputRect
                << " devicePixelRatio ="
                << (primaryScreen
                        ? primaryScreen->devicePixelRatio()
                        : -1.0);
#endif


            return true;
        }


        ++adapterIndex;
    }


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
        return -1;
    }


    const int id =
        s_nextRegionId++;


    s_regions.insert(
        id,
        rect
        );


    s_atlasDirty = true;


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


    s_regions.remove(
        regionId
        );


    s_regionCache.remove(
        regionId
        );


    s_atlasRects.remove(
        regionId
        );


    s_atlasDirty = true;
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
    QVector<int> regionIds;

    regionIds.reserve(
        s_regions.size()
        );

    for(auto it = s_regions.constBegin();
         it != s_regions.constEnd();
         ++it)
    {
        regionIds.append(
            it.key()
            );
    }


    return beginFrame(
        regionIds
        );
}


// =========================================================
// BEGIN FRAME — BATCH
// =========================================================

bool ScreenCapture::beginFrame(
    const QVector<int> &regionIds
    )
{
    if(s_frameAcquired)
    {
        return true;
    }


    if(!ensureInit())
        return false;


    if(regionIds.isEmpty())
    {
        return false;
    }


    ComPtr<IDXGIResource>
        desktopResource;


    DXGI_OUTDUPL_FRAME_INFO
        frameInfo = {};


#ifdef QT_DEBUG
    QElapsedTimer acquireTimer;

    acquireTimer.start();
#endif


    HRESULT hr =
        s_duplication->AcquireNextFrame(
            0,
            &frameInfo,
            desktopResource.GetAddressOf()
            );


#ifdef QT_DEBUG
    g_captureProfiler.acquireNs +=
        acquireTimer.nsecsElapsed();
#endif


    if(hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        return false;
    }


    if(hr == DXGI_ERROR_ACCESS_LOST)
    {
        reinit();

        return false;
    }


    if(FAILED(hr))
    {
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
        s_duplication->ReleaseFrame();

        return false;
    }


    s_currentFrame =
        frameTexture;


    s_frameAcquired =
        true;


    // Il layout dell'atlas viene ricostruito solo quando
    // cambia la configurazione delle ROI.
    if(
        s_atlasDirty &&
        !rebuildAtlas()
        )
    {
        endFrame();

        return false;
    }


    // Tutte le copie GPU vengono accodate prima del Map().
    if(
        !copyRegionsToAtlas(
            regionIds
            )
        )
    {
        endFrame();

        return false;
    }


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
// REBUILD ATLAS
// =========================================================

bool ScreenCapture::rebuildAtlas()
{
    s_atlasRects.clear();


    if(s_regions.isEmpty())
    {
        s_staging.Reset();
        s_stagingSize = QSize();
        s_atlasDirty = false;

        return true;
    }


    // Packing a righe.
    //
    // Manteniamo l'atlas compatto senza creare un enorme
    // bounding box tra ROI distanti.
    const int maxRowWidth = 4096;


    int x = 0;
    int y = 0;
    int rowHeight = 0;


    for(auto it = s_regions.constBegin();
         it != s_regions.constEnd();
         ++it)
    {
        const int id =
            it.key();

        const QRect rect =
            it.value();


        const int width =
            rect.width();

        const int height =
            rect.height();


        if(
            x > 0 &&
            x + width > maxRowWidth
            )
        {
            x = 0;

            y += rowHeight;

            rowHeight = 0;
        }


        s_atlasRects.insert(
            id,
            QRect(
                x,
                y,
                width,
                height
                )
            );


        x += width;

        rowHeight =
            qMax(
                rowHeight,
                height
                );
    }


    const int atlasWidth =
        qMax(
            1,
            qMin(
                maxRowWidth,
                x > 0 ? x : maxRowWidth
                )
            );


    int atlasHeight =
        y + rowHeight;


    if(atlasHeight <= 0)
    {
        atlasHeight = 1;
    }


    const QSize atlasSize(
        atlasWidth,
        atlasHeight
        );


    if(
        !ensureAtlas(
            atlasSize
            )
        )
    {
        s_atlasRects.clear();

        return false;
    }


    s_atlasDirty = false;

    return true;
}


// =========================================================
// ENSURE ATLAS
// =========================================================

bool ScreenCapture::ensureAtlas(
    const QSize &size
    )
{
    if(size.isEmpty())
        return false;


    if(
        s_staging &&
        s_stagingSize == size
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


    ComPtr<ID3D11Texture2D> newStaging;


    HRESULT hr =
        s_device->CreateTexture2D(
            &desc,
            nullptr,
            newStaging.GetAddressOf()
            );


    if(FAILED(hr))
    {
        return false;
    }


    s_staging =
        newStaging;

    s_stagingSize =
        size;


    return true;
}


// =========================================================
// COPY REGIONS TO ATLAS
// =========================================================

bool ScreenCapture::copyRegionsToAtlas(
    const QVector<int> &regionIds
    )
{
    if(!s_staging)
        return false;


#ifdef QT_DEBUG
    QElapsedTimer copyTimer;

    copyTimer.start();
#endif


    int copiedCount = 0;


    for(const int regionId : regionIds)
    {
        if(!s_regions.contains(regionId))
        {
            continue;
        }


        if(!s_atlasRects.contains(regionId))
        {
            continue;
        }


        const QRect sourceRect =
            s_regions.value(
                regionId
                );


        const QRect atlasRect =
            s_atlasRects.value(
                regionId
                );


        QRect validRect =
            sourceRect.intersected(
                s_desktopRect
                );


        if(validRect.isEmpty())
        {
            continue;
        }


        if(validRect.size() != sourceRect.size())
        {
            continue;
        }


        D3D11_BOX srcBox = {};


        srcBox.left =
            static_cast<UINT>(
                sourceRect.left()
                );


        srcBox.top =
            static_cast<UINT>(
                sourceRect.top()
                );


        srcBox.front =
            0;


        srcBox.right =
            static_cast<UINT>(
                sourceRect.left() +
                sourceRect.width()
                );


        srcBox.bottom =
            static_cast<UINT>(
                sourceRect.top() +
                sourceRect.height()
                );


        srcBox.back =
            1;


        s_context->CopySubresourceRegion(
            s_staging.Get(),
            0,

            static_cast<UINT>(
                atlasRect.x()
                ),

            static_cast<UINT>(
                atlasRect.y()
                ),

            0,

            s_currentFrame.Get(),
            0,

            &srcBox
            );


        ++copiedCount;
    }


#ifdef QT_DEBUG
    g_captureProfiler.copyNs +=
        copyTimer.nsecsElapsed();

    g_captureProfiler.copyCount +=
        copiedCount;
#endif


    return copiedCount > 0;
}


// =========================================================
// MAP ATLAS
// =========================================================

bool ScreenCapture::mapAtlas()
{
    if(s_atlasMapped)
        return true;


    if(!s_staging)
        return false;


#ifdef QT_DEBUG
    QElapsedTimer mapTimer;

    mapTimer.start();
#endif


    D3D11_MAPPED_SUBRESOURCE mapped = {};


    HRESULT hr =
        s_context->Map(
            s_staging.Get(),
            0,
            D3D11_MAP_READ,
            0,
            &mapped
            );


#ifdef QT_DEBUG
    g_captureProfiler.mapNs +=
        mapTimer.nsecsElapsed();

    ++g_captureProfiler.mapCount;
#endif


    if(FAILED(hr))
    {
        return false;
    }


    s_mappedAtlas =
        mapped;

    s_atlasMapped =
        true;


    return true;
}


// =========================================================
// UNMAP ATLAS
// =========================================================

void ScreenCapture::unmapAtlas()
{
    if(!s_atlasMapped)
        return;


    s_context->Unmap(
        s_staging.Get(),
        0
        );


    s_mappedAtlas = {};

    s_atlasMapped =
        false;
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
        return QImage();
    }


    // Trova la ROI corrispondente nell'atlas.
    int regionId = -1;


    for(auto it = s_regions.constBegin();
         it != s_regions.constEnd();
         ++it)
    {
        if(it.value() == rect)
        {
            regionId = it.key();

            break;
        }
    }


    if(regionId < 0)
    {
        return QImage();
    }


    if(!s_atlasRects.contains(regionId))
    {
        return QImage();
    }


    if(!mapAtlas())
    {
        return QImage();
    }


    const QRect atlasRect =
        s_atlasRects.value(
            regionId
            );


    QImage image(
        rect.width(),
        rect.height(),
        QImage::Format_ARGB32
        );


    if(image.isNull())
    {
        return QImage();
    }


#ifdef QT_DEBUG
    QElapsedTimer cpuCopyTimer;

    cpuCopyTimer.start();
#endif


    const uchar *src =
        static_cast<const uchar *>(
            s_mappedAtlas.pData
            );


    for(int y = 0;
         y < rect.height();
         ++y)
    {
        std::memcpy(
            image.scanLine(y),

            src +
                (atlasRect.y() + y) *
                    s_mappedAtlas.RowPitch +
                atlasRect.x() * 4,

            static_cast<size_t>(
                rect.width()
                ) * 4
            );
    }


#ifdef QT_DEBUG
    g_captureProfiler.cpuCopyNs +=
        cpuCopyTimer.nsecsElapsed();
#endif


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
        return QImage();
    }


    if(!s_frameAcquired)
    {
        return s_regionCache.value(
            regionId
            );
    }


#ifdef QT_DEBUG
    QElapsedTimer captureTimer;

    captureTimer.start();
#endif


    const QRect rect =
        s_regions.value(
            regionId
            );


    QImage image =
        captureRectFromCurrentFrame(
            rect
            );


#ifdef QT_DEBUG
    g_captureProfiler.captureNs +=
        captureTimer.nsecsElapsed();

    ++g_captureProfiler.regionCount;
#endif


    if(!image.isNull())
    {
        s_regionCache.insert(
            regionId,
            image
            );


        return image;
    }


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


    if(s_atlasMapped)
    {
        unmapAtlas();
    }


    if(s_duplication)
    {
        s_duplication->ReleaseFrame();
    }


    s_currentFrame.Reset();

    s_frameAcquired =
        false;


#ifdef QT_DEBUG

    ++g_captureProfiler.frames;


    if(
        (g_captureProfiler.frames % 100) == 0
        )
    {
        g_captureProfiler.print();
    }

#endif
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


    QVector<int> regionIds;

    regionIds.append(
        tempId
        );


    if(beginFrame(regionIds))
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
    if(rect.isNull() || rect.isEmpty())
    {
        return false;
    }


    if(!s_regions.contains(regionId))
    {
        return false;
    }


    s_regions[regionId] =
        rect;


    s_regionCache.remove(
        regionId
        );


    s_atlasDirty = true;


    return true;
}