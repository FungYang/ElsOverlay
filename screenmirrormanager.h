#pragma once

// ============================================================
// SCREEN MIRROR - MANAGER
//
// Ogni secondo cattura una regione dello schermo (sorgente) e
// la mostra, 1:1 e senza scalare, in un overlay posizionato su
// un'altra regione (destinazione).
//
// La cattura usa il CaptureCoordinator esistente (stesso screen
// capture DXGI delle altre funzioni): qui non c'e' codice di
// cattura nuovo, solo una sottoscrizione a 1 secondo.
// ============================================================

#include <QImage>
#include <QObject>
#include <QPointer>
#include <QRect>

class GlobalKeyboard;
class OverlayRoot;
class ScreenMirrorOverlay;
class ScreenMirrorSetup;

class ScreenMirrorManager : public QObject
{
    Q_OBJECT

public:

    // Intervallo di cattura: 1 secondo.
    static constexpr int CAPTURE_INTERVAL_MS = 100;

    ScreenMirrorManager(
        GlobalKeyboard *keyboard,
        OverlayRoot *overlayRoot,
        QObject *parent = nullptr
        );

    ~ScreenMirrorManager() override;


public slots:

    // Apre la finestra con i due rettangoli (Invio conferma).
    void configure();

    // Toggle ON/OFF della funzione.
    void setEnabled(
        bool enabled
        );

    // Rilegge le regioni salvate e riparte (a funzione attiva).
    // Chiamata alla conferma della configurazione: cosi' le nuove
    // regioni valgono subito, senza riavviare il programma.
    void reloadConfiguration();


private slots:

    // Frame catturato dal CaptureCoordinator (thread GUI).
    void onFrameCaptured(
        quint64 frameId,
        QImage frame
        );


private:

    void loadRects();
    void saveRects() const;

    void startCapture();
    void stopCapture();

    static QString settingsPath();
    static QString resolutionSuffix();


    GlobalKeyboard *m_keyboard = nullptr;
    OverlayRoot *m_overlayRoot = nullptr;

    QPointer<ScreenMirrorSetup> m_setup;
    QPointer<ScreenMirrorOverlay> m_overlay;

    QRect m_source;
    QRect m_dest;

    int m_regionId = -1;

    bool m_enabled = false;
};
