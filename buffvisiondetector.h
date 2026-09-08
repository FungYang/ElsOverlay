#ifndef BUFFVISIONDETECTOR_H
#define BUFFVISIONDETECTOR_H

#include <QObject>
#include <QPixmap>

#include "digitdetector.h"


    class BuffVisionDetector : public QObject
{
    Q_OBJECT

public:

    explicit BuffVisionDetector(
        QObject *parent = nullptr
        );


    /*
     * Carica il modello YOLO.
     */
    bool loadModel(
        const QString &modelPath
        );


    /*
     * Rileva il numero del Crop 1.
     *
     * Ritorna:
     *   numero rilevato
     *   1000 = Unknown
     */
    int detectCrop1(
        const QPixmap &current
        );


    /*
     * Rileva il numero del Crop 2.
     *
     * Ritorna:
     *   numero rilevato
     *   1000 = Unknown
     */
    int detectCrop2(
        const QPixmap &current
        );


    /*
     * Indica se il modello è stato caricato.
     */
    bool isLoaded() const;


private:

    DigitDetector digitDetector;


    bool loaded = false;

};

#endif
