#pragma once

#include <QObject>
#include <QImage>
#include <QVector>

class ResonanceGateWorker : public QObject
{
    Q_OBJECT

public:
    static constexpr int TOLERANCE = 35;
    static constexpr double MATCH_RATIO = 0.92;
    static constexpr int FOREGROUND_SATURATION_THRESHOLD = 40;
    static constexpr int FOREGROUND_BRIGHTNESS_THRESHOLD = 170;

    explicit ResonanceGateWorker(QObject *parent = nullptr);

public slots:
    void loadReference(QString imagesDir); // legge invariant.png
    void compareFrame(QImage frame);

signals:
    void referenceLoaded(bool ok);
    void compared(bool isMatch);

#ifdef QT_DEBUG
    void debugFrame(QImage frame, bool isMatch);
#endif

private:
    QImage m_reference;
    QVector<bool> m_mask;

    static bool isForegroundPixel(QRgb pixel);
    static QVector<bool> buildForegroundMask(const QImage &reference);
    static bool matches(const QImage &current, const QImage &reference,
                        const QVector<bool> &mask, int tolerance);
};