#pragma once

#include <QObject>
#include <QRect>
#include <QPixmap>

class BuffVisionCapture : public QObject
{
    Q_OBJECT

public:
    explicit BuffVisionCapture(QObject *parent = nullptr);

    bool loadSettings();

    // Mantenuto per compatibilita' con il codice esistente
    // (BuffVisionManager lo chiama nel costruttore).
    // ScreenCapture si inizializza da se' al primo utilizzo,
    // quindi qui non c'e' piu' nulla da fare esplicitamente.
    bool initDuplication();

    QPixmap captureCrop1();
    QPixmap captureCrop2();

    void setCropAreas(QRect crop1, QRect crop2);

    void saveReference1();
    void saveReference2();

private:
    QRect cropRect1;
    QRect cropRect2;
};