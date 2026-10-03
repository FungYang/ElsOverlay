#pragma once

#include <QImage>
#include <QRect>
#include <QVector>

class CustomSearcherMultiFinder
{
public:

    struct Template
    {
        int id = -1;
        QImage image;

        int pixelTolerance = 7;
        double matchThreshold = 97.5;
    };

    struct Result
    {
        int templateId = -1;
        QRect rect;
        double score = 0.0;
    };

    CustomSearcherMultiFinder() = default;

    void setTemplates(
        const QVector<Template> &templates
        );

    void clear();

    QVector<Result> find(
        const QImage &frame
        ) const;

    int templateCount() const;

private:

    struct FastSample
    {
        int x = 0;
        int y = 0;
        QRgb pixel = 0;
    };

    struct PreparedTemplate
    {
        int id = -1;

        QImage image;

        int width = 0;
        int height = 0;
        int stride = 0;

        int pixelTolerance = 7;
        double matchThreshold = 97.5;

        QVector<FastSample> fastSamples;
    };

    static PreparedTemplate prepareTemplate(
        const Template &source
        );

    static bool fastCheck(
        const QImage &frame,
        const PreparedTemplate &templ,
        int offsetX,
        int offsetY
        );

    static double fullCompare(
        const QImage &frame,
        const PreparedTemplate &templ,
        int offsetX,
        int offsetY
        );

    static bool pixelsMatch(
        QRgb a,
        QRgb b,
        int tolerance
        );

    static QVector<FastSample> buildFastSamples(
        const QImage &image
        );

private:

    QVector<PreparedTemplate> m_templates;

    static constexpr int FAST_SAMPLE_COUNT = 16;
    static constexpr int BORDER = 1;
};