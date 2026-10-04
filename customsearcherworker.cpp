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
// SUBMIT FRAME - ZONE 1
// ============================================================
//
// LATEST FRAME WINS.
//
// Se il worker sta ancora elaborando un frame Zone1,
// il nuovo frame sostituisce quello precedente in attesa.
//

void CustomSearcherWorker::submitFrameZone1(
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
        // LATEST FRAME WINS - ZONE 1
        // ====================================================

        m_pendingFrameZone1 =
            frame;


        if(!m_processScheduledZone1)
        {
            m_processScheduledZone1 = true;
            schedule = true;
        }
    }


    if(!schedule)
    {
        return;
    }


    QMetaObject::invokeMethod(
        this,
        "processLatestZone1",
        Qt::QueuedConnection
        );
}


// ============================================================
// SUBMIT FRAME - ZONE 2
// ============================================================

void CustomSearcherWorker::submitFrameZone2(
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
        // LATEST FRAME WINS - ZONE 2
        // ====================================================

        m_pendingFrameZone2 =
            frame;


        if(!m_processScheduledZone2)
        {
            m_processScheduledZone2 = true;
            schedule = true;
        }
    }


    if(!schedule)
    {
        return;
    }


    QMetaObject::invokeMethod(
        this,
        "processLatestZone2",
        Qt::QueuedConnection
        );
}


// ============================================================
// PROCESS LATEST - ZONE 1
// ============================================================

void CustomSearcherWorker::processLatestZone1()
{
    QImage frame;


    {
        QMutexLocker locker(
            &m_mutex
            );


        frame =
            m_pendingFrameZone1;


        m_pendingFrameZone1 =
            QImage();


        if(frame.isNull())
        {
            m_processScheduledZone1 = false;
            return;
        }
    }


    // ========================================================
    // HEAVY CPU WORK
    //
    // Questo codice viene eseguito nel thread del Worker.
    //
    // SOLO template appartenenti alla Zone1.
    // ========================================================

    const QVector<
        CustomSearcherMultiFinder::Result
        > results =
        m_finderZone1.find(
            frame
            );


    if(!results.isEmpty())
    {
        emit resultsReadyZone1(
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


        if(!m_pendingFrameZone1.isNull())
        {
            processAgain = true;
        }
        else
        {
            m_processScheduledZone1 = false;
        }
    }


    if(processAgain)
    {
        QMetaObject::invokeMethod(
            this,
            "processLatestZone1",
            Qt::QueuedConnection
            );
    }
}


// ============================================================
// PROCESS LATEST - ZONE 2
// ============================================================

void CustomSearcherWorker::processLatestZone2()
{
    QImage frame;


    {
        QMutexLocker locker(
            &m_mutex
            );


        frame =
            m_pendingFrameZone2;


        m_pendingFrameZone2 =
            QImage();


        if(frame.isNull())
        {
            m_processScheduledZone2 = false;
            return;
        }
    }


    // ========================================================
    // HEAVY CPU WORK
    //
    // Questo codice viene eseguito nel thread del Worker.
    //
    // SOLO template appartenenti alla Zone2.
    // ========================================================

    const QVector<
        CustomSearcherMultiFinder::Result
        > results =
        m_finderZone2.find(
            frame
            );


    if(!results.isEmpty())
    {
        emit resultsReadyZone2(
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


        if(!m_pendingFrameZone2.isNull())
        {
            processAgain = true;
        }
        else
        {
            m_processScheduledZone2 = false;
        }
    }


    if(processAgain)
    {
        QMetaObject::invokeMethod(
            this,
            "processLatestZone2",
            Qt::QueuedConnection
            );
    }
}


// ============================================================
// SET TEMPLATES - ZONE 1
// ============================================================

void CustomSearcherWorker::setTemplatesZone1(
    const QVector<
        CustomSearcherMultiFinder::Template
        > &templates
    )
{
    QMetaObject::invokeMethod(
        this,
        "applyTemplatesZone1",
        Qt::QueuedConnection,
        Q_ARG(
            QVector<CustomSearcherMultiFinder::Template>,
            templates
            )
        );
}


// ============================================================
// SET TEMPLATES - ZONE 2
// ============================================================

void CustomSearcherWorker::setTemplatesZone2(
    const QVector<
        CustomSearcherMultiFinder::Template
        > &templates
    )
{
    QMetaObject::invokeMethod(
        this,
        "applyTemplatesZone2",
        Qt::QueuedConnection,
        Q_ARG(
            QVector<CustomSearcherMultiFinder::Template>,
            templates
            )
        );
}


// ============================================================
// APPLY TEMPLATES - ZONE 1
// ============================================================

void CustomSearcherWorker::applyTemplatesZone1(
    const QVector<
        CustomSearcherMultiFinder::Template
        > &templates
    )
{
    m_finderZone1.setTemplates(
        templates
        );
}


// ============================================================
// APPLY TEMPLATES - ZONE 2
// ============================================================

void CustomSearcherWorker::applyTemplatesZone2(
    const QVector<
        CustomSearcherMultiFinder::Template
        > &templates
    )
{
    m_finderZone2.setTemplates(
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
    m_finderZone1.clear();

    m_finderZone2.clear();
}