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
        QCoreApplication::applicationDirPath() +
            "/ElsOverlay.ini",
        QSettings::IniFormat
        );


    m_pauseScanCode =
        settings.value(
                    "Keys/PauseScanCode",
                    0x01
                    ).toInt();


    m_pauseExtended =
        settings.value(
                    "Keys/PauseExtended",
                    false
                    ).toBool();


    m_resetScanCode =
        settings.value(
                    "Keys/ResetScanCode",
                    0x1D
                    ).toInt();


    m_resetExtended =
        settings.value(
                    "Keys/ResetExtended",
                    true
                    ).toBool();


    qDebug()
        << "PAUSA:"
        << "ScanCode =" << Qt::hex << m_pauseScanCode
        << "Extended =" << m_pauseExtended;


    qDebug()
        << "RESET:"
        << "ScanCode =" << Qt::hex << m_resetScanCode
        << "Extended =" << m_resetExtended;


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


        const int scanCode =
            static_cast<int>(
                key->scanCode
                );


        const bool extended =
            (key->flags & LLKHF_EXTENDED) != 0;


        // ==================================================
        // KEY DOWN
        // ==================================================

        if(wParam == WM_KEYDOWN ||
            wParam == WM_SYSKEYDOWN)
        {
            // ==============================================
            // PAUSA
            // ==============================================

            if(scanCode == instance->m_pauseScanCode &&
                extended == instance->m_pauseExtended)
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
            // PAUSA ATTIVA
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
            // RESET GLOBALE
            // ==============================================

            if(scanCode == instance->m_resetScanCode &&
                extended == instance->m_resetExtended)
            {
                qDebug()
                << "RESET PREMUTO:"
                << "ScanCode =" << Qt::hex << scanCode
                << "Extended =" << extended;


                emit instance->resetPressed();


                return CallNextHookEx(
                    hook,
                    nCode,
                    wParam,
                    lParam
                    );
            }


            // ==============================================
            // TRASCENDENCE RESET
            // ==============================================

            if(key->vkCode == '7')
            {
                emit instance->transcendenceResetPressed();
            }


            // ==============================================
            // KEY PRESSED
            // ==============================================

            emit instance->keyPressed(
                static_cast<int>(
                    key->vkCode
                    )
                );


            // ==============================================
            // CTRL SINISTRO
            // ==============================================

            if(key->vkCode == VK_LCONTROL &&
                !(key->flags & LLKHF_EXTENDED))
            {
                emit instance->ctrlPressed();
            }
        }


        // ==================================================
        // KEY UP
        // ==================================================

        else if(wParam == WM_KEYUP ||
                 wParam == WM_SYSKEYUP)
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


void GlobalKeyboard::setPauseKey(
    int scanCode,
    bool extended
    )
{
    m_pauseScanCode =
        scanCode;

    m_pauseExtended =
        extended;


    qDebug()
        << "PAUSE KEY CAMBIATO:"
        << "ScanCode =" << Qt::hex << scanCode
        << "Extended =" << extended;
}


void GlobalKeyboard::setResetKey(
    int scanCode,
    bool extended
    )
{
    m_resetScanCode =
        scanCode;

    m_resetExtended =
        extended;


    qDebug()
        << "RESET KEY CAMBIATO:"
        << "ScanCode =" << Qt::hex << scanCode
        << "Extended =" << extended;
}