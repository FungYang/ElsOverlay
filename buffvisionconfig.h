#ifndef BUFFVISIONCONFIG_H
#define BUFFVISIONCONFIG_H

#include <QSize>

namespace BuffVisionConfig
{

constexpr int FULL_HD_WIDTH  = 1920;
constexpr int FULL_HD_HEIGHT = 1080;

constexpr int FULL_HD_CROP_WIDTH  = 25;
constexpr int FULL_HD_CROP_HEIGHT = 15;


constexpr int TWO_K_WIDTH  = 2560;
constexpr int TWO_K_HEIGHT = 1440;

constexpr int TWO_K_CROP_WIDTH  = 35;
constexpr int TWO_K_CROP_HEIGHT = 20;


inline QSize cropSizeForScreen(
    const QSize &screenSize
    )
{
    if(
        screenSize.width() == TWO_K_WIDTH &&
        screenSize.height() == TWO_K_HEIGHT
        )
    {
        return QSize(
            TWO_K_CROP_WIDTH,
            TWO_K_CROP_HEIGHT
            );
    }

    // Full HD e fallback

    return QSize(
        FULL_HD_CROP_WIDTH,
        FULL_HD_CROP_HEIGHT
        );
}


inline bool is2K(
    const QSize &screenSize
    )
{
    return
        screenSize.width() == TWO_K_WIDTH &&
        screenSize.height() == TWO_K_HEIGHT;
}

}

#endif