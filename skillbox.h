#ifndef SKILLBOX_H
#define SKILLBOX_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>




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


protected:

    void paintEvent(QPaintEvent *event) override;


private:

    QString skillName;

    QPixmap image;
    QPixmap grayImage;

    int cooldown;
    int currentCooldown;

    bool activeCooldown = false;
};


#endif