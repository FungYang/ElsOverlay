#pragma once

#include <QLabel>

#ifdef QT_DEBUG

class TranscendenceDebugOverlay : public QLabel
{
    Q_OBJECT

public:
    explicit TranscendenceDebugOverlay(QWidget *parent = nullptr);

public slots:
    void setSearching();
    void setFirstFindTime(qint64 milliseconds);
};

#endif