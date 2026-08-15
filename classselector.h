#ifndef CLASSSELECTOR_H
#define CLASSSELECTOR_H

#include <QWidget>
#include <QList>
#include <QRect>
#include <QPixmap>
#include <QString>


class ClassSelector : public QWidget
{
    Q_OBJECT

public:

    explicit ClassSelector(
        QWidget *parent = nullptr
        );


    void open();


signals:

    void classSelected(
        const QString &id,
        const QString &imagePath
        );


protected:

    void paintEvent(
        QPaintEvent *event
        ) override;


    void mousePressEvent(
        QMouseEvent *event
        ) override;


private:

    struct ClassButton
    {
        QRect rect;

        QString id;

        QString imagePath;

        QPixmap image;
    };


    QList<ClassButton> buttons;


    static constexpr int Columns = 12;

    static constexpr int Rows = 5;

    static constexpr int CellSize = 70;


    void loadClasses();

};

#endif // CLASSSELECTOR_H