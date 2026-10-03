#include "customsearcherworker.h"

#include <QMetaObject>
#include <QMutexLocker>


CustomSearcherWorker::CustomSearcherWorker(
    QObject *parent
    )
    : QObject(parent)
{
}


// ============================================================
// SUBMIT FRAME
// ============================================================

void CustomSearcherWorker::submitFrame(
    const QImage &frame
    )
{
    if(frame.isNull())
    {
        return;
    }



    bool schedule = false;


    {
        QMutexLocker locker(
            &m_mutex
            );


        // ====================================================
        // LATEST FRAME WINS
        // ====================================================

        m_pendingFrame =
            frame;


        if(!m_processScheduled)
        {
            m_processScheduled = true;
            schedule = true;
        }
    }


    if(!schedule)
    {
        return;
    }


    QMetaObject::invokeMethod(
        this,
        "processLatest",
        Qt::QueuedConnection
        );
}


// ============================================================
// PROCESS LATEST
// ============================================================

void CustomSearcherWorker::processLatest()
{
    QImage frame;


    {
        QMutexLocker locker(
            &m_mutex
            );


        frame =
            m_pendingFrame;


        m_pendingFrame =
            QImage();


        if(frame.isNull())
        {
            m_processScheduled = false;
            return;
        }
    }


    // ========================================================
    // HEAVY CPU WORK
    //
    // QUESTO CODICE VIENE ESEGUITO NEL THREAD DEL WORKER
    // ========================================================

    const QVector<
        CustomSearcherMultiFinder::Result
        > results =
        m_finder.find(
            frame
            );


    if(!results.isEmpty())
    {
        emit resultsReady(
            results
            );
    }


    // ========================================================
    // CHECK FOR NEWER FRAME
    // ========================================================

    bool processAgain = false;


    {
        QMutexLocker locker(
            &m_mutex
            );


        if(!m_pendingFrame.isNull())
        {
            processAgain = true;
        }
        else
        {
            m_processScheduled = false;
        }
    }


    if(processAgain)
    {
        QMetaObject::invokeMethod(
            this,
            "processLatest",
            Qt::QueuedConnection
            );
    }
}


// ============================================================
// SET TEMPLATES
// ============================================================

void CustomSearcherWorker::setTemplates(
    const QVector<
        CustomSearcherMultiFinder::Template
        > &templates
    )
{
    QMetaObject::invokeMethod(
        this,
        "applyTemplates",
        Qt::QueuedConnection,
        Q_ARG(
            QVector<CustomSearcherMultiFinder::Template>,
            templates
            )
        );
}


// ============================================================
// APPLY TEMPLATES
// ============================================================

void CustomSearcherWorker::applyTemplates(
    const QVector<
        CustomSearcherMultiFinder::Template
        > &templates
    )
{
    m_finder.setTemplates(
        templates
        );
}


// ============================================================
// CLEAR TEMPLATES
// ============================================================

void CustomSearcherWorker::clearTemplates()
{
    QMetaObject::invokeMethod(
        this,
        "applyClearTemplates",
        Qt::QueuedConnection
        );
}


// ============================================================
// APPLY CLEAR
// ============================================================

void CustomSearcherWorker::applyClearTemplates()
{
    m_finder.clear();
}