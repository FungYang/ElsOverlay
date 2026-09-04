#include "keyedit.h"

#include <QKeyEvent>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif


KeyEdit::KeyEdit(
    QWidget *parent
    )
    : QLineEdit(parent)
{
    setReadOnly(true);


    setPlaceholderText(
        "Premi un tasto..."
        );


    setFocusPolicy(
        Qt::StrongFocus
        );
}


// ============================================================
// GET KEY CODE
// ============================================================

int KeyEdit::keyCode() const
{
    return m_keyCode;
}


// ============================================================
// SET KEY CODE
// ============================================================

void KeyEdit::setKeyCode(
    int key
    )
{
    m_keyCode =
        key;


    if(key == 0)
    {
        clear();

        return;
    }


    setText(
        keyName(key)
        );
}


// ============================================================
// KEY PRESS
// ============================================================

void KeyEdit::keyPressEvent(
    QKeyEvent *event
    )
{
    if(!event)
    {
        return;
    }


#ifdef Q_OS_WIN

    int vk =
        qtKeyToVirtualKey(event);


    qDebug()
        << "Qt key:"
        << event->key()
        << "nativeVirtualKey:"
        << event->nativeVirtualKey()
        << "nativeScanCode:"
        << event->nativeScanCode()
        << "mapped VK:"
        << vk;


    if(vk != 0)
    {
        setKeyCode(
            vk
            );


        event->accept();


        return;
    }

#endif


    event->accept();
}


// ============================================================
// KEY RELEASE
// ============================================================

void KeyEdit::keyReleaseEvent(
    QKeyEvent *event
    )
{
    if(!event)
    {
        return;
    }


    event->accept();
}


// ============================================================
// KEY NAME
// ============================================================

