
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
        int cooldownTime,
        QWidget *parent = nullptr
        );


    void startCooldown();
    void resetCooldown();
    void tick();


    void setImage(
        const QString &imagePath
        );


    void setSkillName(
        const QString &name
        );


    void setCooldown(
        int cooldownTime
        );


    void setScale(
        double scale
        );


    double getScale() const;


    QString getSkillName() const;
    QString getImagePath() const;
    int getCooldown() const;


protected:

    void paintEvent(
        QPaintEvent *event
        ) override;


private:

    QString skillName;
    QString imagePath;

    QPixmap image;
    QPixmap grayImage;

    int cooldown = 0;
    int currentCooldown = 0;

    bool activeCooldown = false;

    double scale = 1.0;
};

#endif
