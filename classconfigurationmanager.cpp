
#include "classconfigurationmanager.h"

#include <QSettings>


ClassConfigurationManager::ClassConfigurationManager(
    QObject *parent
    )
    : QObject(parent)
{
    load();
}


// ============================================================
// CONFIGURATIONS
// ============================================================

QList<ClassConfiguration>
ClassConfigurationManager::configurations() const
{
    return m_configurations;
}


// ============================================================
// ACTIVE CONFIGURATION
// ============================================================

QString
ClassConfigurationManager::activeConfigurationId() const
{
    return m_activeConfigurationId;
}


// ============================================================
// CONTAINS
// ============================================================

bool ClassConfigurationManager::contains(
    const QString &id
    ) const
{
    for(const ClassConfiguration &configuration :
         m_configurations)
    {
        if(configuration.id == id)
        {
            return true;
        }
    }


    return false;
}


// ============================================================
// ADD CONFIGURATION
// ============================================================

bool ClassConfigurationManager::addConfiguration(
    const QString &id,
    const QString &imagePath
    )
{
    if(id.isEmpty())
    {
        return false;
    }


    if(contains(id))
    {
        return false;
    }


    ClassConfiguration configuration;


    configuration.id =
        id;


    configuration.imagePath =
        imagePath;


    m_configurations.append(
        configuration
        );


    /*
     * Se è la prima configurazione,
     * la rendiamo automaticamente attiva.
     */

    if(m_activeConfigurationId.isEmpty())
    {
        m_activeConfigurationId =
            id;
    }


    save();


    emit configurationsChanged();


    emit activeConfigurationChanged(
        m_activeConfigurationId
        );


    return true;
}


// ============================================================
// REMOVE CONFIGURATION
// ============================================================

void ClassConfigurationManager::removeConfiguration(
    const QString &id
    )
{
    for(int i = 0;
         i < m_configurations.size();
         ++i)
    {
        if(m_configurations[i].id != id)
        {
            continue;
        }


        m_configurations.removeAt(
            i
            );


        /*
         * Se abbiamo cancellato quella attiva,
         * scegliamo automaticamente la prima
         * configurazione rimasta.
         */

        if(m_activeConfigurationId == id)
        {
            if(m_configurations.isEmpty())
            {
                m_activeConfigurationId.clear();
            }
            else
            {
                m_activeConfigurationId =
                    m_configurations.first().id;
            }
        }


        save();


        emit configurationsChanged();


        emit activeConfigurationChanged(
            m_activeConfigurationId
            );


        return;
    }
}


// ============================================================
// SET ACTIVE CONFIGURATION
// ============================================================

void ClassConfigurationManager::setActiveConfiguration(
    const QString &id
    )
{
    if(!contains(id))
    {
        return;
    }


    if(m_activeConfigurationId == id)
    {
        return;
    }


    m_activeConfigurationId =
        id;


    save();


    emit activeConfigurationChanged(
        id
        );
}


// ============================================================
// LOAD
// ============================================================

void ClassConfigurationManager::load()
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    int count =
        settings.value(
                    "ClassConfigurations/count",
                    0
                    ).toInt();


    for(int i = 0;
         i < count;
         ++i)
    {
        QString prefix =
            "ClassConfigurations/" +
            QString::number(i);


        ClassConfiguration configuration;


        configuration.id =
            settings.value(
                        prefix + "/id"
                        ).toString();


        configuration.imagePath =
            settings.value(
                        prefix + "/imagePath"
                        ).toString();


        int buffCount =
            settings.value(
                        prefix + "/buffCount",
                        0
                        ).toInt();


        for(int j = 0;
             j < buffCount;
             ++j)
        {
            QString buffPrefix =
                prefix +
                "/buffs/" +
                QString::number(j);


            /*
             * Il tasto viene salvato come VK code.
             *
             * Esempi:
             *
             * L-Ctrl  = 162
             * R-Ctrl  = 163
             * L-Shift = 160
             * R-Shift = 161
             * L-Alt   = 164
             * R-Alt   = 165
             * L-Win   = 91
             * R-Win   = 92
             */

            int keyCode =
                settings.value(
                            buffPrefix + "/key",
                            0
                            ).toInt();


            if(keyCode == 0)
            {
                continue;
            }


            BuffConfiguration buff;


            buff.key =
                keyCode;


            // ====================================================
            // COOLDOWN
            // ====================================================

            buff.cooldown =
                settings.value(
                            buffPrefix + "/cooldown",
                            1
                            ).toInt();


            // ====================================================
            // POSITION
            // ====================================================

            buff.position =
                QPoint(
                    settings.value(
                                buffPrefix + "/x",
                                100
                                ).toInt(),

                    settings.value(
                                buffPrefix + "/y",
                                100
                                ).toInt()
                    );


            // ====================================================
            // SIZE
            // ====================================================

            int width =
                settings.value(
                            buffPrefix + "/width",
                            42
                            ).toInt();


            int height =
                settings.value(
                            buffPrefix + "/height",
                            42
                            ).toInt();


            /*
             * Protezione per vecchie configurazioni
             * oppure valori non validi.
             *
             * La dimensione minima consentita dal BuffBox
             * è 20x20.
             */

            if(width < 20)
            {
                width =
                    42;
            }


            if(height < 20)
            {
                height =
                    42;
            }


            buff.size =
                QSize(
                    width,
                    height
                    );


            configuration.buffs.append(
                buff
                );
        }


        if(configuration.id.isEmpty())
        {
            continue;
        }


        m_configurations.append(
            configuration
            );
    }


    // ============================================================
    // ACTIVE CONFIGURATION
    // ============================================================

    m_activeConfigurationId =
        settings.value(
                    "ClassConfigurations/active",
                    ""
                    ).toString();


    /*
     * Se il salvataggio precedente contiene
     * un ID non più presente, lo correggiamo.
     */

    if(
        !m_activeConfigurationId.isEmpty() &&
        !contains(m_activeConfigurationId)
        )
    {
        m_activeConfigurationId.clear();
    }


    if(
        m_activeConfigurationId.isEmpty() &&
        !m_configurations.isEmpty()
        )
    {
        m_activeConfigurationId =
            m_configurations.first().id;
    }
}


