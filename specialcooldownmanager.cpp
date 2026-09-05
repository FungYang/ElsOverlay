
#include "specialcooldownmanager.h"

#include <QCoreApplication>
#include <QSettings>


SpecialCooldownManager::SpecialCooldownManager()
{
}


// ============================================================
// LOAD
// ============================================================

void SpecialCooldownManager::load()
{
    m_configurations.clear();


    const QString settingsPath =
        QCoreApplication::applicationDirPath() +
        "/ElsOverlay.ini";


    QSettings settings(
        settingsPath,
        QSettings::IniFormat
        );


    const int count =
        settings.value(
                    "SpecialCooldowns/count",
                    0
                    ).toInt();


    for(
        int i = 0;
        i < count;
        ++i
        )
    {
        const QString prefix =
            QString(
                "SpecialCooldowns/%1/"
                ).arg(i);


        SpecialCooldownConfiguration configuration;


        // ====================================================
        // ACCEPTED KEYS
        // ====================================================

        const int acceptedKeysCount =
            settings.value(
                        prefix + "acceptedKeys/count",
                        0
                        ).toInt();


        for(
            int keyIndex = 0;
            keyIndex < acceptedKeysCount;
            ++keyIndex
            )
        {
            const int key =
                settings.value(
                            prefix +
                                QString(
                                    "acceptedKeys/%1"
                                    ).arg(
                                        keyIndex
                                        ),
                            0
                            ).toInt();


            if(key != 0)
            {
                configuration.acceptedKeys.append(
                    key
                    );
            }
        }


        // ====================================================
        // COMPATIBILITÀ VECCHIO FORMATO
        // ====================================================

        /*
         * Se il vecchio formato contiene ancora:
         *
         * key=162
         *
         * e non abbiamo trovato acceptedKeys,
         * convertiamo automaticamente il vecchio
         * tasto in una lista con un solo elemento.
         */

        if(configuration.acceptedKeys.isEmpty())
        {
            const int oldKey =
                settings.value(
                            prefix + "key",
                            0
                            ).toInt();


            if(oldKey != 0)
            {
                configuration.acceptedKeys.append(
                    oldKey
                    );
            }
        }


        // ====================================================
        // IMAGE
        // ====================================================

        configuration.imagePath =
            settings.value(
                        prefix + "image",
                        QString()
                        ).toString();


        // ====================================================
        // COOLDOWN
        // ====================================================

        configuration.cooldown =
            settings.value(
                        prefix + "cooldown",
                        0
                        ).toInt();


        // ====================================================
        // POSITION
        // ====================================================

        configuration.position =
            QPoint(
                settings.value(
                            prefix + "x",
                            100
                            ).toInt(),

                settings.value(
                            prefix + "y",
                            100
                            ).toInt()
                );


        // ====================================================
        // SIZE
        // ====================================================

        int width =
            settings.value(
                        prefix + "width",
                        42
                        ).toInt();


        int height =
            settings.value(
                        prefix + "height",
                        42
                        ).toInt();


        if(width < 20)
        {
            width = 42;
        }


        if(height < 20)
        {
            height = 42;
        }


        configuration.size =
            QSize(
                width,
                height
                );


        m_configurations.append(
            configuration
            );
    }
}


// ============================================================
// SAVE
// ============================================================

void SpecialCooldownManager::save() const
{
    const QString settingsPath =
        QCoreApplication::applicationDirPath() +
        "/ElsOverlay.ini";


    QSettings settings(
        settingsPath,
        QSettings::IniFormat
        );


    settings.remove(
        "SpecialCooldowns"
        );


    settings.beginGroup(
        "SpecialCooldowns"
        );


    settings.setValue(
        "count",
        m_configurations.size()
        );


    for(
        int i = 0;
        i < m_configurations.size();
        ++i
        )
    {
        const SpecialCooldownConfiguration &configuration =
            m_configurations.at(i);


        settings.beginGroup(
            QString::number(i)
            );


        // ====================================================
        // ACCEPTED KEYS
        // ====================================================

        settings.beginGroup(
            "acceptedKeys"
            );


        settings.setValue(
            "count",
            configuration.acceptedKeys.size()
            );


        for(
            int keyIndex = 0;
            keyIndex < configuration.acceptedKeys.size();
            ++keyIndex
            )
        {
            settings.setValue(
                QString::number(
                    keyIndex
                    ),

                configuration.acceptedKeys.at(
                    keyIndex
                    )
                );
        }


        settings.endGroup();


        // ====================================================
        // IMAGE
        // ====================================================

        settings.setValue(
            "image",
            configuration.imagePath
            );


        // ====================================================
        // COOLDOWN
        // ====================================================

        settings.setValue(
            "cooldown",
            configuration.cooldown
            );


        // ====================================================
        // POSITION
        // ====================================================

        settings.setValue(
            "x",
            configuration.position.x()
            );


        settings.setValue(
            "y",
            configuration.position.y()
            );


        // ====================================================
        // SIZE
        // ====================================================

        settings.setValue(
            "width",
            configuration.size.width()
            );


        settings.setValue(
            "height",
            configuration.size.height()
            );


        settings.endGroup();
    }


    settings.endGroup();


    settings.sync();
}


// ============================================================
// GET CONFIGURATIONS
// ============================================================

QList<SpecialCooldownConfiguration>
SpecialCooldownManager::configurations() const
{
    return m_configurations;
}


// ============================================================
// SET CONFIGURATIONS
// ============================================================

void SpecialCooldownManager::setConfigurations(
    const QList<SpecialCooldownConfiguration> &configurations
    )
{
    m_configurations =
        configurations;
}


// ============================================================
// ADD
// ============================================================

void SpecialCooldownManager::addConfiguration(
    const SpecialCooldownConfiguration &configuration
    )
{
    m_configurations.append(
        configuration
        );
}


// ============================================================
// REMOVE
// ============================================================

void SpecialCooldownManager::removeConfiguration(
    int index
    )
{
    if(
        index < 0 ||
        index >= m_configurations.size()
        )
    {
        return;
    }


    m_configurations.removeAt(
        index
        );
}


// ============================================================
// CLEAR
// ============================================================

void SpecialCooldownManager::clear()
{
    m_configurations.clear();
}
