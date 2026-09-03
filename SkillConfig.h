#ifndef SKILLCONFIG_H
#define SKILLCONFIG_H

#include <QList>
#include <QString>


enum class SkillActivation
{
    CtrlCipolla,
    Combo,
    Artifact
};


struct SkillConfig
{
    QString name;
    QString imagePath;

    int cooldown = 0;

    SkillActivation activation =
        SkillActivation::CtrlCipolla;

    QList<int> comboKeys;
};


struct SkillOverlayConfig
{
    // Tasto che inizia la sequenza
    int stateKey = 'G';

    // Tasto Cipolla
    int cipollaKey = '6';

    // Skill
    SkillConfig up;
    SkillConfig left;
    SkillConfig down;
    SkillConfig right;
    SkillConfig artifact;
};

#endif