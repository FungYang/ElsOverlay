#pragma once

// ============================================================
// SCREEN MIRROR - OVERLAY
//
// Finestra che mostra, 1:1 e senza scalare, l'immagine catturata
// dalla regione sorgente. Sta nella regione di destinazione: se
// la destinazione e' piu' piccola dell'immagine questa viene
// tagliata, se e' piu' grande il resto resta trasparente.
// ============================================================

#include <QImage>
#include <QRect>
#include <QWidget>

class ScreenMirrorOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit ScreenMirrorOverlay(
        QWidget *parent = nullptr
        );

    // Posizione e dimensione della finestra (regione di destinazione).
    void setRegion(
        const QRect &region
        );

    // Nuova immagine da mostrare (pixel 1:1).
    void setImage(
        const QImage &image
        );

    void clearImage();

    const QImage &image() const;


protected:

    void paintEvent(
        QPaintEvent *event
        ) override;


private:

    QImage m_image;
};
