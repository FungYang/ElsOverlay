#pragma once

#include <QObject>
#include <QImage>
#include <array>

class AtmaZoneWorker : public QObject
{
    Q_OBJECT

public:
    static constexpr int RED_COUNT = 6;
    static constexpr int TOLERANCE = 35;

    explicit AtmaZoneWorker(QObject *parent = nullptr);

public slots:
    // Caricamento riferimenti: eseguito sul thread worker.
    void loadReferences(QString imagesDir);

    // Un frame per zona rossa (index 0..5) o per il blu (index = -1).
    void compareRedFrame(int index, QImage frame);
    void compareBlueFrame(QImage frame);

signals:
    void referencesLoaded(bool ok);

    // isMatch = risultato del confronto per questo frame.
    // La logica di stato/transizione resta nel manager (thread GUI).
    void redCompared(int index, bool isMatch);
    void blueCompared(bool isMatch);

#ifdef QT_DEBUG
    void redDebugFrame(int index, QImage frame, bool isMatch);
    void blueDebugFrame(QImage frame, bool isMatch);
#endif

private:
    std::array<QImage, RED_COUNT> m_redReferences;
    QImage m_blueReference;

    static bool matches(const QImage &current, const QImage &reference, int tolerance);
};