// ============================================================
// SAVE
// ============================================================

void ClassConfigurationManager::save() const
{
    QSettings settings(
        "ElsOverlay.ini",
        QSettings::IniFormat
        );


    /*
     * Ricostruiamo completamente la sezione
     * delle configurazioni.
     *
     * In questo modo eventuali buff cancellati
     * non rimangono nel file INI.
     */

    settings.remove(
        "ClassConfigurations"
        );


    settings.setValue(
        "ClassConfigurations/count",
        m_configurations.size()
        );


    for(int i = 0;
         i < m_configurations.size();
         ++i)
    {
        const ClassConfiguration &configuration =
            m_configurations[i];


        QString prefix =
            "ClassConfigurations/" +
            QString::number(i);


        // ========================================================
        // CONFIGURATION DATA
        // ========================================================

        settings.setValue(
            prefix + "/id",
            configuration.id
            );


        settings.setValue(
            prefix + "/imagePath",
            configuration.imagePath
            );


        settings.setValue(
            prefix + "/buffCount",
            configuration.buffs.size()
            );


        // ========================================================
        // BUFFS
        // ========================================================

        for(int j = 0;
             j < configuration.buffs.size();
             ++j)
        {
            const BuffConfiguration &buff =
                configuration.buffs[j];


            QString buffPrefix =
                prefix +
                "/buffs/" +
                QString::number(j);


            /*
             * Salviamo direttamente il VK code
             * come intero.
             *
             * Esempi:
             *
             * L-Ctrl  = 162
             * R-Ctrl  = 163
             * L-Shift = 160
             * R-Shift = 161
             * L-Alt   = 164
             * R-Alt   = 165
             * L-Win   = 91
             * R-Win   = 92
             */

            settings.setValue(
                buffPrefix + "/key",
                buff.key
                );


            // ====================================================
            // COOLDOWN
            // ====================================================

            settings.setValue(
                buffPrefix + "/cooldown",
                buff.cooldown
                );


            // ====================================================
            // POSITION
            // ====================================================

            settings.setValue(
                buffPrefix + "/x",
                buff.position.x()
                );


            settings.setValue(
                buffPrefix + "/y",
                buff.position.y()
                );


            // ====================================================
            // SIZE
            // ====================================================

            settings.setValue(
                buffPrefix + "/width",
                buff.size.width()
                );


            settings.setValue(
                buffPrefix + "/height",
                buff.size.height()
                );
        }
    }


    // ============================================================
    // ACTIVE CONFIGURATION
    // ============================================================

    settings.setValue(
        "ClassConfigurations/active",
        m_activeConfigurationId
        );


    settings.sync();
}


// ============================================================
// SET BUFFS
// ============================================================

void ClassConfigurationManager::setBuffs(
    const QString &id,
    const QList<BuffConfiguration> &buffs
    )
{
    for(ClassConfiguration &configuration :
         m_configurations)
    {
        if(configuration.id != id)
        {
            continue;
        }


        configuration.buffs =
            buffs;


        save();


        emit configurationsChanged();


        return;
    }
}


// ============================================================
// GET BUFFS
// ============================================================

QList<BuffConfiguration>
ClassConfigurationManager::buffs(
    const QString &id
    ) const
{
    for(const ClassConfiguration &configuration :
         m_configurations)
    {
        if(configuration.id == id)
        {
            return configuration.buffs;
        }
    }


    return {};
}
