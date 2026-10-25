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


    // Attiva/disattiva il titolo. Se disattivato il box sparisce,
    // il cooldown viene azzerato, non puo' ripartire e non mostra
    // il glow di selezione. (Non si chiama setEnabled per non
    // nascondere QWidget::setEnabled.)
    void setTitleEnabled(
        bool enabled
        );

    bool isTitleEnabled() const;


    double getScale() const;


    QString getSkillName() const;
    QString getImagePath() const;
    double getCooldown() const;

public slots:

    void setTransparency(
        int value
        );



signals:

    // Emesso al RILASCIO del mouse dopo un trascinamento:
    // posizione finale (angolo in alto a sinistra) del box.
    void dragFinished(
        const QPoint &topLeft
        );


protected:

    void mousePressEvent(
        QMouseEvent *event
        ) override;

    void mouseMoveEvent(
        QMouseEvent *event
        ) override;

    void mouseReleaseEvent(
        QMouseEvent *event
        ) override;

    void paintEvent(
        QPaintEvent *event
        ) override;


private:

    int transparency = 255;

    bool cooldownPaused = false;
    bool selected = false;
    bool titleEnabled = true;

    // Trascinamento del box (ogni box e' una finestra indipendente).
    bool dragging = false;
    QPoint dragOffset;

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