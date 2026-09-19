#include "atmazoneworker.h"

AtmaZoneWorker::AtmaZoneWorker(QObject *parent)
    : QObject(parent)
{
}

void AtmaZoneWorker::loadReferences(QString imagesDir)
{
    bool ok = true;

    for (int i = 0; i < RED_COUNT; ++i)
    {
        QImage img;
        if (!img.load(imagesDir + QString("ref%1.png").arg(i + 1)))
        {
            ok = false;
            continue;
        }
        m_redReferences[i] = img.convertToFormat(QImage::Format_ARGB32);
    }

    QImage blue;
    if (!blue.load(imagesDir + "invariant.png"))
        ok = false;
    else
        m_blueReference = blue.convertToFormat(QImage::Format_ARGB32);

    emit referencesLoaded(ok);
}

void AtmaZoneWorker::compareRedFrame(int index, QImage frame)
{
    if (index < 0 || index >= RED_COUNT)
        return;

    const bool isMatch =
        matches(frame, m_redReferences[index], TOLERANCE);

#ifdef QT_DEBUG
    emit redDebugFrame(index, frame, isMatch);
#endif
    emit redCompared(index, isMatch);
}

void AtmaZoneWorker::compareBlueFrame(QImage frame)
{
    const bool isMatch =
        matches(frame, m_blueReference, TOLERANCE);

#ifdef QT_DEBUG
    emit blueDebugFrame(frame, isMatch);
#endif
    emit blueCompared(isMatch);
}

bool AtmaZoneWorker::matches(const QImage &current, const QImage &reference, int tolerance)
{
    if (current.isNull() || reference.isNull())
        return false;

    if (current.size() != reference.size())
        return false;

    const int w = current.width();
    const int h = current.height();

    for (int y = 0; y < h; ++y)
    {
        const QRgb *curLine = reinterpret_cast<const QRgb *>(current.constScanLine(y));
        const QRgb *refLine = reinterpret_cast<const QRgb *>(reference.constScanLine(y));

        for (int x = 0; x < w; ++x)
        {
            const QRgb c = curLine[x];
            const QRgb r = refLine[x];

            const int dr = qAbs(qRed(c) - qRed(r));
            const int dg = qAbs(qGreen(c) - qGreen(r));
            const int db = qAbs(qBlue(c) - qBlue(r));

            if (dr > tolerance || dg > tolerance || db > tolerance)
                return false;
        }
    }

    return true;
}

