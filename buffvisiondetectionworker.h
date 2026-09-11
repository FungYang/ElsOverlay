#pragma once

#include <QObject>
#include <QImage>
#include "buffvisiondetector.h"

class BuffVisionDetectionWorker : public QObject
{
    Q_OBJECT

public:
    explicit BuffVisionDetectionWorker(QObject *parent = nullptr);

public slots:
    void loadModel(QString modelPath);

    // cropId: 1 o 2, per sapere quale numero è stato rilevato
    void detectFrame(int cropId, QImage frame);

signals:
    void modelLoaded(bool ok);
    void numberDetected(int cropId, int number);

private:
    BuffVisionDetector *m_detector = nullptr;
};