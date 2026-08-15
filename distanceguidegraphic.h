#ifndef DISTANCEGUIDEGRAPHIC_H
#define DISTANCEGUIDEGRAPHIC_H

#include <QWidget>

class DistanceGuideGraphic : public QWidget
{
    Q_OBJECT

public:

    explicit DistanceGuideGraphic(
        QWidget *parent = nullptr
        );


    virtual void setConfigurationMode(
        bool enabled
        );


signals:

    void configurationConfirmed();

protected:

    bool m_configurationMode = false;
};

#endif