#pragma once

#include <QObject>
#include <QImage>
#include <QVector>
#include <array>

class AtmaZoneWorker : public QObject
{
    Q_OBJECT

public:
    static constexpr int RED_COUNT = 6;
    static constexpr int TOLERANCE = 35;

    // Percentuale minima di pixel di foreground (testo/icona) che
    // devono combaciare entro TOLERANCE perché la zona sia "Match".
    static constexpr double MATCH_RATIO = 0.92;

    // Un pixel della reference è "foreground" (testo/icona da
    // controllare) se sufficientemente saturo o luminoso da
    // distinguersi dallo sfondo grigio/nero variabile del pannello
    // semitrasparente. Pixel che non superano queste soglie sono
    // considerati sfondo e vengono ignorati nel confronto.
    static constexpr int FOREGROUND_SATURATION_THRESHOLD = 40;
    static constexpr int FOREGROUND_BRIGHTNESS_THRESHOLD = 170;

    explicit AtmaZoneWorker(QObject *parent = nullptr);

public slots:
    // Caricamento riferimenti: eseguito sul thread worker.
    void loadReferences(QString imagesDir);

    // Un frame per zona rossa (index 0..5) o per il blu (index = -1).
    void compareRedFrame(int index, QImage frame);

signals:
    void referencesLoaded(bool ok);

    // isMatch = risultato del confronto per questo frame.
    // La logica di stato/transizione resta nel manager (thread GUI).
    void redCompared(int index, bool isMatch);

#ifdef QT_DEBUG
    void redDebugFrame(int index, QImage frame, bool isMatch);
#endif

private:
    std::array<QImage, RED_COUNT> m_redReferences;

    // Maschera di foreground per ciascuna reference: true = pixel
    // di testo/icona da controllare, false = sfondo da ignorare.
    // Stessa dimensione dell'immagine corrispondente (layout riga per riga).
    std::array<QVector<bool>, RED_COUNT> m_redMasks;

    static QVector<bool> buildForegroundMask(const QImage &reference);
    static bool isForegroundPixel(QRgb pixel);

    // Confronto con maschera: conta solo i pixel di foreground,
    // richiede che almeno MATCH_RATIO di essi combaci entro tolerance.
    // Se la maschera è vuota (nessun foreground rilevato), ripiega
    // sul confronto pixel-perfect completo come fallback di sicurezza.
    static bool matches(
        const QImage &current,
        const QImage &reference,
        const QVector<bool> &mask,
        int tolerance
        );
};