QString KeyEdit::keyName(
    int key
    ) const
{
#ifdef Q_OS_WIN

    switch(key)
    {

        // ----------------------------------------------------
        // CONTROLLO
        // ----------------------------------------------------

    case VK_BACK:
        return "BACKSPACE";

    case VK_TAB:
        return "TAB";

    case VK_CLEAR:
        return "CLEAR";

    case VK_RETURN:
        return "ENTER";

    case VK_PAUSE:
        return "PAUSE";

    case VK_CAPITAL:
        return "CAPS LOCK";

    case VK_ESCAPE:
        return "ESC";

    case VK_SPACE:
        return "SPACE";


        // ----------------------------------------------------
        // NAVIGAZIONE
        // ----------------------------------------------------

    case VK_PRIOR:
        return "PAGE UP";

    case VK_NEXT:
        return "PAGE DOWN";

    case VK_END:
        return "END";

    case VK_HOME:
        return "HOME";

    case VK_LEFT:
        return "LEFT";

    case VK_UP:
        return "UP";

    case VK_RIGHT:
        return "RIGHT";

    case VK_DOWN:
        return "DOWN";

    case VK_SELECT:
        return "SELECT";

    case VK_PRINT:
        return "PRINT";

    case VK_EXECUTE:
        return "EXECUTE";

    case VK_SNAPSHOT:
        return "PRINT SCREEN";

    case VK_INSERT:
        return "INSERT";

    case VK_DELETE:
        return "DELETE";

    case VK_HELP:
        return "HELP";


        // ----------------------------------------------------
        // WINDOWS
        // ----------------------------------------------------

    case VK_LWIN:
        return "LEFT WIN";

    case VK_RWIN:
        return "RIGHT WIN";

    case VK_APPS:
        return "MENU";


        // ----------------------------------------------------
        // MODIFICATORI
        // ----------------------------------------------------

    case VK_LSHIFT:
        return "LEFT SHIFT";

    case VK_RSHIFT:
        return "RIGHT SHIFT";

    case VK_LCONTROL:
        return "LEFT CTRL";

    case VK_RCONTROL:
        return "RIGHT CTRL";

    case VK_LMENU:
        return "LEFT ALT";

    case VK_RMENU:
        return "RIGHT ALT";


        // ----------------------------------------------------
        // LOCK
        // ----------------------------------------------------

    case VK_NUMLOCK:
        return "NUM LOCK";

    case VK_SCROLL:
        return "SCROLL LOCK";


        // ----------------------------------------------------
        // NUMPAD
        // ----------------------------------------------------

    case VK_NUMPAD0:
        return "NUMPAD 0";

    case VK_NUMPAD1:
        return "NUMPAD 1";

    case VK_NUMPAD2:
        return "NUMPAD 2";

    case VK_NUMPAD3:
        return "NUMPAD 3";

    case VK_NUMPAD4:
        return "NUMPAD 4";

    case VK_NUMPAD5:
        return "NUMPAD 5";

    case VK_NUMPAD6:
        return "NUMPAD 6";

    case VK_NUMPAD7:
        return "NUMPAD 7";

    case VK_NUMPAD8:
        return "NUMPAD 8";

    case VK_NUMPAD9:
        return "NUMPAD 9";

    case VK_MULTIPLY:
        return "NUMPAD *";

    case VK_ADD:
        return "NUMPAD +";

    case VK_SEPARATOR:
        return "NUMPAD SEPARATOR";

    case VK_SUBTRACT:
        return "NUMPAD -";

    case VK_DECIMAL:
        return "NUMPAD .";

    case VK_DIVIDE:
        return "NUMPAD /";


        // ----------------------------------------------------
        // F1 - F24
        // ----------------------------------------------------

    case VK_F1:
        return "F1";

    case VK_F2:
        return "F2";

    case VK_F3:
        return "F3";

    case VK_F4:
        return "F4";

    case VK_F5:
        return "F5";

    case VK_F6:
        return "F6";

    case VK_F7:
        return "F7";

    case VK_F8:
        return "F8";

    case VK_F9:
        return "F9";

    case VK_F10:
        return "F10";

    case VK_F11:
        return "F11";

    case VK_F12:
        return "F12";

    case VK_F13:
        return "F13";

    case VK_F14:
        return "F14";

    case VK_F15:
        return "F15";

    case VK_F16:
        return "F16";

    case VK_F17:
        return "F17";

    case VK_F18:
        return "F18";

    case VK_F19:
        return "F19";

    case VK_F20:
        return "F20";

    case VK_F21:
        return "F21";

    case VK_F22:
        return "F22";

    case VK_F23:
        return "F23";

    case VK_F24:
        return "F24";


        // ----------------------------------------------------
        // OEM
        // ----------------------------------------------------

    case VK_OEM_1:
        return ";";

    case VK_OEM_PLUS:
        return "=";

    case VK_OEM_COMMA:
        return ",";

    case VK_OEM_MINUS:
        return "-";

    case VK_OEM_PERIOD:
        return ".";

    case VK_OEM_2:
        return "/";

    case VK_OEM_3:
        return "`";

    case VK_OEM_4:
        return "[";

    case VK_OEM_5:
        return "\\";

    case VK_OEM_6:
        return "]";

    case VK_OEM_7:
        return "'";

    case VK_OEM_8:
        return "OEM 8";

    case VK_OEM_102:
        return "OEM 102";


        // ----------------------------------------------------
        // BROWSER
        // ----------------------------------------------------

    case VK_BROWSER_BACK:
        return "BROWSER BACK";

    case VK_BROWSER_FORWARD:
        return "BROWSER FORWARD";

    case VK_BROWSER_REFRESH:
        return "BROWSER REFRESH";

    case VK_BROWSER_STOP:
        return "BROWSER STOP";

    case VK_BROWSER_SEARCH:
        return "BROWSER SEARCH";

    case VK_BROWSER_FAVORITES:
        return "BROWSER FAVORITES";

    case VK_BROWSER_HOME:
        return "BROWSER HOME";


        // ----------------------------------------------------
        // VOLUME / MEDIA
        // ----------------------------------------------------

    case VK_VOLUME_MUTE:
        return "VOLUME MUTE";

    case VK_VOLUME_DOWN:
        return "VOLUME DOWN";

    case VK_VOLUME_UP:
        return "VOLUME UP";

    case VK_MEDIA_NEXT_TRACK:
        return "MEDIA NEXT";

    case VK_MEDIA_PREV_TRACK:
        return "MEDIA PREVIOUS";

    case VK_MEDIA_STOP:
        return "MEDIA STOP";

    case VK_MEDIA_PLAY_PAUSE:
        return "MEDIA PLAY/PAUSE";


        // ----------------------------------------------------
        // LAUNCH
        // ----------------------------------------------------

    case VK_LAUNCH_MAIL:
        return "LAUNCH MAIL";

    case VK_LAUNCH_MEDIA_SELECT:
        return "LAUNCH MEDIA";

    case VK_LAUNCH_APP1:
        return "LAUNCH APP 1";

    case VK_LAUNCH_APP2:
        return "LAUNCH APP 2";

    }


    // --------------------------------------------------------
    // LETTERE A-Z
    // --------------------------------------------------------

    if(
        key >= 'A' &&
        key <= 'Z'
        )
    {
        return QString(
            QChar(key)
            );
    }


    // --------------------------------------------------------
    // NUMERI 0-9
    // --------------------------------------------------------

    if(
        key >= '0' &&
        key <= '9'
        )
    {
        return QString(
            QChar(key)
            );
    }


    // --------------------------------------------------------
    // FALLBACK
    // --------------------------------------------------------

    return QString(
               "VK 0x%1"
               )
        .arg(
            key,
            2,
            16,
            QChar('0')
            )
        .toUpper();

#else

    Q_UNUSED(key);

    return "KEY";

#endif
}


