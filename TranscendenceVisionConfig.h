#pragma once

namespace TranscendenceVisionConfig
{
constexpr int ICON_WIDTH  = 28;
constexpr int ICON_HEIGHT = 28;

// Dimensione del riquadro giallo mostrato nel setup.
// Il template reale rimane sempre 28x28.
constexpr int ICON_BOX_WIDTH  = 84;
constexpr int ICON_BOX_HEIGHT = 84;

// Ingrandimento della finestra di precisione.
constexpr int PRECISION_ZOOM = 3;

// Tempo di attesa dopo l'inizio del cooldown
// prima che la scansione inizi.
constexpr int DELAY_MS = 18000;

// Frequenza di scansione una volta attiva.
constexpr int SCAN_INTERVAL_MS = 20;

constexpr double MATCH_THRESHOLD = 95.0;
constexpr int    PIXEL_TOLERANCE = 15;
constexpr double FAST_TOLERANCE  = 70.0;
}
