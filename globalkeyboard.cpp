#include "globalkeyboard.h"

#include <QDebug>
#include <QSettings>
#include <QCoreApplication>


HHOOK GlobalKeyboard::hook =
    nullptr;


GlobalKeyboard *GlobalKeyboard::instance =
    nullptr;


// =========================================================
// CONSTRUCTOR
// =========================================================

GlobalKeyboard::GlobalKeyboard(
    QObject *parent
    )
    : QObject(parent)
{
    instance = this;


    // =====================================================
    // SETTINGS
    // =====================================================

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


    // =====================================================
    // QUEUE TIMER
    // =====================================================
    //
    // L'hook NON chiama Qt direttamente.
    //
    // Il timer svuota la coda dal normale thread Qt.
    // =====================================================

    m_queueTimer.setInterval(1);

    connect(
        &m_queueTimer,
        &QTimer::timeout,
        this,
        &GlobalKeyboard::processPendingEvents
        );


    m_queueTimer.start();


    // =====================================================
    // GLOBAL LOW LEVEL KEYBOARD HOOK
    // =====================================================

    hook =
        SetWindowsHookEx(
            WH_KEYBOARD_LL,
            keyboardProc,
            GetModuleHandle(nullptr),
            0
            );


    if(!hook)
    {
        qDebug()
        << "GlobalKeyboard:"
        << "SetWindowsHookEx FAILED:"
        << GetLastError();
    }
    else
    {
        qDebug()
        << "GlobalKeyboard:"
        << "keyboard hook attivo";
    }
}


// =========================================================
// DESTRUCTOR
// =========================================================

GlobalKeyboard::~GlobalKeyboard()
{
    // Prima fermiamo il timer Qt.

    m_queueTimer.stop();


    // Poi rimuoviamo l'hook Windows.

    if(hook)
    {
        UnhookWindowsHookEx(
            hook
            );

        hook = nullptr;
    }


    instance = nullptr;
}


// =========================================================
// WINDOWS KEYBOARD HOOK
// =========================================================
//
// IMPORTANTISSIMO:
//
// Qui NON facciamo:
// - QMetaObject::invokeMethod
// - emit
// - qDebug
// - QSettings
// - elaborazioni
// - accesso allo stato Qt
//
// Facciamo solamente:
//
// 1. leggiamo i dati del tasto
// 2. inseriamo l'evento nella ring buffer
// 3. CallNextHookEx IMMEDIATO
//
// Questo riduce al minimo il tempo trascorso nell'hook.
// =========================================================

LRESULT CALLBACK GlobalKeyboard::keyboardProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if(nCode != HC_ACTION)
    {
        return CallNextHookEx(
            hook,
            nCode,
            wParam,
            lParam
            );
    }


    GlobalKeyboard *kb =
        instance;


    if(!kb)
    {
        return CallNextHookEx(
            hook,
            nCode,
            wParam,
            lParam
            );
    }


    const KBDLLHOOKSTRUCT *key =
        reinterpret_cast<const KBDLLHOOKSTRUCT *>(
            lParam
            );


    if(!key)
    {
        return CallNextHookEx(
            hook,
            nCode,
            wParam,
            lParam
            );
    }


    const bool isKeyDown =
        wParam == WM_KEYDOWN ||
        wParam == WM_SYSKEYDOWN;


    const bool isKeyUp =
        wParam == WM_KEYUP ||
        wParam == WM_SYSKEYUP;


    if(isKeyDown || isKeyUp)
    {
        KeyEvent event;

        event.vkCode =
            static_cast<int>(
                key->vkCode
                );

        event.scanCode =
            static_cast<int>(
                key->scanCode
                );

        event.extended =
            (key->flags & LLKHF_EXTENDED) != 0;

        event.keyDown =
            isKeyDown;


        // =================================================
        // INSERIMENTO RAPIDO NELLA CODA
        // =================================================
        //
        // Se la coda fosse piena, non blocchiamo Windows.
        // CallNextHookEx viene comunque eseguito.
        // =================================================

        kb->enqueueEvent(
            event
            );
    }


    // =====================================================
    // FONDAMENTALE
    // =====================================================
    //
    // Il gioco riceve SEMPRE il normale evento Windows.
    // =====================================================

    return CallNextHookEx(
        hook,
        nCode,
        wParam,
        lParam
        );
}


