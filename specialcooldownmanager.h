
#ifndef SPECIALCOOLDOWNMANAGER_H
#define SPECIALCOOLDOWNMANAGER_H

#include <QList>

#include "specialcooldownconfiguration.h"

    class SpecialCooldownManager
{
public:
    SpecialCooldownManager();

    void load();
    void save() const;

    QList<SpecialCooldownConfiguration> configurations() const;

    void setConfigurations(
        const QList<SpecialCooldownConfiguration> &configurations
        );

    void addConfiguration(
        const SpecialCooldownConfiguration &configuration
        );

    void removeConfiguration(
        int index
        );

    void clear();

private:
    QList<SpecialCooldownConfiguration> m_configurations;
};

#endif