// ============================================================
// QT KEY -> WINDOWS VIRTUAL KEY
// ============================================================

#ifdef Q_OS_WIN

int KeyEdit::qtKeyToVirtualKey(
    QKeyEvent *event
    ) const
{
    if(!event)
    {
        return 0;
    }


    const int qtKey =
        event->key();


    const Qt::KeyboardModifiers modifiers =
        event->modifiers();


    const quint32 scanCode =
        static_cast<quint32>(
            event->nativeScanCode()
            );


    const quint32 nativeVirtualKey =
        static_cast<quint32>(
            event->nativeVirtualKey()
            );


    // ========================================================
    // SHIFT SINISTRO / DESTRO
    // ========================================================

    if(qtKey == Qt::Key_Shift)
    {
        /*
         * Shift destro:
         *
         * Scan code:
         * 0x36
         */

        if(scanCode == 0x36)
        {
            return VK_RSHIFT;
        }


        /*
         * Shift sinistro:
         *
         * Scan code:
         * 0x2A
         */

        return VK_LSHIFT;
    }


    // ========================================================
    // CTRL SINISTRO / DESTRO
    // ========================================================

    if(qtKey == Qt::Key_Control)
    {
        /*
         * CTRL DESTRO
         *
         * Qt sul nostro sistema restituisce:
         *
         * nativeScanCode = 57373
         *
         * 57373 = 0xE01D
         */

        if(
            scanCode == 0xE01D ||
            scanCode == 57373
            )
        {
            return VK_RCONTROL;
        }


        /*
         * CTRL DESTRO:
         *
         * fallback nel caso Qt/Windows
         * fornisca direttamente VK_RCONTROL.
         */

        if(nativeVirtualKey == VK_RCONTROL)
        {
            return VK_RCONTROL;
        }


        /*
         * CTRL SINISTRO
         *
         * Scan code:
         * 0x1D
         */

        return VK_LCONTROL;
    }


    // ========================================================
    // ALT SINISTRO / DESTRO
    // ========================================================

    if(qtKey == Qt::Key_Alt)
    {
        /*
         * ALT DESTRO
         *
         * Scan code esteso:
         * 0xE038
         */

        if(
            scanCode == 0xE038 ||
            scanCode == 0x138
            )
        {
            return VK_RMENU;
        }


        /*
         * Fallback se Windows
         * fornisce direttamente VK_RMENU.
         */

        if(nativeVirtualKey == VK_RMENU)
        {
            return VK_RMENU;
        }


        /*
         * ALT SINISTRO
         */

        return VK_LMENU;
    }


    // ========================================================
    // WIN SINISTRO / DESTRO
    // ========================================================

    if(qtKey == Qt::Key_Meta)
    {
        /*
         * WIN DESTRO
         *
         * Scan code:
         * 0xE05C
         */

        if(
            scanCode == 0xE05C ||
            scanCode == 0x15C
            )
        {
            return VK_RWIN;
        }


        /*
         * Fallback native VK.
         */

        if(nativeVirtualKey == VK_RWIN)
        {
            return VK_RWIN;
        }


        /*
         * WIN SINISTRO
         */

        return VK_LWIN;
    }


    // ========================================================
    // TASTI SPECIALI
    // ========================================================

    switch(qtKey)
    {

    case Qt::Key_Backspace:
        return VK_BACK;

    case Qt::Key_Tab:
        return VK_TAB;

    case Qt::Key_Clear:
        return VK_CLEAR;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        return VK_RETURN;

    case Qt::Key_Pause:
        return VK_PAUSE;

    case Qt::Key_CapsLock:
        return VK_CAPITAL;

    case Qt::Key_NumLock:
        return VK_NUMLOCK;

    case Qt::Key_ScrollLock:
        return VK_SCROLL;

    case Qt::Key_Escape:
        return VK_ESCAPE;

    case Qt::Key_Space:
        return VK_SPACE;

    case Qt::Key_PageUp:
        return VK_PRIOR;

    case Qt::Key_PageDown:
        return VK_NEXT;

    case Qt::Key_End:
        return VK_END;

    case Qt::Key_Home:
        return VK_HOME;

    case Qt::Key_Left:
        return VK_LEFT;

    case Qt::Key_Up:
        return VK_UP;

    case Qt::Key_Right:
        return VK_RIGHT;

    case Qt::Key_Down:
        return VK_DOWN;

    case Qt::Key_Insert:
        return VK_INSERT;

    case Qt::Key_Delete:
        return VK_DELETE;

    case Qt::Key_Print:
        return VK_SNAPSHOT;

    case Qt::Key_SysReq:
        return VK_SNAPSHOT;

    case Qt::Key_Menu:
        return VK_APPS;

    case Qt::Key_Select:
        return VK_SELECT;

    case Qt::Key_Execute:
        return VK_EXECUTE;

    case Qt::Key_Help:
        return VK_HELP;

    }


    // ========================================================
    // F1 - F24
    // ========================================================

    if(
        qtKey >= Qt::Key_F1 &&
        qtKey <= Qt::Key_F24
        )
    {
        return VK_F1 +
               (
                   qtKey -
                   Qt::Key_F1
                   );
    }


    // ========================================================
    // NUMPAD
    // ========================================================

    if(
        modifiers &
        Qt::KeypadModifier
        )
    {

        switch(qtKey)
        {

        case Qt::Key_0:
            return VK_NUMPAD0;

        case Qt::Key_1:
            return VK_NUMPAD1;

        case Qt::Key_2:
            return VK_NUMPAD2;

        case Qt::Key_3:
            return VK_NUMPAD3;

        case Qt::Key_4:
            return VK_NUMPAD4;

        case Qt::Key_5:
            return VK_NUMPAD5;

        case Qt::Key_6:
            return VK_NUMPAD6;

        case Qt::Key_7:
            return VK_NUMPAD7;

        case Qt::Key_8:
            return VK_NUMPAD8;

        case Qt::Key_9:
            return VK_NUMPAD9;

        case Qt::Key_Asterisk:
            return VK_MULTIPLY;

        case Qt::Key_Plus:
            return VK_ADD;

        case Qt::Key_Minus:
            return VK_SUBTRACT;

        case Qt::Key_Slash:
            return VK_DIVIDE;

        case Qt::Key_Period:
            return VK_DECIMAL;

        case Qt::Key_Comma:
            return VK_SEPARATOR;

        }

    }


    // ========================================================
    // NUMERI NORMALI
    // ========================================================

    if(
        qtKey >= Qt::Key_0 &&
        qtKey <= Qt::Key_9
        )
    {

        if(
            !(modifiers &
              Qt::KeypadModifier)
            )
        {
            return '0' +
                   (
                       qtKey -
                       Qt::Key_0
                       );
        }

    }


    // ========================================================
    // LETTERE
    // ========================================================

    if(
        qtKey >= Qt::Key_A &&
        qtKey <= Qt::Key_Z
        )
    {
        return 'A' +
               (
                   qtKey -
                   Qt::Key_A
                   );
    }


    // ========================================================
    // TASTI OEM
    // ========================================================

    if(
        nativeVirtualKey >= VK_OEM_1 &&
        nativeVirtualKey <= VK_OEM_102
        )
    {
        return static_cast<int>(
            nativeVirtualKey
            );
    }


    // ========================================================
    // BROWSER / MULTIMEDIA
    // ========================================================

    switch(nativeVirtualKey)
    {

    case VK_BROWSER_BACK:
    case VK_BROWSER_FORWARD:
    case VK_BROWSER_REFRESH:
    case VK_BROWSER_STOP:
    case VK_BROWSER_SEARCH:
    case VK_BROWSER_FAVORITES:
    case VK_BROWSER_HOME:

    case VK_VOLUME_MUTE:
    case VK_VOLUME_DOWN:
    case VK_VOLUME_UP:

    case VK_MEDIA_NEXT_TRACK:
    case VK_MEDIA_PREV_TRACK:
    case VK_MEDIA_STOP:
    case VK_MEDIA_PLAY_PAUSE:

    case VK_LAUNCH_MAIL:
    case VK_LAUNCH_MEDIA_SELECT:
    case VK_LAUNCH_APP1:
    case VK_LAUNCH_APP2:

        return static_cast<int>(
            nativeVirtualKey
            );

    }


    // ========================================================
    // FALLBACK NATIVE VIRTUAL KEY
    // ========================================================

    if(nativeVirtualKey != 0)
    {
        return static_cast<int>(
            nativeVirtualKey
            );
    }


    // ========================================================
    // FALLBACK ASCII
    // ========================================================

    QString text =
        event->text();


    if(!text.isEmpty())
    {

        QChar c =
            text.at(0);


        ushort unicode =
            c.unicode();


        if(
            unicode >= 32 &&
            unicode < 127
            )
        {
            return static_cast<int>(
                unicode
                );
        }

    }


    return 0;
}

#endif
