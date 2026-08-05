#include "globalkeyboard.h"
#include <QDebug>


HHOOK GlobalKeyboard::hook = nullptr;

GlobalKeyboard* GlobalKeyboard::instance = nullptr;



GlobalKeyboard::GlobalKeyboard(QObject *parent)
    : QObject(parent)
{

    instance = this;


    hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        keyboardProc,
        GetModuleHandle(nullptr),
        0
        );

}



GlobalKeyboard::~GlobalKeyboard()
{

    if(hook)
        UnhookWindowsHookEx(hook);

}



LRESULT CALLBACK GlobalKeyboard::keyboardProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{

    if(nCode == HC_ACTION)
    {

        KBDLLHOOKSTRUCT *key =
            reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);


        if (wParam == WM_KEYDOWN)
        {
            if (instance)
            {
                // SPACE attiva/disattiva la pausa dell'overlay
                if (key->vkCode == VK_SPACE)
                {
                    instance->paused = !instance->paused;
                    emit instance->pauseChanged(instance->paused);

                    return CallNextHookEx(
                        hook,
                        nCode,
                        wParam,
                        lParam
                        );
                }

                // ESC prepara la chiusura ma non blocca gli input
                if (key->vkCode == VK_ESCAPE)
                {
                    instance->waitingForExit = true;
                }


                // Controllo P dopo ESC
                else if(instance->waitingForExit)
                {
                    if(key->vkCode == 'P')
                    {
                        instance->waitingForExit = false;

                        emit instance->escPressed();

                        return CallNextHookEx(
                            hook,
                            nCode,
                            wParam,
                            lParam
                            );
                    }

                    // qualsiasi altro tasto annulla la chiusura
                    instance->waitingForExit = false;
                }
                if(key->vkCode == VK_RETURN)
                {
                    // qDebug() << "ENTER DOWN";
                    emit instance->confirmPressed();
                }

                // Se siamo in pausa, ignoriamo tutti gli altri input
                if (instance->paused)
                {
                    return CallNextHookEx(hook, nCode, wParam, lParam);
                }
                if(key->vkCode == '7')
                {
                    emit instance->transcendenceResetPressed();
                }

                emit instance->keyPressed(key->vkCode);

                if (key->vkCode == VK_LCONTROL)
                {
                    emit instance->ctrlPressed();
                }

                if (key->vkCode == VK_RCONTROL)
                {
                    emit instance->resetPressed();
                }
            }
        }

    }


    return CallNextHookEx(
        hook,
        nCode,
        wParam,
        lParam
        );
}