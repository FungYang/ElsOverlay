#include "customsearchermultifinder.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>


void CustomSearcherMultiFinder::setTemplates(
    const QVector<Template> &templates
    )
{
    m_templates.clear();
    m_templates.reserve(templates.size());

    for (const Template &templ : templates)
    {
        if (templ.id < 0)
            continue;

        if (templ.image.isNull())
            continue;

        if (templ.image.width() <= BORDER * 2 ||
            templ.image.height() <= BORDER * 2)
        {
            continue;
        }

        m_templates.append(
            prepareTemplate(templ)
            );
    }
}


void CustomSearcherMultiFinder::clear()
{
    m_templates.clear();
}


int CustomSearcherMultiFinder::templateCount() const
{
    return m_templates.size();
}


// ============================================================
// FIND
// ============================================================
//
// IMPORTANTE:
//
// La ROI/frame viene attraversata UNA SOLA VOLTA.
//
// Non facciamo:
//
//     template A -> tutta ROI
//     template B -> tutta ROI
//     template C -> tutta ROI
//
// Facciamo:
//
//     posizione ROI
//         -> A
//         -> B
//         -> C
//         -> ...
//
// Quando un template viene trovato viene marcato come trovato
// e non viene più controllato nelle posizioni successive dello
// stesso frame.
//

QVector<CustomSearcherMultiFinder::Result>
CustomSearcherMultiFinder::find(
    const QImage &frame
    ) const
{
    QVector<Result> results;

    if (frame.isNull())
        return results;

    if (m_templates.isEmpty())
        return results;

    const int frameWidth =
        frame.width();

    const int frameHeight =
        frame.height();

    if (frameWidth <= 0 ||
        frameHeight <= 0)
    {
        return results;
    }

    const int templateCount =
        m_templates.size();

    results.reserve(templateCount);

    // Un template trovato durante questo frame
    // non deve essere controllato nuovamente.
    QVector<bool> found(
        templateCount,
        false
        );

    int remainingTemplates =
        templateCount;

    //
    // Troviamo la dimensione massima del template.
    //
    // Non possiamo però usare solamente questa dimensione
    // per limitare il ciclo, perché i template possono avere
    // dimensioni diverse.
    //

    for (int y = 0;
         y < frameHeight;
         ++y)
    {
        for (int x = 0;
             x < frameWidth;
             ++x)
        {
            if (remainingTemplates == 0)
                return results;

            //
            // Testiamo tutti i template ancora attivi
            // sulla stessa posizione.
            //

            for (int i = 0;
                 i < templateCount;
                 ++i)
            {
                if (found[i])
                    continue;

                const PreparedTemplate &templ =
                    m_templates[i];

                //
                // Il template deve stare completamente
                // dentro il frame.
                //
                if (x + templ.width > frameWidth ||
                    y + templ.height > frameHeight)
                {
                    continue;
                }

                //
                // FAST CHECK
                //
                if (!fastCheck(
                        frame,
                        templ,
                        x,
                        y))
                {
                    continue;
                }

                //
                // FULL CHECK
                //
                const double score =
                    fullCompare(
                        frame,
                        templ,
                        x,
                        y
                        );

                if (score < templ.matchThreshold)
                    continue;

                //
                // TEMPLATE TROVATO.
                //
                found[i] = true;

                --remainingTemplates;

                Result result;

                result.templateId =
                    templ.id;

                result.rect =
                    QRect(
                        x,
                        y,
                        templ.width,
                        templ.height
                        );

                result.score =
                    score;

                results.append(result);

                //
                // IMPORTANTE:
                //
                // Non controlliamo altri template alla
                // stessa posizione? No: questo dipende
                // dall'architettura desiderata.
                //
                // Se due template identici/sovrapposti sono
                // configurati, vogliamo poterli trovare
                // entrambi.
                //
                // Per questo NON facciamo break qui.
            }
        }
    }

    return results;
}


// ============================================================
// PREPARE TEMPLATE
// ============================================================

CustomSearcherMultiFinder::PreparedTemplate
CustomSearcherMultiFinder::prepareTemplate(
    const Template &source
    )
{
    PreparedTemplate result;

    result.id =
        source.id;

    //
    // Manteniamo una copia del template già preparato.
    //
    // setTemplates() viene chiamato quando cambia il set
    // dei template, NON per ogni frame.
    //
    result.image =
        source.image.convertToFormat(
            QImage::Format_ARGB32
            );

    result.width =
        result.image.width();

    result.height =
        result.image.height();

    result.stride =
        result.image.bytesPerLine();

    result.pixelTolerance =
        source.pixelTolerance;

    result.matchThreshold =
        source.matchThreshold;

    result.fastSamples =
        buildFastSamples(
            result.image
            );

    return result;
}


// ============================================================
// FAST CHECK
// ============================================================
//
// Usiamo 16 punti distribuiti nel template.
//
// Lo scopo NON è decidere definitivamente se è un match.
// Serve solamente a eliminare velocemente le posizioni
// chiaramente sbagliate.
//

