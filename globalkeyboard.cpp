#include "globalkeyboard.h"

#include <QDebug>


HHOOK GlobalKeyboard::hook =
    nullptr;


GlobalKeyboard *GlobalKeyboard::instance =
    nullptr;



GlobalKeyboard::GlobalKeyboard(
    QObject *parent
    )
    : QObject(parent)
{
    instance =
        this;


    hook =
        SetWindowsHookEx(
            WH_KEYBOARD_LL,
            keyboardProc,
            GetModuleHandle(nullptr),
            0
            );
}



GlobalKeyboard::~GlobalKeyboard()
{
    if(hook)
    {
        UnhookWindowsHookEx(
            hook
            );

        hook =
            nullptr;
    }


    instance =
        nullptr;
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
            reinterpret_cast<KBDLLHOOKSTRUCT *>(
                lParam
                );


        if(!instance)
        {
            return CallNextHookEx(
                hook,
                nCode,
                wParam,
                lParam
                );
        }



        // ==================================================
        // KEY DOWN
        // ==================================================

        if(wParam == WM_KEYDOWN)
        {
            // ==============================================
            // SPACE
            // ==============================================

            if(key->vkCode == VK_SPACE)
            {
                instance->paused =
                    !instance->paused;


                emit instance->pauseChanged(
                    instance->paused
                    );


                return CallNextHookEx(
                    hook,
                    nCode,
                    wParam,
                    lParam
                    );
            }



            // ==============================================
            // ENTER
            // ==============================================

            if(key->vkCode == VK_RETURN)
            {
                if(instance->paused)
                {
                    instance->paused =
                        false;


                    emit instance->pauseChanged(
                        false
                        );
                }
                else
                {
                    emit instance->confirmPressed();
                }


                return CallNextHookEx(
                    hook,
                    nCode,
                    wParam,
                    lParam
                    );
            }



            // ==============================================
            // PAUSA
            // ==============================================

            if(instance->paused)
            {
                return CallNextHookEx(
                    hook,
                    nCode,
                    wParam,
                    lParam
                    );
            }



            // ==============================================
            // TASTI SPECIALI
            // ==============================================

            if(key->vkCode == '7')
            {
                emit instance->transcendenceResetPressed();
            }


            emit instance->keyPressed(
                static_cast<int>(
                    key->vkCode
                    )
                );



            // ==============================================
            // CTRL SINISTRO
            // ==============================================

            if(key->vkCode == VK_LCONTROL)
            {
                emit instance->ctrlPressed();
            }



            // ==============================================
            // CTRL DESTRO
            // ==============================================

            if(key->vkCode == VK_RCONTROL)
            {
                emit instance->resetPressed();
            }
        }



        // ==================================================
        // KEY UP
        // ==================================================

        else if(wParam == WM_KEYUP)
        {
            if(instance->paused)
            {
                return CallNextHookEx(
                    hook,
                    nCode,
                    wParam,
                    lParam
                    );
            }


            emit instance->keyReleased(
                static_cast<int>(
                    key->vkCode
                    )
                );
        }
    }


    return CallNextHookEx(
        hook,
        nCode,
        wParam,
        lParam
        );
}