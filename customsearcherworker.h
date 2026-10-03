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

    void submitFrame(
        const QImage &frame
        );


    void setTemplates(
        const QVector<CustomSearcherMultiFinder::Template> &templates
        );


    void clearTemplates();


signals:

    void resultsReady(
        const QVector<CustomSearcherMultiFinder::Result> &results
        );


private slots:

    void processLatest();


    void applyTemplates(
        const QVector<CustomSearcherMultiFinder::Template> &templates
        );


    void applyClearTemplates();


private:

    QMutex m_mutex;

    QImage m_pendingFrame;

    bool m_processScheduled = false;

    CustomSearcherMultiFinder m_finder;
};

#endif