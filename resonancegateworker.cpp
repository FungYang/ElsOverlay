#include "resonancegateworker.h"

ResonanceGateWorker::ResonanceGateWorker(QObject *parent) : QObject(parent) {}

void ResonanceGateWorker::loadReference(QString imagesDir)
{
    QImage img;
    const bool ok = img.load(imagesDir + "invariant.png");

    if (ok)
    {
        m_reference = img.convertToFormat(QImage::Format_ARGB32);
        m_mask = buildForegroundMask(m_reference);
    }

    emit referenceLoaded(ok);
}

void ResonanceGateWorker::compareFrame(QImage frame)
{
    const bool isMatch = matches(frame, m_reference, m_mask, TOLERANCE);

#ifdef QT_DEBUG
    emit debugFrame(frame, isMatch);
#endif

    emit compared(isMatch);
}

bool ResonanceGateWorker::isForegroundPixel(QRgb pixel)
{
    const int r = qRed(pixel);
    const int g = qGreen(pixel);
    const int b = qBlue(pixel);

    const int maxC = qMax(r, qMax(g, b));
    const int minC = qMin(r, qMin(g, b));
    const int saturation = maxC - minC; // proxy di saturazione, 0 = grigio puro

    if (saturation >= FOREGROUND_SATURATION_THRESHOLD)
        return true; // colore vivido -> testo/icona

    if (maxC >= FOREGROUND_BRIGHTNESS_THRESHOLD)
        return true; // molto luminoso (es. bianco) -> testo/icona

    return false; // grigio scuro/spento -> sfondo variabile, ignorato
}

QVector<bool> ResonanceGateWorker::buildForegroundMask(const QImage &reference)
{
    QVector<bool> mask;

    if (reference.isNull())
        return mask;

    const int w = reference.width();
    const int h = reference.height();

    mask.resize(w * h);

    for (int y = 0; y < h; ++y)
    {
        const QRgb *line = reinterpret_cast<const QRgb *>(reference.constScanLine(y));

        for (int x = 0; x < w; ++x)
            mask[y * w + x] = isForegroundPixel(line[x]);
    }

    return mask;
}

// ============================================================
// MATCH (con maschera di foreground)
// ============================================================

bool ResonanceGateWorker::matches(
    const QImage &current,
    const QImage &reference,
    const QVector<bool> &mask,
    int tolerance
    )
{
    if (current.isNull() || reference.isNull())
        return false;

    if (current.size() != reference.size())
        return false;

    const int w = current.width();
    const int h = current.height();

    int foregroundCount = 0;
    int foregroundMatched = 0;

    for (int y = 0; y < h; ++y)
    {
        const QRgb *curLine = reinterpret_cast<const QRgb *>(current.constScanLine(y));
        const QRgb *refLine = reinterpret_cast<const QRgb *>(reference.constScanLine(y));

        for (int x = 0; x < w; ++x)
        {
            const int idx = y * w + x;

            // Pixel di sfondo: lo ignoriamo del tutto, qualunque
            // cosa ci sia dietro il pannello semitrasparente.
            if (idx >= mask.size() || !mask[idx])
                continue;

            ++foregroundCount;

            const QRgb c = curLine[x];
            const QRgb r = refLine[x];

            const int dr = qAbs(qRed(c) - qRed(r));
            const int dg = qAbs(qGreen(c) - qGreen(r));
            const int db = qAbs(qBlue(c) - qBlue(r));

            if (dr <= tolerance && dg <= tolerance && db <= tolerance)
                ++foregroundMatched;
        }
    }

    // Nessun pixel di foreground rilevato nella reference: caso
    // anomalo (reference tutta scura/uniforme), ripieghiamo sul
    // confronto pixel-perfect completo come comportamento sicuro.
    if (foregroundCount == 0)
    {
        for (int y = 0; y < h; ++y)
        {
            const QRgb *curLine = reinterpret_cast<const QRgb *>(current.constScanLine(y));
            const QRgb *refLine = reinterpret_cast<const QRgb *>(reference.constScanLine(y));

            for (int x = 0; x < w; ++x)
            {
                const QRgb c = curLine[x];
                const QRgb r = refLine[x];

                if (qAbs(qRed(c) - qRed(r)) > tolerance ||
                    qAbs(qGreen(c) - qGreen(r)) > tolerance ||
                    qAbs(qBlue(c) - qBlue(r)) > tolerance)
                {
                    return false;
                }
            }
        }
        return true;
    }

    const double ratio = static_cast<double>(foregroundMatched) / foregroundCount;
    return ratio >= MATCH_RATIO;
}