bool CustomSearcherMultiFinder::fastCheck(
    const QImage &frame,
    const PreparedTemplate &templ,
    int offsetX,
    int offsetY
    )
{
    if(templ.fastSamples.isEmpty())
    {
        return true;
    }


    const int tolerance =
        templ.pixelTolerance;


    const uchar *frameBits =
        frame.constBits();


    const int frameStride =
        frame.bytesPerLine();


    for(const FastSample &sample :
         templ.fastSamples)
    {
        const QRgb *frameLine =
            reinterpret_cast<const QRgb *>(
                frameBits +
                (offsetY + sample.y) *
                    frameStride
                );


        const QRgb framePixel =
            frameLine[
                offsetX + sample.x
        ];


        if(!pixelsMatch(
                framePixel,
                sample.pixel,
                tolerance))
        {
            return false;
        }
    }


    return true;
}


// ============================================================
// FULL COMPARE
// ============================================================

double CustomSearcherMultiFinder::fullCompare(
    const QImage &frame,
    const PreparedTemplate &templ,
    int offsetX,
    int offsetY
    )
{
    const int width =
        templ.width;

    const int height =
        templ.height;

    const int innerWidth =
        width - BORDER * 2;

    const int innerHeight =
        height - BORDER * 2;

    if (innerWidth <= 0 ||
        innerHeight <= 0)
    {
        return 0.0;
    }

    const int totalPixels =
        innerWidth *
        innerHeight;

    //
    // Con soglia 97.5% possiamo tollerare al massimo
    // il 2.5% dei pixel differenti.
    //

    const double allowedDifferentRatio =
        1.0 -
        (templ.matchThreshold / 100.0);

    const int maxDifferentPixels =
        static_cast<int>(
            std::floor(
                allowedDifferentRatio *
                totalPixels
                )
            );

    int differentPixels = 0;

    const uchar *frameBits =
        frame.constBits();

    const uchar *templateBits =
        templ.image.constBits();

    const int frameStride =
        frame.bytesPerLine();

    const int templateStride =
        templ.stride;

    //
    // ARGB32 su Qt:
    //
    // 4 byte per pixel.
    //

    for (int y = BORDER;
         y < height - BORDER;
         ++y)
    {
        const QRgb *frameLine =
            reinterpret_cast<const QRgb *>(
                frameBits +
                (offsetY + y) *
                    frameStride
                );

        const QRgb *templateLine =
            reinterpret_cast<const QRgb *>(
                templateBits +
                y *
                    templateStride
                );

        for (int x = BORDER;
             x < width - BORDER;
             ++x)
        {
            const QRgb framePixel =
                frameLine[offsetX + x];

            const QRgb templatePixel =
                templateLine[x];

            if (!pixelsMatch(
                    framePixel,
                    templatePixel,
                    templ.pixelTolerance))
            {
                ++differentPixels;

                //
                // Early exit:
                //
                // sappiamo già che questo candidato
                // non può raggiungere la soglia.
                //
                if (differentPixels >
                    maxDifferentPixels)
                {
                    const double score =
                        100.0 *
                        (
                            1.0 -
                            static_cast<double>(
                                differentPixels
                                ) /
                                static_cast<double>(
                                    totalPixels
                                    )
                            );

                    return score;
                }
            }
        }
    }

    const double score =
        100.0 *
        (
            1.0 -
            static_cast<double>(
                differentPixels
                ) /
                static_cast<double>(
                    totalPixels
                    )
            );

    return score;
}


// ============================================================
// PIXEL MATCH
// ============================================================

bool CustomSearcherMultiFinder::pixelsMatch(
    QRgb a,
    QRgb b,
    int tolerance
    )
{
    const int dr =
        qAbs(
            qRed(a) -
            qRed(b)
            );

    const int dg =
        qAbs(
            qGreen(a) -
            qGreen(b)
            );

    const int db =
        qAbs(
            qBlue(a) -
            qBlue(b)
            );

    return
        dr <= tolerance &&
        dg <= tolerance &&
        db <= tolerance;
}


// ============================================================
// FAST SAMPLES
// ============================================================

QVector<
    CustomSearcherMultiFinder::FastSample
    >
CustomSearcherMultiFinder::buildFastSamples(
    const QImage &image
    )
{
    QVector<FastSample> samples;

    if (image.isNull())
        return samples;

    const int width =
        image.width();

    const int height =
        image.height();

    if (width <= BORDER * 2 ||
        height <= BORDER * 2)
    {
        return samples;
    }

    //
    // 4x4 = 16 punti.
    //
    // Li distribuiamo nell'area interna del template,
    // evitando il bordo.
    //

    constexpr int GRID = 4;

    samples.reserve(
        FAST_SAMPLE_COUNT
        );

    for (int gy = 0;
         gy < GRID;
         ++gy)
    {
        const int y =
            BORDER +
            (
                gy *
                (height - BORDER * 2 - 1)
                ) /
                (GRID - 1);

        for (int gx = 0;
             gx < GRID;
             ++gx)
        {
            const int x =
                BORDER +
                (
                    gx *
                    (width - BORDER * 2 - 1)
                    ) /
                    (GRID - 1);

            FastSample sample;

            sample.x =
                x;

            sample.y =
                y;

            sample.pixel =
                image.pixel(
                    x,
                    y
                    );

            samples.append(sample);
        }
    }

    return samples;
}