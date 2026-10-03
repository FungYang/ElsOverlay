#include "customsearchertemplatecrop.h"

#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QCloseEvent>


CustomSearcherTemplateCrop::CustomSearcherTemplateCrop(
    const QImage &source,
    const QSize &initialCropSize,
    QWidget *parent
    )
    : QWidget(parent),
    m_source(
        source.convertToFormat(
            QImage::Format_ARGB32
            )
        )
{
    setWindowTitle(
        QStringLiteral(
            "Custom Searcher - Template Crop"
            )
        );

    setWindowFlags(
        Qt::Tool |
        Qt::WindowStaysOnTopHint |
        Qt::WindowCloseButtonHint
        );

    setFocusPolicy(
        Qt::StrongFocus
        );

    setMouseTracking(
        true
        );


    const int cropWidth =
        qMax(
            1,
            initialCropSize.width()
            );

    const int cropHeight =
        qMax(
            1,
            initialCropSize.height()
            );


    if (!m_source.isNull() &&
        m_source.width() >= cropWidth &&
        m_source.height() >= cropHeight)
    {
        m_cropRect =
            QRect(
                (m_source.width() - cropWidth) / 2,
                (m_source.height() - cropHeight) / 2,
                cropWidth,
                cropHeight
                );
    }
    else if (!m_source.isNull())
    {
        m_cropRect =
            QRect(
                0,
                0,
                qMin(
                    cropWidth,
                    m_source.width()
                    ),
                qMin(
                    cropHeight,
                    m_source.height()
                    )
                );
    }

    clampCrop();


    const int imageWidth =
        m_source.width() * PIXEL_SIZE;

    const int imageHeight =
        m_source.height() * PIXEL_SIZE;

    setMinimumSize(
        imageWidth + IMAGE_MARGIN * 2,
        imageHeight + 120
        );

    resize(
        imageWidth + IMAGE_MARGIN * 2,
        imageHeight + 120
        );
}


QRect CustomSearcherTemplateCrop::cropRect() const
{
    return m_cropRect;
}


QImage CustomSearcherTemplateCrop::croppedImage() const
{
    if (m_source.isNull() ||
        m_cropRect.isEmpty())
    {
        return QImage();
    }

    return m_source.copy(
        m_cropRect
        );
}


void CustomSearcherTemplateCrop::increaseCropSize()
{
    if (m_source.isNull())
        return;

    if (m_cropRect.width() >= m_source.width() ||
        m_cropRect.height() >= m_source.height())
    {
        return;
    }

    m_cropRect.setWidth(
        m_cropRect.width() + 1
        );

    m_cropRect.setHeight(
        m_cropRect.height() + 1
        );

    clampCrop();
    update();
}


void CustomSearcherTemplateCrop::decreaseCropSize()
{
    if (m_cropRect.width() <= 1 ||
        m_cropRect.height() <= 1)
    {
        return;
    }

    m_cropRect.setWidth(
        m_cropRect.width() - 1
        );

    m_cropRect.setHeight(
        m_cropRect.height() - 1
        );

    clampCrop();
    update();
}


void CustomSearcherTemplateCrop::moveLeft()
{
    m_cropRect.translate(
        -1,
        0
        );

    clampCrop();
    update();
}


void CustomSearcherTemplateCrop::moveRight()
{
    m_cropRect.translate(
        1,
        0
        );

    clampCrop();
    update();
}


void CustomSearcherTemplateCrop::moveUp()
{
    m_cropRect.translate(
        0,
        -1
        );

    clampCrop();
    update();
}


void CustomSearcherTemplateCrop::moveDown()
{
    m_cropRect.translate(
        0,
        1
        );

    clampCrop();
    update();
}


void CustomSearcherTemplateCrop::clampCrop()
{
    if (m_source.isNull())
        return;


    if (m_cropRect.width() > m_source.width())
    {
        m_cropRect.setWidth(
            m_source.width()
            );
    }

    if (m_cropRect.height() > m_source.height())
    {
        m_cropRect.setHeight(
            m_source.height()
            );
    }


    if (m_cropRect.left() < 0)
        m_cropRect.moveLeft(0);

    if (m_cropRect.top() < 0)
        m_cropRect.moveTop(0);


    if (m_cropRect.right() >= m_source.width())
    {
        m_cropRect.moveRight(
            m_source.width() - 1
            );
    }

    if (m_cropRect.bottom() >= m_source.height())
    {
        m_cropRect.moveBottom(
            m_source.height() - 1
            );
    }
}


