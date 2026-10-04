#ifndef CUSTOMSEARCHERWORKER_H
#define CUSTOMSEARCHERWORKER_H

#include <QObject>
#include <QImage>
#include <QMutex>
#include <QVector>

#include "customsearchermultifinder.h"


class CustomSearcherWorker : public QObject
{
    Q_OBJECT

public:

    explicit CustomSearcherWorker(
        QObject *parent = nullptr
        );

    ~CustomSearcherWorker() override = default;


    // ========================================================
    // THREAD-SAFE INPUT
    // ========================================================

    void submitFrameZone1(
        const QImage &frame
        );

    void submitFrameZone2(
        const QImage &frame
        );


    // ========================================================
    // TEMPLATES
    // ========================================================

    void setTemplatesZone1(
        const QVector<CustomSearcherMultiFinder::Template> &templates
        );

    void setTemplatesZone2(
        const QVector<CustomSearcherMultiFinder::Template> &templates
        );


    void clearTemplates();


signals:

    // ========================================================
    // RESULTS
    // ========================================================

    void resultsReadyZone1(
        const QVector<CustomSearcherMultiFinder::Result> &results
        );

    void resultsReadyZone2(
        const QVector<CustomSearcherMultiFinder::Result> &results
        );


private slots:

    // ========================================================
    // PROCESSING
    // ========================================================

    void processLatestZone1();

    void processLatestZone2();


    // ========================================================
    // APPLY TEMPLATES
    // ========================================================

    void applyTemplatesZone1(
        const QVector<CustomSearcherMultiFinder::Template> &templates
        );

    void applyTemplatesZone2(
        const QVector<CustomSearcherMultiFinder::Template> &templates
        );


    void applyClearTemplates();


private:

    // ========================================================
    // SHARED FRAME STATE
    // ========================================================

    QMutex m_mutex;


    // ========================================================
    // ZONE 1
    // ========================================================

    QImage m_pendingFrameZone1;

    bool m_processScheduledZone1 = false;


    // ========================================================
    // ZONE 2
    // ========================================================

    QImage m_pendingFrameZone2;

    bool m_processScheduledZone2 = false;


    // ========================================================
    // FINDERS
    // ========================================================

    CustomSearcherMultiFinder m_finderZone1;

    CustomSearcherMultiFinder m_finderZone2;
};

#endif