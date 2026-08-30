#pragma once

namespace TranscendenceVisionConfig
{
constexpr int ICON_WIDTH  = 28;
constexpr int ICON_HEIGHT = 28;

// Tempo di attesa dopo l'inizio del cooldown
// prima che la scansione inizi.
constexpr int DELAY_MS = 18000;

// Frequenza di scansione una volta attiva.
constexpr int SCAN_INTERVAL_MS = 150;

constexpr double MATCH_THRESHOLD = 85.0;
constexpr int    PIXEL_TOLERANCE = 20;
constexpr double FAST_TOLERANCE  = 70.0;
}