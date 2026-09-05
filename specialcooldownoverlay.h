#ifndef SPECIALCOOLDOWNOVERLAY_H
#define SPECIALCOOLDOWNOVERLAY_H

#include <QList>
#include <QPoint>
#include <QSize>
#include <QTimer>
#include <QWidget>

#include "specialcooldownconfiguration.h"

    class GlobalKeyboard;
class SpecialCooldownManager;

class SpecialCooldownOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit SpecialCooldownOverlay(
        SpecialCooldownManager *manager,
        GlobalKeyboard *keyboard,
        QWidget *parent = nullptr
        );

    void setEnabled(
        bool enabled
        );

    void resetAll();

protected:
    void paintEvent(
        QPaintEvent *event
        ) override;

    void resizeEvent(
        QResizeEvent *event
        ) override;

private:

    struct CooldownState
    {
        SpecialCooldownConfiguration configuration;

        int remainingMilliseconds = 0;
        bool active = false;
    };


    QList<CooldownState> m_states;

    SpecialCooldownManager *m_manager = nullptr;
    GlobalKeyboard *m_keyboard = nullptr;

    QTimer m_timer;

    bool m_enabled = false;

    void loadConfigurations();

    void createWidgets();

    void clearWidgets();

    void activateKey(
        int key
        );

    void updateCooldowns();

    void updateWidget(
        int index
        );

    void savePositionsAndSizes();

    QString keyName(
        int key
        ) const;
};

#endif
