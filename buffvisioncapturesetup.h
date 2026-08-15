#ifndef BUFFVISIONCAPTURESETUP_H
#define BUFFVISIONCAPTURESETUP_H

#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QTimer>



class BuffVisionCaptureSetup : public QWidget
{
    Q_OBJECT

public:

    explicit BuffVisionCaptureSetup(
        QWidget *parent = nullptr
        );

    void loadSettings();

    void saveSettings();

    void resetSetup();

    QRect getCropRect1() const;

    QRect getCropRect2() const;

    void showFeedback(
        const QString &text
        );

    void setCaptureMode(
        bool active
        );



protected:

    void paintEvent(
        QPaintEvent *event
        ) override;


    void mousePressEvent(
        QMouseEvent *event
        ) override;


    void mouseMoveEvent(
        QMouseEvent *event
        ) override;


    void mouseReleaseEvent(
        QMouseEvent *event
        ) override;


    void keyPressEvent(
        QKeyEvent *event
        ) override;



private:

    QString feedbackText;

    QTimer feedbackTimer;


    QRect captureRect;

    QRect cropRect1;

    QRect cropRect2;


    bool configured = false;

    bool captureMode = false;


    enum DragMode
    {
        None,
        Capture,
        Crop1,
        Crop2
    };


    DragMode dragMode = None;


    QPoint lastMousePosition;



    void moveCapture(
        QPoint delta
        );


    void moveCrop(
        QRect &crop,
        QPoint delta
        );


    void keepInsideCapture(
        QRect &crop
        );

};

#endif