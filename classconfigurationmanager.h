#ifndef CLASSCONFIGURATIONMANAGER_H
#define CLASSCONFIGURATIONMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QPoint>
#include <QChar>
#include <QVector>
#include <QSize>


struct BuffConfiguration {
    int key = 0;
    int cooldown = 0;
    QPoint position;
    QSize size;};

struct ClassConfiguration
{
    QString id;

    QString imagePath;

    QList<BuffConfiguration> buffs;
};


class ClassConfigurationManager : public QObject
{
    Q_OBJECT

public:

    explicit ClassConfigurationManager(
        QObject *parent = nullptr
        );


    QList<ClassConfiguration> configurations() const;


    QString activeConfigurationId() const;


    void setActiveConfiguration(
        const QString &id
        );


    bool addConfiguration(
        const QString &id,
        const QString &imagePath
        );


    void removeConfiguration(
        const QString &id
        );


    bool contains(
        const QString &id
        ) const;


    QList<BuffConfiguration> buffs(
        const QString &id
        ) const;


    void setBuffs(
        const QString &id,
        const QList<BuffConfiguration> &buffs
        );


signals:

    void configurationsChanged();

    void activeConfigurationChanged(
        const QString &id
        );


private:

    QList<ClassConfiguration> m_configurations;

    QString m_activeConfigurationId;


    void load();

    void save() const;

};

#endif