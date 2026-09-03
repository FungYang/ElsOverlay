#ifndef KEYEDIT_H
#define KEYEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif


class KeyEdit : public QLineEdit
{
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


private:

    int m_keyCode = 0;


    QString keyName(
        int key
        ) const;


#ifdef Q_OS_WIN

    int qtKeyToVirtualKey(
        QKeyEvent *event
        ) const;

#endif
};

#endif