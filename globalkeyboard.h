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
    void keyReleased(
        int key
        );
    void pauseChanged(bool paused);
    void confirmPressed();
    void transcendenceResetPressed();

public slots:
    void setPauseKey(int vkCode);


private:
    bool paused = false;
    static HHOOK hook;
    static GlobalKeyboard *instance;
    int m_pauseKey = VK_SPACE;


    static LRESULT CALLBACK keyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
        );
};

#endif