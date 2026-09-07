#ifndef SPECIALCOOLDOWNOVERLAY_H
#define SPECIALCOOLDOWNOVERLAY_H

#include <QList>
#include <QTimer>
#include <QWidget>

#include "specialcooldownmanager.h"


    class GlobalKeyboard;
class OverlayRoot;


class SpecialCooldownOverlay : public QWidget
{
    Q_OBJECT

public:

    explicit SpecialCooldownOverlay(
        SpecialCooldownManager *manager,
        GlobalKeyboard *keyboard,
        OverlayRoot *root,
        QWidget *parent = nullptr
        );


public slots:

    void setEnabled(
        bool enabled
        );

    void resetAll();


private slots:

    void activateKey(
        int key
        );

    void updateCooldowns();


private:

    struct CooldownState
    {
        SpecialCooldownConfiguration configuration;

        int remainingMilliseconds = 0;

        bool active = false;
    };


    SpecialCooldownManager *m_manager = nullptr;

    GlobalKeyboard *m_keyboard = nullptr;

    OverlayRoot *m_root = nullptr;


    QList<CooldownState> m_states;

    QList<QWidget *> m_widgets;


    QTimer m_timer;


    bool m_enabled = false;


    void loadConfigurations();

    void createWidgets();

    void updateWidgets();

    void updateWidget(
        int index
        );

    void clearWidgets();

    void savePositionsAndSizes();

    QString keyName(
        int key
        ) const;
};


#endif

