#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>

class QLabel;

class AtmaDebugWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AtmaDebugWindow(QWidget *parent = nullptr);

public slots:
    void updateRed(int index, const QImage &image, bool isMatch);
    void updateBlue(const QImage &image, bool isMatch);

private:
    QLabel *m_redImage = nullptr;
    QLabel *m_redState = nullptr;

    QLabel *m_blueImage = nullptr;
    QLabel *m_blueState = nullptr;

    static QPixmap makePixmap(const QImage &image);
    static void setState(QLabel *label, bool isMatch);
};