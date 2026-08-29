#include "globalkeyboard.h"

#include <QDebug>
#include <QSettings>
#include <QCoreApplication>
#include <QElapsedTimer>



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


    // qDebug()
    //     << "PAUSA:"
    //     << "ScanCode =" << Qt::hex << m_pauseScanCode
    //     << "Extended =" << m_pauseExtended;


    // qDebug()
    //     << "RESET:"
    //     << "ScanCode =" << Qt::hex << m_resetScanCode
    //     << "Extended =" << m_resetExtended;


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
    static QElapsedTimer timer;
    static bool timerStarted = false;

    static qint64 previousEntryNs = -1;
    static quint64 eventCounter = 0;

    if(!timerStarted)
    {
        timer.start();
        timerStarted = true;
    }

    // ---------------------------------------------------------
    // L'evento non è valido per noi:
    // passiamo immediatamente al prossimo hook.
    // ---------------------------------------------------------

    if(nCode != HC_ACTION)
    {
        return CallNextHookEx(
            hook,
            nCode,
            wParam,
            lParam
            );
    }

    // ---------------------------------------------------------
    // INGRESSO REALE NELL'HOOK
    // ---------------------------------------------------------

    const qint64 entryNs =
        timer.nsecsElapsed();

    ++eventCounter;

    // Intervallo dall'evento precedente entrato nell'hook.
    const qint64 intervalNs =
        previousEntryNs >= 0
            ? entryNs - previousEntryNs
            : 0;

    previousEntryNs = entryNs;

    // ---------------------------------------------------------
    // DATI TASTO
    // ---------------------------------------------------------

    const KBDLLHOOKSTRUCT *key =
        reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

    if(!key)
    {
        return CallNextHookEx(
            hook,
            nCode,
            wParam,
            lParam
            );
    }

    const int vkCode =
        static_cast<int>(key->vkCode);

    const int scanCode =
        static_cast<int>(key->scanCode);

    const bool extended =
        (key->flags & LLKHF_EXTENDED) != 0;

    // ---------------------------------------------------------
    // PROCESSING DELL'HOOK
    // ---------------------------------------------------------

    if(wParam == WM_KEYDOWN)
    {
        // =====================================================
        // PAUSA
        // =====================================================

        if(scanCode == instance->m_pauseScanCode &&
            extended == instance->m_pauseExtended)
        {
            instance->paused =
                !instance->paused;

            emit instance->pauseChanged(
                instance->paused
                );

            goto call_next;
        }

        // =====================================================
        // ENTER
        // =====================================================

        if(scanCode == 0x1C &&
            !extended)
        {
            if(instance->paused)
            {
                instance->paused = false;

                emit instance->pauseChanged(
                    false
                    );
            }
            else
            {
                emit instance->confirmPressed();
            }

            goto call_next;
        }

        // =====================================================
        // PAUSA ATTIVA
        // =====================================================

        if(instance->paused)
        {
            goto call_next;
        }

        // =====================================================
        // TASTO 7
        // =====================================================

        if(vkCode == '7')
        {
            emit instance->transcendenceResetPressed();
        }

        // =====================================================
        // TASTO GENERICO
        // =====================================================

        emit instance->keyPressed(
            vkCode
            );

        // =====================================================
        // CTRL SINISTRO
        // =====================================================

        if(scanCode == 0x1D &&
            !extended)
        {
            emit instance->ctrlPressed();
        }

        // =====================================================
        // RESET GLOBALE
        // =====================================================

        if(scanCode == instance->m_resetScanCode &&
            extended == instance->m_resetExtended)
        {
            emit instance->resetPressed();
        }
    }

    // ---------------------------------------------------------
    // KEY UP
    // ---------------------------------------------------------

    else if(wParam == WM_KEYUP)
    {
        if(instance->paused)
        {
            goto call_next;
        }

        emit instance->keyReleased(
            vkCode
            );
    }

    // ---------------------------------------------------------
    // CALL NEXT HOOK
    // ---------------------------------------------------------

call_next:

    const qint64 beforeCallNextNs =
        timer.nsecsElapsed();

    const LRESULT result =
        CallNextHookEx(
            hook,
            nCode,
            wParam,
            lParam
            );

    const qint64 afterCallNextNs =
        timer.nsecsElapsed();

    // ---------------------------------------------------------
    // MISURAZIONI
    // ---------------------------------------------------------

    const qint64 processingNs =
        beforeCallNextNs - entryNs;

    const qint64 callNextNs =
        afterCallNextNs - beforeCallNextNs;

    const qint64 totalNs =
        afterCallNextNs - entryNs;

    // ---------------------------------------------------------
    // LOG
    //
    // PROCESS  = tempo impiegato dal nostro codice
    // CALLNEXT = tempo passato dentro CallNextHookEx
    // TOTAL    = ingresso hook -> ritorno da CallNextHookEx
    //
    // ---------------------------------------------------------

    // qDebug()
    //     << "HOOK:"
    //     << "#"
    //     << eventCounter
    //     << "VK ="
    //     << QString("0x%1").arg(vkCode, 2, 16, QChar('0')).toUpper()
    //     << "SCAN ="
    //     << QString("0x%1").arg(scanCode, 2, 16, QChar('0')).toUpper()
    //     << "EXT ="
    //     << extended
    //     << "MSG ="
    //     << QString("0x%1").arg(
    //                           static_cast<quint64>(wParam),
    //                           2,
    //                           16,
    //                           QChar('0')
    //                           ).toUpper()
    //     << "INTERVAL ="
    //     << intervalNs / 1000000.0
    //     << "ms"
    //     << "PROCESS ="
    //     << processingNs / 1000000.0
    //     << "ms"
    //     << "CALLNEXT ="
    //     << callNextNs / 1000000.0
    //     << "ms"
    //     << "TOTAL ="
    //     << totalNs / 1000000.0
    //     << "ms";

    return result;
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


    // qDebug()
    //     << "PAUSE KEY CAMBIATO:"
    //     << "ScanCode =" << Qt::hex << scanCode
    //     << "Extended =" << extended;
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


    // qDebug()
    //     << "RESET KEY CAMBIATO:"
    //     << "ScanCode =" << Qt::hex << scanCode
    //     << "Extended =" << extended;
}