#include <QApplication>

#include "overlay.h"
#include "globalkeyboard.h"
#include "skilloverlay.h"
#include "buffoverlay.h"
#include "classselector.h"
#include "buffvisionmanager.h"
#include "overlayroot.h"



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
    // ROOT OVERLAY
    // =========================


    OverlayRoot *overlayRoot =
        new OverlayRoot();


    overlayRoot->show();





    // =========================
    // OGGETTI PRINCIPALI
    // =========================


    GlobalKeyboard keyboard;




    Overlay *overlay =
        new Overlay(
            overlayRoot
            );



    SkillOverlay *skills =
        new SkillOverlay(
            &keyboard,
            overlayRoot
            );



    BuffOverlay *buffs =
        new BuffOverlay(
            overlayRoot
            );



    ClassSelector *selector =
        new ClassSelector(
            overlayRoot
            );






    // =========================
    // REGISTRA OVERLAY
    // =========================


    overlayRoot->registerOverlay(
        overlay
        );


    overlayRoot->registerOverlay(
        skills
        );


    overlayRoot->registerOverlay(
        buffs
        );


    overlayRoot->registerOverlay(
        selector
        );







    // =========================
    // TRANSCENDENCE
    // =========================



    QObject::connect(
        &keyboard,
        &GlobalKeyboard::ctrlPressed,
        overlay,
        [overlay]()
        {
            overlay->startCooldown();
        }
        );



    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        overlay,
        [overlay]()
        {
            overlay->resetCooldown();
        }
        );



    QObject::connect(
        &keyboard,
        &GlobalKeyboard::transcendenceResetPressed,
        overlay,
        [overlay]()
        {
            overlay->resetCooldown();
        }
        );



    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        overlay,
        [overlay](int key)
        {

            if(key == '6')
            {
                overlay->startCooldown();
            }

        }
        );








    // =========================
    // CHIUSURA
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
    // BUFF STANDARD
    // =========================


    QObject::connect(
        &keyboard,
        &GlobalKeyboard::keyPressed,
        buffs,
        &BuffOverlay::handleKey
        );







    QObject::connect(
        selector,
        &ClassSelector::classSelected,
        buffs,
        [buffs](ClassType type)
        {

            ClassData data =
                getClassData(type);



            buffs->loadBuffs(
                classTypeToString(type),
                data.buffs
                );

        }
        );







    QObject::connect(
        &keyboard,
        &GlobalKeyboard::resetPressed,
        buffs,
        [buffs, selector, overlayRoot]()
        {

            buffs->clearBuffs();


            selector->show();


            overlayRoot->raiseAll();

        }
        );







    QObject::connect(
        &keyboard,
        &GlobalKeyboard::confirmPressed,
        buffs,
        &BuffOverlay::confirmAll
        );









    // =========================
    // AVVIO
    // =========================


    overlay->show();

    skills->show();

    selector->show();



    overlayRoot->raiseAll();








    // =========================
    // BUFF VISION
    // =========================


    BuffVisionManager atma(
        &keyboard,
        overlayRoot
        );





    return app.exec();

}