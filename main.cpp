#include <QApplication>
#include <QTimer>

#include "overlay.h"
#include "globalkeyboard.h"
#include "skilloverlay.h"
#include "buffoverlay.h"
#include "classselector.h"
QString classTypeToString(ClassType type)
{
    switch(type)
    {
    case ClassType::ES:
        return "ES";

    case ClassType::FL:
        return "FL";

    case ClassType::BQ:
        return "BQ";

    case ClassType::AD:
        return "AD";
    }

    return "Unknown";
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);



    // =========================
    // OGGETTI PRINCIPALI
    // =========================

    GlobalKeyboard keyboard;

    Overlay overlay;

    SkillOverlay skills(&keyboard);

    BuffOverlay buffs;

    ClassSelector selector;


    // =========================
    // OVERLAY SKILL PRINCIPALE
    // =========================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::ctrlPressed,
        &overlay,
        [&overlay]()
        {
            overlay.startCooldown();
        }
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        &overlay,
        [&overlay]()
        {
            overlay.resetCooldown();
        }
        );



    // =========================
    // CHIUSURA ESC + P
    // =========================

    QObject::connect(
        &keyboard,
        &GlobalKeyboard::escPressed,
        [&]()
        {
            app.quit();
        }
        );



    // =========================
    // SISTEMA BUFF
    // =========================


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        &buffs,
        &BuffOverlay::handleKey
        );




    // =========================
    // SELETTORE CLASSI
    // =========================


    QObject::connect(
        &selector,
        &ClassSelector::classSelected,
        [&buffs](ClassType type)
        {

            ClassData data =
                getClassData(type);


            buffs.loadBuffs(
                classTypeToString(type),
                data.buffs
                );

        }
        );


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        [&buffs, &selector]()
        {
            buffs.clearBuffs();
            //selector.show();
        }
        );
    QObject::connect(
        &keyboard,
        &GlobalKeyboard::confirmPressed,
        &buffs,
        &BuffOverlay::confirmAll
        );


    // =========================
    // AVVIO
    // =========================

    overlay.show();

    skills.show();

    selector.show();


    return app.exec();
}