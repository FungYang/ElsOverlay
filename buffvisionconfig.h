#ifndef BUFFVISIONCONFIG_H
#define BUFFVISIONCONFIG_H

#include <QSize>

namespace BuffVisionConfig
{

constexpr int FULL_HD_WIDTH  = 1920;
constexpr int FULL_HD_HEIGHT = 1080;
constexpr int FULL_HD_CROP_SIZE = 25;

constexpr int TWO_K_WIDTH  = 2560;
constexpr int TWO_K_HEIGHT = 1440;
constexpr int TWO_K_CROP_SIZE = 33;


inline int cropSizeForScreen(const QSize &screenSize)
{
    if(screenSize.width() == TWO_K_WIDTH &&
        screenSize.height() == TWO_K_HEIGHT)
    {
        return TWO_K_CROP_SIZE;
    }

    // Full HD e fallback
    return FULL_HD_CROP_SIZE;
}


inline QSize cropSizeForScreen(const QSize &screenSize, bool)
{
    const int size = cropSizeForScreen(screenSize);

    return QSize(size, size);
}

}

#endif