#include "atmadebugwindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>

AtmaDebugWindow::AtmaDebugWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Atma Zones - Debug");

    auto *layout = new QVBoxLayout(this);

    // ========================================================
    // REF 1
    // ========================================================

    auto *redTitle = new QLabel("REF 1 - CURRENT CROP", this);

    m_redImage = new QLabel(this);
    m_redImage->setMinimumSize(180, 180);
    m_redImage->setAlignment(Qt::AlignCenter);
    m_redImage->setFrameShape(QFrame::Box);

    m_redState = new QLabel("UNKNOWN", this);
    m_redState->setAlignment(Qt::AlignCenter);

    layout->addWidget(redTitle);
    layout->addWidget(m_redImage);
    layout->addWidget(m_redState);

    // ========================================================
    // INVARIANT
    // ========================================================

    auto *blueTitle = new QLabel("INVARIANT - CURRENT CROP", this);

    m_blueImage = new QLabel(this);
    m_blueImage->setMinimumSize(180, 180);
    m_blueImage->setAlignment(Qt::AlignCenter);
    m_blueImage->setFrameShape(QFrame::Box);

    m_blueState = new QLabel("UNKNOWN", this);
    m_blueState->setAlignment(Qt::AlignCenter);

    layout->addSpacing(15);
    layout->addWidget(blueTitle);
    layout->addWidget(m_blueImage);
    layout->addWidget(m_blueState);

    resize(420, 850);
}

void AtmaDebugWindow::updateRed(
    int index,
    const QImage &image,
    bool isMatch)
{
    // Per ora visualizziamo solo REF 1.
    if (index != 0)
        return;

    if (!image.isNull())
        m_redImage->setPixmap(makePixmap(image));

    setState(m_redState, isMatch);
}

void AtmaDebugWindow::updateBlue(
    const QImage &image,
    bool isMatch)
{
    if (!image.isNull())
        m_blueImage->setPixmap(makePixmap(image));

    setState(m_blueState, isMatch);
}

QPixmap AtmaDebugWindow::makePixmap(const QImage &image)
{
    if (image.isNull())
        return {};

    return QPixmap::fromImage(image).scaled(
        240,
        240,
        Qt::KeepAspectRatio,
        Qt::FastTransformation
        );
}

void AtmaDebugWindow::setState(
    QLabel *label,
    bool isMatch)
{
    if (!label)
        return;

    label->setText(isMatch ? "MATCH" : "MISMATCH");
}