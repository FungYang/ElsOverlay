#ifndef SKILLBOX_H
#define SKILLBOX_H

#include <QWidget>
#include <QPixmap>


class SkillBox : public QWidget
{
    Q_OBJECT

public:

    explicit SkillBox(
        const QString &imagePath,
        const QString &skillName,
        double cooldownTime,
        QWidget *parent = nullptr
        );


    void startCooldown();
    void resetCooldown();
    void tick();
    void pauseCooldown();
    void resumeCooldown();


    void setImage(
        const QString &imagePath
        );


    void setSkillName(
        const QString &name
        );


    void setCooldown(
        double cooldownTime
        );


    void setScale(
        double scale
        );


    void setSelected(
        bool selected
        );


    double getScale() const;


    QString getSkillName() const;
    QString getImagePath() const;
    double getCooldown() const;

public slots:

    void setTransparency(
        int value
        );


protected:

    void paintEvent(
        QPaintEvent *event
        ) override;


private:

    int transparency = 255;

    bool cooldownPaused = false;
    bool selected = false;

    QString skillName;
    QString imagePath;

    QPixmap image;
    QPixmap grayImage;

    double cooldown = 0.0;
    double currentCooldown = 0.0;

    bool activeCooldown = false;

    double scale = 1.0;
};

#endif