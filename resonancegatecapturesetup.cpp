#include "resonancegatecapturesetup.h"
#include "screencapture.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QFile>

ResonanceGateCaptureSetup::ResonanceGateCaptureSetup(QWidget *parent)
    : QWidget(parent)
{
    QScreen *screen = QApplication::primaryScreen();
    if (screen) setGeometry(screen->geometry());

    setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    m_zoneRect = QRect(QPoint(400, 500), QSize(120, 40));

    loadSettings();

    connect(&feedbackTimer, &QTimer::timeout, this,
            [this]() { feedbackText.clear(); update(); });
}

void ResonanceGateCaptureSetup::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.setBrush(QColor(0, 0, 255, 230));
    p.setPen(QPen(QColor(0, 0, 255), 2));
    p.drawRect(m_zoneRect);

    const int s = RESIZE_MARGIN;
    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawRect(QRect(m_zoneRect.left(), m_zoneRect.top(), s, s));
    p.drawRect(QRect(m_zoneRect.right() - s + 1, m_zoneRect.top(), s, s));
    p.drawRect(QRect(m_zoneRect.left(), m_zoneRect.bottom() - s + 1, s, s));
    p.drawRect(QRect(m_zoneRect.right() - s + 1, m_zoneRect.bottom() - s + 1, s, s));

    if (!feedbackText.isEmpty())
    {
        QFont font; font.setBold(true); font.setPointSize(18);
        p.setFont(font); p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, feedbackText);
    }
}

ResonanceGateCaptureSetup::ResizeCorner
ResonanceGateCaptureSetup::cornerAt(const QRect &zone, const QPoint &pos) const
{
    const int m = RESIZE_MARGIN;
    const bool nearLeft = qAbs(pos.x() - zone.left()) <= m;
    const bool nearRight = qAbs(pos.x() - zone.right()) <= m;
    const bool nearTop = qAbs(pos.y() - zone.top()) <= m;
    const bool nearBottom = qAbs(pos.y() - zone.bottom()) <= m;

    if (nearLeft && nearTop) return ResizeCorner::TopLeft;
    if (nearRight && nearTop) return ResizeCorner::TopRight;
    if (nearLeft && nearBottom) return ResizeCorner::BottomLeft;
    if (nearRight && nearBottom) return ResizeCorner::BottomRight;
    return ResizeCorner::None;
}

void ResonanceGateCaptureSetup::mousePressEvent(QMouseEvent *event)
{
    const QPoint pos = event->position().toPoint();
    m_lastMousePosition = pos;

    const ResizeCorner c = cornerAt(m_zoneRect, pos);
    if (c != ResizeCorner::None)
    {
        m_dragMode = DragMode::Resize;
        m_dragCorner = c;
        return;
    }

    m_dragMode = m_zoneRect.contains(pos) ? DragMode::Move : DragMode::None;
}

void ResonanceGateCaptureSetup::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragMode == DragMode::None) return;

    const QPoint current = event->position().toPoint();
    const QPoint delta = current - m_lastMousePosition;

    if (m_dragMode == DragMode::Move)
    {
        m_zoneRect.translate(delta);
        keepInsideScreen(m_zoneRect);
    }
    else
    {
        applyCornerResize(m_zoneRect, m_dragCorner, delta);
        keepInsideScreen(m_zoneRect);
    }

    m_lastMousePosition = current;
    update();
}

void ResonanceGateCaptureSetup::mouseReleaseEvent(QMouseEvent *)
{
    m_dragMode = DragMode::None;
    m_dragCorner = ResizeCorner::None;
}

