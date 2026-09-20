#include "transcendencedebugoverlay.h"

#ifdef QT_DEBUG

TranscendenceDebugOverlay::TranscendenceDebugOverlay(QWidget *parent)
    : QLabel(parent)
{
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    setAttribute(
        Qt::WA_TransparentForMouseEvents,
        true
        );

    setStyleSheet(
        "QLabel {"
        "background: rgba(0, 0, 0, 150);"
        "color: rgba(255, 255, 255, 220);"
        "padding: 5px 9px;"
        "border-radius: 6px;"
        "font-size: 12px;"
        "}"
        );

    setText("Transcendenza: -- ms");

    adjustSize();
}

void TranscendenceDebugOverlay::setSearching()
{
    setText("Transcendenza: ricerca...");

    adjustSize();
    show();
}

void TranscendenceDebugOverlay::setFirstFindTime(
    qint64 milliseconds
    )
{
    setText(
        QString("Transcendenza: %1 ms")
            .arg(milliseconds)
        );

    adjustSize();
    show();
}

#endif