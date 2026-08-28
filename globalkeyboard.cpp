#include "globalkeyboard.h"

#include <QDebug>
#include <QSettings>
#include <QCoreApplication>


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

    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    m_pauseKey = settings.value("Keys/PauseKey", VK_SPACE).toInt();


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

            if(key->vkCode == instance->m_pauseKey)
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

void GlobalKeyboard::setPauseKey(int vkCode)
{
    m_pauseKey = vkCode;
}