void ResonanceGateCaptureSetup::applyCornerResize(
    QRect &zone, ResizeCorner corner, const QPoint &delta
    )
{
    int left = zone.left(), top = zone.top(), right = zone.right(), bottom = zone.bottom();

    switch (corner)
    {
    case ResizeCorner::TopLeft: left += delta.x(); top += delta.y(); break;
    case ResizeCorner::TopRight: right += delta.x(); top += delta.y(); break;
    case ResizeCorner::BottomLeft: left += delta.x(); bottom += delta.y(); break;
    case ResizeCorner::BottomRight: right += delta.x(); bottom += delta.y(); break;
    default: return;
    }

    if (right - left < MIN_ZONE_SIZE)
    {
        if (corner == ResizeCorner::TopLeft || corner == ResizeCorner::BottomLeft)
            left = right - MIN_ZONE_SIZE;
        else
            right = left + MIN_ZONE_SIZE;
    }

    if (bottom - top < MIN_ZONE_SIZE)
    {
        if (corner == ResizeCorner::TopLeft || corner == ResizeCorner::TopRight)
            top = bottom - MIN_ZONE_SIZE;
        else
            bottom = top + MIN_ZONE_SIZE;
    }

    zone.setCoords(left, top, right, bottom);
}

void ResonanceGateCaptureSetup::keepInsideScreen(QRect &zone)
{
    const QRect screenRect = rect();
    if (zone.left() < screenRect.left()) zone.moveLeft(screenRect.left());
    if (zone.top() < screenRect.top()) zone.moveTop(screenRect.top());
    if (zone.right() > screenRect.right()) zone.moveRight(screenRect.right());
    if (zone.bottom() > screenRect.bottom()) zone.moveBottom(screenRect.bottom());
}

void ResonanceGateCaptureSetup::loadSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/ElsOverlay.ini", QSettings::IniFormat);
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return;

    const QSize resolution = screen->size();
    const QString suffix = QString("_%1x%2").arg(resolution.width()).arg(resolution.height());

    m_zoneRect.setWidth(settings.value("ResonanceGate/W" + suffix, m_zoneRect.width()).toInt());
    m_zoneRect.setHeight(settings.value("ResonanceGate/H" + suffix, m_zoneRect.height()).toInt());
    m_zoneRect.moveTo(
        settings.value("ResonanceGate/X" + suffix, m_zoneRect.x()).toInt(),
        settings.value("ResonanceGate/Y" + suffix, m_zoneRect.y()).toInt()
        );

    keepInsideScreen(m_zoneRect);
}

void ResonanceGateCaptureSetup::saveSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/ElsOverlay.ini", QSettings::IniFormat);
    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return;

    const QSize resolution = screen->size();
    const QString suffix = QString("_%1x%2").arg(resolution.width()).arg(resolution.height());

    settings.setValue("ResonanceGate/X" + suffix, m_zoneRect.x());
    settings.setValue("ResonanceGate/Y" + suffix, m_zoneRect.y());
    settings.setValue("ResonanceGate/W" + suffix, m_zoneRect.width());
    settings.setValue("ResonanceGate/H" + suffix, m_zoneRect.height());
    settings.sync();
}

QRect ResonanceGateCaptureSetup::gateZoneRect() const { return m_zoneRect; }

void ResonanceGateCaptureSetup::captureReference()
{
    hide();

    QTimer::singleShot(120, this, [this]()
                       {
                           QScreen *screen = QApplication::primaryScreen();
                           if (!screen)
                           {
                               show();
                               showFeedback("ERRORE: schermo non disponibile");
                               return;
                           }

                           QDir dir(QCoreApplication::applicationDirPath() + "/images");
                           if (!dir.exists()) dir.mkpath(".");

                           const QImage img = ScreenCapture::captureRegionReliable(screen, m_zoneRect);
                           const bool ok = !img.isNull() && img.save(dir.filePath("invariant.png"));

                           show(); raise(); activateWindow(); setFocus();
                           showFeedback(ok ? "RIFERIMENTO SALVATO" : "ERRORE: cattura fallita");
                       });
}

bool ResonanceGateCaptureSetup::referenceExists()
{
    return QFile::exists(QCoreApplication::applicationDirPath() + "/images/invariant.png");
}

void ResonanceGateCaptureSetup::showFeedback(const QString &text)
{
    feedbackText = text;
    feedbackTimer.start(1500);
    update();
}

void ResonanceGateCaptureSetup::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) { hide(); event->accept(); return; }
    QWidget::keyPressEvent(event);
}