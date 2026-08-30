#ifndef GLOBALKEYBOARD_H
#define GLOBALKEYBOARD_H

#include <QObject>
#include <QTimer>

#include <windows.h>

#include <array>
#include <atomic>
#include <cstdint>


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
    void keyReleased(int key);

    void pauseChanged(bool paused);
    void confirmPressed();
    void transcendenceResetPressed();

public slots:
    void setPauseKey(int scanCode, bool extended);
    void setResetKey(int scanCode, bool extended);

private:

    // =====================================================
    // KEY EVENT
    // =====================================================

    struct KeyEvent
    {
        int vkCode = 0;
        int scanCode = 0;

        bool extended = false;
        bool keyDown = false;
    };


    // =====================================================
    // RING BUFFER
    // =====================================================
    //
    // Il callback Windows produce eventi.
    // Il thread Qt li consuma.
    //
    // Non vengono effettuate allocazioni durante l'hook.
    // =====================================================

    static constexpr uint32_t QUEUE_SIZE = 8192;

    std::array<KeyEvent, QUEUE_SIZE> m_queue;

    std::atomic<uint32_t> m_writeIndex{0};
    std::atomic<uint32_t> m_readIndex{0};


    // =====================================================
    // TIMER
    // =====================================================

    QTimer m_queueTimer;


    // =====================================================
    // STATO
    // =====================================================

    bool paused = false;

    int m_pauseScanCode = 0x01;
    bool m_pauseExtended = false;

    int m_resetScanCode = 0x1D;
    bool m_resetExtended = true;


    // =====================================================
    // GLOBAL HOOK
    // =====================================================

    static HHOOK hook;

    static GlobalKeyboard *instance;


    // =====================================================
    // HOOK WINDOWS
    // =====================================================

    static LRESULT CALLBACK keyboardProc(
        int nCode,
        WPARAM wParam,
        LPARAM lParam
        );


    // =====================================================
    // QUEUE
    // =====================================================

    bool enqueueEvent(
        const KeyEvent &event
        );

    bool dequeueEvent(
        KeyEvent &event
        );


    // =====================================================
    // PROCESSING QT
    // =====================================================

    void processPendingEvents();

    void processKeyDown(
        const KeyEvent &event
        );

    void processKeyUp(
        const KeyEvent &event
        );
};

#endif