#ifndef KEYEDIT_H
#define KEYEDIT_H

#include <QLineEdit>

class QKeyEvent;

class KeyEdit : public QLineEdit
{
    Q_OBJECT

public:

    explicit KeyEdit(
        QWidget *parent = nullptr
        );

    int keyCode() const;

    void setKeyCode(
        int key
        );

protected:

    void keyPressEvent(
        QKeyEvent *event
        ) override;

    void keyReleaseEvent(
        QKeyEvent *event
        ) override;

private:

    QString keyName(
        int key
        ) const;

#ifdef Q_OS_WIN

    int qtKeyToVirtualKey(
        QKeyEvent *event
        ) const;

#endif

private:

    int m_keyCode = 0;
};

#endif // KEYEDIT_H