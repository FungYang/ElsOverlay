#ifndef CLASSDATA_H
#define CLASSDATA_H

#include <QList>
#include <QString>
#include <QVector>
#include <QChar>

// Alcuni buff se usati mentre sono ancora attivi hanno effetti diversi, per cui prendo una lista di durata per rappresentare i vari stati
enum class ClassType
{
    ES,
    FL,
    BQ,
    AD,
    None
};



struct BuffData
{
    QChar key;
    QVector<int> cooldowns;
};



struct ClassData
{
    QList<BuffData> buffs;
    QString image;
};



inline ClassData getClassData(ClassType type)
{

    switch(type)
    {

    case ClassType::ES:

        return
            {
                {
                    {'A',{7}},
                    {'W',{30}},
                    {'D',{12}},
                    {'C',{34}},
                    {'R',{15}},
                },
                "ES.png"
            };


    case ClassType::FL:

        return
            {
                {
                    {'A',{7}},
                    {'E',{10}}
                },
                "FL.png"
            };


    case ClassType::BQ:

        return
            {
                {
                    {'A',{7}},
                    {'D',{18}},
                    {'C',{13,20}}
                },
                "BQ.png"
            };


    case ClassType::AD:

        return
            {
                {
                    {'A',{7}},
                },
                "AD.png"
            };


    default:

        return {};

    }

}


#endif