void CustomSearcherTemplateCrop::paintEvent(
    QPaintEvent *
    )
{
    QPainter painter(this);

    painter.fillRect(
        rect(),
        QColor(25, 25, 25)
        );

    if (m_source.isNull())
        return;


    const int imageLeft =
        IMAGE_MARGIN;

    const int imageTop =
        IMAGE_MARGIN;


    const QRect target(
        imageLeft,
        imageTop,
        m_source.width() * PIXEL_SIZE,
        m_source.height() * PIXEL_SIZE
        );


    painter.drawImage(
        target,
        m_source
        );


    // ==================================================
    // PIXEL GRID
    // ==================================================

    painter.setPen(
        QPen(
            QColor(255, 255, 255, 45),
            1
            )
        );


    for (int x = 0;
         x <= m_source.width();
         ++x)
    {
        const int px =
            imageLeft +
            x * PIXEL_SIZE;

        painter.drawLine(
            px,
            imageTop,
            px,
            imageTop +
                m_source.height() *
                    PIXEL_SIZE
            );
    }


    for (int y = 0;
         y <= m_source.height();
         ++y)
    {
        const int py =
            imageTop +
            y * PIXEL_SIZE;

        painter.drawLine(
            imageLeft,
            py,
            imageLeft +
                m_source.width() *
                    PIXEL_SIZE,
            py
            );
    }


    // ==================================================
    // CROP RECT
    // ==================================================

    const QRect cropVisual(
        imageLeft +
            m_cropRect.left() *
                PIXEL_SIZE,

        imageTop +
            m_cropRect.top() *
                PIXEL_SIZE,

        m_cropRect.width() *
            PIXEL_SIZE,

        m_cropRect.height() *
            PIXEL_SIZE
        );


    painter.setPen(
        QPen(
            QColor(255, 220, 0),
            4
            )
        );

    painter.setBrush(
        Qt::NoBrush
        );

    painter.drawRect(
        cropVisual
        );


    // ==================================================
    // INFO
    // ==================================================

    painter.setPen(
        Qt::white
        );

    QFont font =
        painter.font();

    font.setPointSize(11);
    font.setBold(true);

    painter.setFont(
        font
        );


    const QString info =
        QString(
            "Ritaglio: %1 x %2 pixel    "
            "Posizione: %3,%4\n"
            "Frecce = sposta    "
            "B = +1 px    "
            "V = -1 px    "
            "Mouse = trascina\n"
            "ENTER = salva    "
            "ESC = annulla"
            )
            .arg(
                m_cropRect.width()
                )
            .arg(
                m_cropRect.height()
                )
            .arg(
                m_cropRect.x()
                )
            .arg(
                m_cropRect.y()
                );


    painter.drawText(
        QRect(
            IMAGE_MARGIN,
            imageTop +
                m_source.height() *
                    PIXEL_SIZE +
                15,
            width() -
                IMAGE_MARGIN * 2,
            70
            ),
        Qt::AlignLeft |
            Qt::TextWordWrap,
        info
        );
}


void CustomSearcherTemplateCrop::mousePressEvent(
    QMouseEvent *event
    )
{
    if (event->button() != Qt::LeftButton)
        return;


    const QPoint local =
        event->pos() -
        QPoint(
            IMAGE_MARGIN,
            IMAGE_MARGIN
            );


    const QPoint sourcePixel(
        local.x() / PIXEL_SIZE,
        local.y() / PIXEL_SIZE
        );


    if (!QRect(
             QPoint(0, 0),
             m_source.size()
             ).contains(sourcePixel))
    {
        return;
    }


    m_mousePressSource =
        sourcePixel;

    m_cropAtMousePress =
        m_cropRect.topLeft();

    m_dragging =
        true;

    setFocus();
}


void CustomSearcherTemplateCrop::mouseMoveEvent(
    QMouseEvent *event
    )
{
    if (!m_dragging)
        return;


    const QPoint local =
        event->pos() -
        QPoint(
            IMAGE_MARGIN,
            IMAGE_MARGIN
            );


    const QPoint sourcePixel(
        local.x() / PIXEL_SIZE,
        local.y() / PIXEL_SIZE
        );


    const QPoint delta =
        sourcePixel -
        m_mousePressSource;


    m_cropRect.moveTopLeft(
        m_cropAtMousePress +
        delta
        );


    clampCrop();

    update();
}


void CustomSearcherTemplateCrop::mouseReleaseEvent(
    QMouseEvent *event
    )
{
    if (event->button() ==
        Qt::LeftButton)
    {
        m_dragging = false;
    }

    QWidget::mouseReleaseEvent(event);
}


void CustomSearcherTemplateCrop::keyPressEvent(
    QKeyEvent *event
    )
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

    case Qt::Key_B:
        increaseCropSize();
        return;

    case Qt::Key_V:
        decreaseCropSize();
        return;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        acceptCrop();
        return;

    case Qt::Key_Escape:
        close();
        return;

    default:
        break;
    }


    QWidget::keyPressEvent(
        event
        );
}


void CustomSearcherTemplateCrop::acceptCrop()
{
    const QImage image =
        croppedImage();

    if (image.isNull())
        return;

    m_accepted = true;

    emit accepted(image);

    close();
}
void CustomSearcherTemplateCrop::closeEvent(
    QCloseEvent *event
    )
{
    if (m_dragging)
        m_dragging = false;

    if (!m_accepted)
        emit canceled();

    QWidget::closeEvent(event);
}