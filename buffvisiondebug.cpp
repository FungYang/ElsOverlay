#include "buffvisiondebug.h"

#include <QVBoxLayout>
#include <QLabel>


BuffVisionDebug::BuffVisionDebug(
    QWidget *parent
    )
    : QWidget(parent)
{
    setWindowTitle(
        "BuffVision - Debug"
        );

    setWindowFlags(
        Qt::Window |
        Qt::WindowStaysOnTopHint
        );

    resize(
        300,
        220
        );


    QVBoxLayout *layout =
        new QVBoxLayout(this);


    QLabel *title =
        new QLabel(
            "<b>BUFFVISION DEBUG</b>",
            this
            );

    layout->addWidget(
        title
        );


    resolutionLabel =
        new QLabel(
            "Risoluzione: ---",
            this
            );

    layout->addWidget(
        resolutionLabel
        );


    crop1Label =
        new QLabel(
            "Crop 1: ---",
            this
            );

    crop1Label->setStyleSheet(
        "font-size: 18px;"
        );

    layout->addWidget(
        crop1Label
        );


    crop2Label =
        new QLabel(
            "Crop 2: ---",
            this
            );

    crop2Label->setStyleSheet(
        "font-size: 18px;"
        );

    layout->addWidget(
        crop2Label
        );


    lastEventLabel =
        new QLabel(
            "Ultimo evento: ---",
            this
            );

    layout->addWidget(
        lastEventLabel
        );
}


void BuffVisionDebug::updateNumbers(
    int crop1,
    int crop2
    )
{
    const QString crop1Text =
        crop1 == 1000
            ? "---"
            : QString::number(crop1);

    const QString crop2Text =
        crop2 == 1000
            ? "---"
            : QString::number(crop2);


    crop1Label->setText(
        "Crop 1: <b>" +
        crop1Text +
        "</b>"
        );


    crop2Label->setText(
        "Crop 2: <b>" +
        crop2Text +
        "</b>"
        );
}


void BuffVisionDebug::setResolution(
    int width,
    int height,
    const QSize &cropSize
    )
{
    resolutionLabel->setText(
        QString(
            "Risoluzione: %1 x %2<br>"
            "Crop: %3 x %4"
            )
            .arg(width)
            .arg(height)
            .arg(cropSize.width())
            .arg(cropSize.height())
        );
}


void BuffVisionDebug::setLastEvent(
    const QString &event
    )
{
    lastEventLabel->setText(
        "Ultimo evento: " +
        event
        );
}