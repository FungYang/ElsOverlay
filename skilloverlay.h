#ifndef SKILLOVERLAY_H
#define SKILLOVERLAY_H

#include <QWidget>
#include <QTimer>
#include "skillbox.h"
#include "globalkeyboard.h"
#include <QVector>
#include <QSettings>


class SkillOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit SkillOverlay(
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );
    void resetAllCooldowns();
    void checkSequences();

protected:

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;


private:

    SkillBox *concerto;
    SkillBox *artifact;
    SkillBox *nightParade;
    SkillBox *settingSun;
    QTimer *timer;
    QPoint dragPosition;
    GlobalKeyboard *keyboard;
    QVector<int> keyBuffer;
    static constexpr int MAX_BUFFER_SIZE = 3;
    bool trackingActive = false;
};


#endif