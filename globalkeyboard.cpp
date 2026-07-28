#include "globalkeyboard.h"


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

                // ESC funziona sempre
                if (key->vkCode == VK_ESCAPE)
                {
                    emit instance->escPressed();
                    return CallNextHookEx(hook, nCode, wParam, lParam);
                }
                if(key->vkCode == VK_RETURN)
                {
                    emit instance->confirmPressed();
                }

                // Se siamo in pausa, ignoriamo tutti gli altri input
                if (instance->paused)
                {
                    return CallNextHookEx(hook, nCode, wParam, lParam);
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