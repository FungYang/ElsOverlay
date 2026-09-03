#include "keyedit.h"

#include <QCoreApplication>


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
    m_keyCode = key;

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


    if(vk != 0)
    {
        setKeyCode(vk);

        event->accept();

        return;
    }

#endif


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
    case VK_BACK:
        return "BACKSPACE";

    case VK_TAB:
        return "TAB";

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

    case VK_INSERT:
        return "INSERT";

    case VK_DELETE:
        return "DELETE";

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

    case VK_SUBTRACT:
        return "NUMPAD -";

    case VK_DECIMAL:
        return "NUMPAD .";

    case VK_DIVIDE:
        return "NUMPAD /";

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

    case VK_LWIN:
        return "LEFT WIN";

    case VK_RWIN:
        return "RIGHT WIN";

    case VK_APPS:
        return "MENU";
    }


    // F1 - F24

    if(key >= VK_F1 &&
        key <= VK_F24)
    {
        return QString(
                   "F%1"
                   ).arg(
                key - VK_F1 + 1
                );
    }


    // Lettere A-Z

    if(key >= 'A' &&
        key <= 'Z')
    {
        return QString(
            QChar(key)
            );
    }


    // Numeri 0-9

    if(key >= '0' &&
        key <= '9')
    {
        return QString(
            QChar(key)
            );
    }


    // Tasti OEM comuni

    switch(key)
    {
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
    }


    return QString(
               "VK 0x%1"
               ).arg(
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
    int qtKey =
        event->key();


    // --------------------------------------------------------
    // TASTI SPECIALI
    // --------------------------------------------------------

    switch(qtKey)
    {
    case Qt::Key_Backspace:
        return VK_BACK;

    case Qt::Key_Tab:
        return VK_TAB;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        return VK_RETURN;

    case Qt::Key_Pause:
        return VK_PAUSE;

    case Qt::Key_CapsLock:
        return VK_CAPITAL;

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

    case Qt::Key_Shift:
        return VK_LSHIFT;

    case Qt::Key_Control:
        return VK_LCONTROL;

    case Qt::Key_Alt:
        return VK_LMENU;

    case Qt::Key_Meta:
        return VK_LWIN;
    }


    // --------------------------------------------------------
    // F1 - F24
    // --------------------------------------------------------

    if(qtKey >= Qt::Key_F1 &&
        qtKey <= Qt::Key_F24)
    {
        return VK_F1 +
               (qtKey - Qt::Key_F1);
    }


    // --------------------------------------------------------
    // NUMPAD
    // --------------------------------------------------------

    switch(qtKey)
    {
    case Qt::Key_0:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD0;
        break;

    case Qt::Key_1:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD1;
        break;

    case Qt::Key_2:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD2;
        break;

    case Qt::Key_3:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD3;
        break;

    case Qt::Key_4:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD4;
        break;

    case Qt::Key_5:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD5;
        break;

    case Qt::Key_6:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD6;
        break;

    case Qt::Key_7:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD7;
        break;

    case Qt::Key_8:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD8;
        break;

    case Qt::Key_9:
        if(event->modifiers() & Qt::KeypadModifier)
            return VK_NUMPAD9;
        break;
    }


    // --------------------------------------------------------
    // LETTERE
    // --------------------------------------------------------

    if(qtKey >= Qt::Key_A &&
        qtKey <= Qt::Key_Z)
    {
        return 'A' +
               (qtKey - Qt::Key_A);
    }


    // --------------------------------------------------------
    // NUMERI
    // --------------------------------------------------------

    if(qtKey >= Qt::Key_0 &&
        qtKey <= Qt::Key_9)
    {
        return '0' +
               (qtKey - Qt::Key_0);
    }


    // --------------------------------------------------------
    // CARATTERE
    // --------------------------------------------------------

    QString text =
        event->text();


    if(!text.isEmpty())
    {
        QChar c =
            text.at(0)
                .toUpper();


        ushort unicode =
            c.unicode();


        if(unicode < 128)
        {
            return static_cast<int>(
                unicode
                );
        }
    }


    return 0;
}

#endif