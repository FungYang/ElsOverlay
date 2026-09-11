#include "buffvisiondetectionworker.h"

BuffVisionDetectionWorker::BuffVisionDetectionWorker(QObject *parent)
    : QObject(parent)
{
    m_detector = new BuffVisionDetector(this); // figlio: si sposta col worker
}

void BuffVisionDetectionWorker::loadModel(QString modelPath)
{
    bool ok = m_detector->loadModel(modelPath);
    emit modelLoaded(ok);
}

void BuffVisionDetectionWorker::detectFrame(int cropId, QImage frame)
{
    if (frame.isNull())
    {
        emit numberDetected(cropId, 1000);
        return;
    }

    const QPixmap pixmap = QPixmap::fromImage(frame);

    const int number =
        (cropId == 1)
            ? m_detector->detectCrop1(pixmap)
            : m_detector->detectCrop2(pixmap);

    emit numberDetected(cropId, number);
}