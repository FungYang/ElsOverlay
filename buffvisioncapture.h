#ifndef BUFFVISIONCAPTURE_H
#define BUFFVISIONCAPTURE_H

#include <QObject>
#include <QPixmap>
#include <QRect>



class BuffVisionCapture : public QObject
{
    Q_OBJECT

public:

    explicit BuffVisionCapture(
        QObject *parent = nullptr
        );


    bool loadSettings();


    QPixmap captureCrop1();

    QPixmap captureCrop2();
    QPixmap& getCrop1Ref1();

    QPixmap& getCrop1Ref2() ;

    QPixmap& getCrop2Ref1() ;

    QPixmap& getCrop2Ref2() ;

    void saveReference1();

    void saveReference2();

    void setCropAreas(
        QRect crop1,
        QRect crop2
        );



private:


    QRect cropRect1;

    QRect cropRect2;



    QPixmap grabScreen(const QRect &rect);
    const QPixmap crop1Ref1 ;
    const QPixmap crop1Ref2;
    const QPixmap crop2Ref1;
    const QPixmap crop2Ref2;



};

#endif