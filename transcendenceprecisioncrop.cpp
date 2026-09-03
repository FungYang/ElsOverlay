#include "transcendenceprecisioncrop.h"
#include "transcendencevisionconfig.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

TranscendencePrecisionCrop::TranscendencePrecisionCrop(
    const QImage &source,
    QWidget *parent
    )
    : QWidget(parent),
    m_source(source.convertToFormat(QImage::Format_ARGB32)),
    m_zoom(TranscendenceVisionConfig::PRECISION_ZOOM),
    m_pixelSize(TranscendenceVisionConfig::PRECISION_ZOOM)
{
    setWindowTitle("Ritaglio preciso 28x28 - 300%");
    setWindowFlags(
        Qt::Tool |
        Qt::WindowStaysOnTopHint |
        Qt::WindowCloseButtonHint
        );

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    if (m_source.width() >= TranscendenceVisionConfig::ICON_WIDTH &&
        m_source.height() >= TranscendenceVisionConfig::ICON_HEIGHT)
    {
        m_cropRect =
            QRect(
                (m_source.width() -
                 TranscendenceVisionConfig::ICON_WIDTH) / 2,
                (m_source.height() -
                 TranscendenceVisionConfig::ICON_HEIGHT) / 2,
                TranscendenceVisionConfig::ICON_WIDTH,
                TranscendenceVisionConfig::ICON_HEIGHT
                );
    }

    const int imageWidth =
        m_source.width() * m_zoom;

    const int imageHeight =
        m_source.height() * m_zoom;

    setMinimumSize(
        imageWidth + 40,
        imageHeight + 120
        );

    resize(
        imageWidth + 40,
        imageHeight + 120
        );
}

QRect TranscendencePrecisionCrop::cropRect() const
{
    return m_cropRect;
}

QImage TranscendencePrecisionCrop::croppedImage() const
{
    if (m_source.isNull() ||
        m_cropRect.width() != TranscendenceVisionConfig::ICON_WIDTH ||
        m_cropRect.height() != TranscendenceVisionConfig::ICON_HEIGHT)
    {
        return QImage();
    }

    return m_source.copy(m_cropRect);
}

void TranscendencePrecisionCrop::moveLeft()
{
    m_cropRect.translate(-1, 0);
    clampCrop();
    update();
}

void TranscendencePrecisionCrop::moveRight()
{
    m_cropRect.translate(1, 0);
    clampCrop();
    update();
}

void TranscendencePrecisionCrop::moveUp()
{
    m_cropRect.translate(0, -1);
    clampCrop();
    update();
}

void TranscendencePrecisionCrop::moveDown()
{
    m_cropRect.translate(0, 1);
    clampCrop();
    update();
}

void TranscendencePrecisionCrop::clampCrop()
{
    if (m_source.isNull())
        return;

    if (m_cropRect.width() > m_source.width())
        m_cropRect.setWidth(m_source.width());

    if (m_cropRect.height() > m_source.height())
        m_cropRect.setHeight(m_source.height());

    if (m_cropRect.left() < 0)
        m_cropRect.moveLeft(0);

    if (m_cropRect.top() < 0)
        m_cropRect.moveTop(0);

    if (m_cropRect.right() >= m_source.width())
        m_cropRect.moveRight(m_source.width() - 1);

    if (m_cropRect.bottom() >= m_source.height())
        m_cropRect.moveBottom(m_source.height() - 1);
}

void TranscendencePrecisionCrop::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(25, 25, 25));

    if (m_source.isNull())
        return;

    const int imageLeft = 20;
    const int imageTop = 20;

    const QRect target(
        imageLeft,
        imageTop,
        m_source.width() * m_zoom,
        m_source.height() * m_zoom
        );

    p.drawImage(
        target,
        m_source
        );

    // Griglia dei pixel reali.
    p.setPen(QPen(QColor(255, 255, 255, 45), 1));

    for (int x = 0; x <= m_source.width(); ++x)
    {
        const int px = imageLeft + x * m_pixelSize;
        p.drawLine(
            px,
            imageTop,
            px,
            imageTop + m_source.height() * m_pixelSize
            );
    }

    for (int y = 0; y <= m_source.height(); ++y)
    {
        const int py = imageTop + y * m_pixelSize;
        p.drawLine(
            imageLeft,
            py,
            imageLeft + m_source.width() * m_pixelSize,
            py
            );
    }

    // Riquadro esatto di 28x28 pixel.
    const QRect cropVisual(
        imageLeft + m_cropRect.left() * m_pixelSize,
        imageTop + m_cropRect.top() * m_pixelSize,
        m_cropRect.width() * m_pixelSize,
        m_cropRect.height() * m_pixelSize
        );

    p.setPen(QPen(QColor(255, 220, 0), 4));
    p.setBrush(Qt::NoBrush);
    p.drawRect(cropVisual);

    p.setPen(Qt::white);

    QFont f = p.font();
    f.setPointSize(11);
    f.setBold(true);
    p.setFont(f);

    const QString info =
        QString(
            "Ritaglio: %1 x %2 pixel    Posizione: %3,%4\n"
            "Frecce = sposta di 1 pixel    Mouse = trascina    "
            "ENTER = salva    ESC = annulla"
            )
            .arg(m_cropRect.width())
            .arg(m_cropRect.height())
            .arg(m_cropRect.x())
            .arg(m_cropRect.y());

    p.drawText(
        QRect(
            20,
            imageTop + m_source.height() * m_zoom + 15,
            width() - 40,
            70
            ),
        Qt::AlignLeft | Qt::TextWordWrap,
        info
        );
}

void TranscendencePrecisionCrop::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    const int imageLeft = 20;
    const int imageTop = 20;

    const QPoint local =
        event->pos() - QPoint(imageLeft, imageTop);

    const QPoint sourcePixel(
        local.x() / m_pixelSize,
        local.y() / m_pixelSize
        );

    if (!QRect(QPoint(0, 0), m_source.size()).contains(sourcePixel))
        return;

    m_mousePressSource = sourcePixel;
    m_cropAtMousePress = m_cropRect.topLeft();
    m_dragging = true;

    setFocus();
}

void TranscendencePrecisionCrop::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging)
        return;

    const int imageLeft = 20;
    const int imageTop = 20;

    const QPoint local =
        event->pos() - QPoint(imageLeft, imageTop);

    const QPoint sourcePixel(
        local.x() / m_pixelSize,
        local.y() / m_pixelSize
        );

    const QPoint delta =
        sourcePixel - m_mousePressSource;

    m_cropRect.moveTopLeft(
        m_cropAtMousePress + delta
        );

    clampCrop();
    update();
}

void TranscendencePrecisionCrop::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Left:
        moveLeft();
        return;

    case Qt::Key_Right:
        moveRight();
        return;

    case Qt::Key_Up:
        moveUp();
        return;

    case Qt::Key_Down:
        moveDown();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        acceptCrop();
        return;

    case Qt::Key_Escape:
        emit canceled();
        close();
        return;

    default:
        break;
    }

    QWidget::keyPressEvent(event);
}

void TranscendencePrecisionCrop::acceptCrop()
{
    const QImage image = croppedImage();

    if (image.isNull())
        return;

    emit accepted(image);
    close();
}
