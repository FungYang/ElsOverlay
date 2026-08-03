#ifndef GLOBALKEYBOARD_H
#define GLOBALKEYBOARD_H

#include <QObject>
#include <windows.h>

class GlobalKeyboard : public QObject
{
    Q_OBJECT

public:
    explicit GlobalKeyboard(QObject *parent = nullptr);
    ~GlobalKeyboard();


signals:
    void ctrlPressed();
    void escPressed();
    void resetPressed();
    void keyPressed(int key);
    void pauseChanged(bool paused);
    void confirmPressed();


private:
    bool paused = false;
    bool waitingForExit = false;
    static HHOOK hook;
    static GlobalKeyboard *instance;


    static LRESULT CALLBACK keyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
        );
};

#endif