// =========================================================
// ENQUEUE
// =========================================================
//
// Producer:
//     Windows keyboard hook
//
// Consumer:
//     Qt timer
//
// Single producer / single consumer.
// =========================================================

bool GlobalKeyboard::enqueueEvent(
    const KeyEvent &event
    )
{
    const uint32_t write =
        m_writeIndex.load(
            std::memory_order_relaxed
            );


    const uint32_t next =
        (write + 1) % QUEUE_SIZE;


    const uint32_t read =
        m_readIndex.load(
            std::memory_order_acquire
            );


    // =====================================================
    // CODA PIENA
    // =====================================================

    if(next == read)
    {
        // Non aspettiamo.
        //
        // Non blocchiamo mai il gioco.
        //
        // In condizioni normali con 8192 elementi
        // questo non dovrebbe praticamente mai accadere.

        return false;
    }


    m_queue[write] =
        event;


    m_writeIndex.store(
        next,
        std::memory_order_release
        );


    return true;
}


// =========================================================
// DEQUEUE
// =========================================================

bool GlobalKeyboard::dequeueEvent(
    KeyEvent &event
    )
{
    const uint32_t read =
        m_readIndex.load(
            std::memory_order_relaxed
            );


    const uint32_t write =
        m_writeIndex.load(
            std::memory_order_acquire
            );


    if(read == write)
        return false;


    event =
        m_queue[read];


    const uint32_t next =
        (read + 1) % QUEUE_SIZE;


    m_readIndex.store(
        next,
        std::memory_order_release
        );


    return true;
}


// =========================================================
// PROCESS PENDING EVENTS
// =========================================================
//
// Questa funzione gira nel normale thread Qt.
//
// Qui possiamo usare tranquillamente:
// - emit
// - stato paused
// - configurazione tasti
// - altri oggetti Qt
// =========================================================

void GlobalKeyboard::processPendingEvents()
{
    KeyEvent event;


    // Svuota completamente la coda disponibile.

    while(dequeueEvent(event))
    {
        if(event.keyDown)
        {
            processKeyDown(
                event
                );
        }
        else
        {
            processKeyUp(
                event
                );
        }
    }
}


// =========================================================
// KEY DOWN
// =========================================================

void GlobalKeyboard::processKeyDown(
    const KeyEvent &event
    )
{
    const int vkCode =
        event.vkCode;


    const int scanCode =
        event.scanCode;


    const bool extended =
        event.extended;


    // =====================================================
    // PAUSA
    // =====================================================

    if(scanCode == m_pauseScanCode &&
        extended == m_pauseExtended)
    {
        paused =
            !paused;


        emit pauseChanged(
            paused
            );


        return;
    }


    // =====================================================
    // ENTER
    // =====================================================

    if(scanCode == 0x1C &&
        !extended)
    {
        if(paused)
        {
            paused = false;

            emit pauseChanged(
                false
                );
        }
        else
        {
            emit confirmPressed();
        }


        return;
    }


    // =====================================================
    // PAUSA ATTIVA
    // =====================================================

    if(paused)
        return;


    // =====================================================
    // TASTO 7
    // =====================================================

    if(vkCode == '7')
    {
        emit transcendenceResetPressed();
    }


    // =====================================================
    // TASTO GENERICO
    // =====================================================

    emit keyPressed(
        vkCode
        );


    // =====================================================
    // CTRL SINISTRO
    // =====================================================

    if(scanCode == 0x1D &&
        !extended)
    {
        emit ctrlPressed();
    }


    // =====================================================
    // RESET GLOBALE
    // =====================================================

    if(scanCode == m_resetScanCode &&
        extended == m_resetExtended)
    {
        emit resetPressed();
    }
}


// =========================================================
// KEY UP
// =========================================================

void GlobalKeyboard::processKeyUp(
    const KeyEvent &event
    )
{
    if(paused)
        return;


    emit keyReleased(
        event.vkCode
        );
}


// =========================================================
// PAUSE KEY
// =========================================================

void GlobalKeyboard::setPauseKey(
    int scanCode,
    bool extended
    )
{
    m_pauseScanCode =
        scanCode;

    m_pauseExtended =
        extended;
}


// =========================================================
// RESET KEY
// =========================================================

void GlobalKeyboard::setResetKey(
    int scanCode,
    bool extended
    )
{
    m_resetScanCode =
        scanCode;

    m_resetExtended =
        extended;
}