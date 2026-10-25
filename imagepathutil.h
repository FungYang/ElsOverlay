#pragma once

// ============================================================
// PERCORSI DELLE IMMAGINI RELATIVI ALL'EXE
//
// Le immagini scelte dall'utente (es. Special Cooldown) venivano
// salvate nell'ini con il percorso ASSOLUTO del file dialog:
//
//     C:/Users/.../QT_Tut/ElsOverlay/images/foo.png
//
// quindi rinominando o spostando la cartella dell'app le immagini
// sparivano. Qui il percorso si salva RELATIVO alla cartella
// dell'exe (come fanno Atma, Gate, ecc. con applicationDirPath()):
//
//     images/foo.png
//
// In memoria il percorso resta sempre ASSOLUTO, cosi' chi lo usa
// (QPixmap, QFileInfo, ...) non cambia.
// ============================================================

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>

namespace ImagePath
{

// Separatori sempre '/' (anche se l'ini contiene dei '\\' scritti a
// mano o da vecchie versioni).
inline QString normalized(
    QString path
    )
{
    path.replace(
        '\\',
        '/'
        );

    return path;
}


// Assoluto anche nello stile "C:/..." (cosi' il comportamento e'
// identico su qualunque piattaforma e si puo' verificare ovunque).
inline bool isAbsolute(
    const QString &path
    )
{
    if(QDir::isAbsolutePath(path))
        return true;

    return path.size() >= 3 &&
           path.at(0).isLetter() &&
           path.at(1) == ':' &&
           path.at(2) == '/';
}


// Cartella dell'exe.
inline QString appDir()
{
    return QCoreApplication::applicationDirPath();
}


// Cartella standard delle immagini: <exe>/images
inline QString imagesDir()
{
    return appDir() + "/images";
}


// Percorso da SALVARE nell'ini:
//  - file dentro la cartella dell'exe -> relativo ("images/foo.png")
//  - file fuori dalla cartella dell'exe -> assoluto (non c'e'
//    un modo sensato di renderlo relativo)
//  - gia' relativo -> invariato
inline QString toStored(
    const QString &path
    )
{
    if(path.isEmpty())
        return path;

    const QString normalizedPath =
        normalized(path);

    if(!isAbsolute(normalizedPath))
        return normalizedPath;

    const QString relative =
        QDir(appDir()).relativeFilePath(
            QFileInfo(normalizedPath).absoluteFilePath()
            );

    // relativeFilePath() restituisce "../..." se il file e' fuori
    // dalla cartella dell'exe (o un percorso assoluto se e' su un
    // altro disco).
    const bool outside =
        relative == ".." ||
        relative.startsWith("../") ||
        isAbsolute(relative);

    if(outside)
        return normalizedPath;

    return relative;
}


// Percorso REALE (assoluto) da usare a runtime, a partire da
// quello salvato nell'ini (o digitato dall'utente).
//  - relativo -> relativo alla cartella dell'exe
//  - assoluto che esiste -> invariato
//  - assoluto che NON esiste piu' (cartella rinominata/spostata,
//    ini vecchio) -> cerco lo stesso nome file in <exe>/images
//  - non trovato -> invariato (cosi' l'errore resta visibile)
inline QString resolve(
    const QString &stored
    )
{
    const QString path =
        normalized(
            stored.trimmed()
            );

    if(path.isEmpty())
        return path;

    if(!isAbsolute(path))
    {
        return QDir::cleanPath(
            appDir() + "/" + path
            );
    }

    if(QFileInfo::exists(path))
        return path;

    // ---- Recupero: stesso nome file dentro <exe>/images ----

    const QString fileName =
        QFileInfo(path).fileName();

    if(fileName.isEmpty())
        return path;

    // Prima direttamente in images/.
    const QString direct =
        imagesDir() + "/" + fileName;

    if(QFileInfo::exists(direct))
        return QDir::cleanPath(direct);

    // Poi nelle sottocartelle (es. images/Classes/...).
    QDirIterator it(
        imagesDir(),
        QDir::Files,
        QDirIterator::Subdirectories
        );

    while(it.hasNext())
    {
        const QString candidate =
            it.next();

        if(QFileInfo(candidate).fileName().compare(
               fileName,
               Qt::CaseInsensitive
               ) == 0)
        {
            return QDir::cleanPath(candidate);
        }
    }

    return path;
}


// Cartella da cui far partire il dialogo "Seleziona immagine":
// quella dell'immagine gia' scelta, altrimenti <exe>/images.
inline QString dialogStartDir(
    const QString &currentText
    )
{
    if(!currentText.trimmed().isEmpty())
    {
        const QFileInfo info(
            resolve(currentText)
            );

        if(info.exists())
            return info.absolutePath();
    }

    if(QDir(imagesDir()).exists())
        return imagesDir();

    return appDir();
}

}
