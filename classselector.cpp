#include "classselector.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>


ClassSelector::ClassSelector(
    QWidget *parent
    )
    : QWidget(parent)
{

    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );


    setAttribute(
        Qt::WA_TranslucentBackground
        );


    resize(
        Columns * CellSize,
        Rows * CellSize
        );


    loadClasses();

}



void ClassSelector::loadClasses()
{

    buttons.clear();


    QDir directory(
        QCoreApplication::applicationDirPath() +
        "/images/classes"
        );


    if(!directory.exists())
        return;


    QStringList filters;

    filters
        << "*.png"
        << "*.jpg"
        << "*.jpeg";


    QFileInfoList files =
        directory.entryInfoList(
            filters,
            QDir::Files |
                QDir::Readable,
            QDir::Name
            );


    /*
     * Massimo 60 classi:
     *
     * 12 colonne × 5 righe.
     */

    const int maxClasses =
        Columns * Rows;


    if(files.size() > maxClasses)
    {
        files =
            files.mid(
                0,
                maxClasses
                );
    }



    for(int i = 0;
         i < files.size();
         ++i)
    {

        const QFileInfo &file =
            files.at(i);


        ClassButton button;


        button.id =
            file.completeBaseName();


        button.imagePath =
            file.absoluteFilePath();


        button.image =
            QPixmap(
                button.imagePath
                );


        const int column =
            i % Columns;


        const int row =
            i / Columns;


        button.rect =
            QRect(
                column * CellSize,
                row * CellSize,
                CellSize,
                CellSize
                );


        buttons.append(
            button
            );

    }

}



void ClassSelector::paintEvent(
    QPaintEvent *
    )
{

    QPainter painter(
        this
        );


    painter.setRenderHint(
        QPainter::Antialiasing
        );


    /*
     * Sfondo della finestra.
     */

    painter.fillRect(
        rect(),
        QColor(
            20,
            20,
            20,
            235
            )
        );



    /*
     * Celle delle classi.
     */

    for(const ClassButton &button :
         buttons)
    {

        painter.setPen(
            QPen(
                Qt::white,
                1
                )
            );


        painter.setBrush(
            QColor(
                40,
                40,
                40,
                220
                )
            );


        painter.drawRect(
            button.rect
            );



        if(!button.image.isNull())
        {

            painter.drawPixmap(
                button.rect.adjusted(
                    4,
                    4,
                    -4,
                    -4
                    ),
                button.image
                );

        }

    }

}



void ClassSelector::mousePressEvent(
    QMouseEvent *event
    )
{

    if(event->button() != Qt::LeftButton)
        return;


    const QPoint position =
        event->pos();


    for(const ClassButton &button :
         buttons)
    {

        if(!button.rect.contains(position))
            continue;


        /*
         * La selezione viene restituita
         * al chiamante.
         */

        emit classSelected(
            button.id,
            button.imagePath
            );


        hide();


        return;

    }

}



void ClassSelector::open()
{

    loadClasses();


    show();

    raise();

    activateWindow();

}