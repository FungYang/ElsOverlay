#include "atmazonecapturesetup.h"
#include "screencapture.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTimer>



AtmaZoneCaptureSetup::AtmaZoneCaptureSetup(QWidget *parent)
    : QWidget(parent)
{
    QScreen *screen = QApplication::primaryScreen();
    if (screen)
        setGeometry(screen->geometry());

    setFocusPolicy(Qt::StrongFocus);

    setWindowFlags(
        Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
        );

    setAttribute(Qt::WA_TranslucentBackground);

    const QSize size = defaultZoneSize();

    // Posizioni iniziali di default, sfalsate per non sovrapporsi.
    for (int i = 0; i < RED_COUNT; ++i)
        m_redRects[i] = QRect(QPoint(400 + i * (size.width() + 20), 300), size);

    m_blueRect = QRect(QPoint(400, 500), size);

    loadSettings();

    connect(&feedbackTimer, &QTimer::timeout, this,
            [this]() { feedbackText.clear(); update(); });
}

QSize AtmaZoneCaptureSetup::defaultZoneSize() const
{
    return QSize(60, 60); // dimensione di partenza ragionevole, l'utente la adatta
}

// =========================================================
// PAINT
// =========================================================


void AtmaZoneCaptureSetup::drawHandles(QPainter &p, const QRect &zone)
{
    const int s = RESIZE_MARGIN;

    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);

    p.drawRect(QRect(zone.left(),  zone.top(),    s, s));               // TopLeft
    p.drawRect(QRect(zone.right() - s + 1, zone.top(),    s, s));       // TopRight
    p.drawRect(QRect(zone.left(),  zone.bottom() - s + 1, s, s));       // BottomLeft
    p.drawRect(QRect(zone.right() - s + 1, zone.bottom() - s + 1, s, s)); // BottomRight
}

void AtmaZoneCaptureSetup::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < RED_COUNT; ++i)
    {
        p.setBrush(QColor(255, 0, 0, 230));
        p.setPen(QPen(QColor(255, 0, 0), 2));
        p.drawRect(m_redRects[i]);

        p.setPen(Qt::white);
        p.drawText(m_redRects[i], Qt::AlignCenter, QString::number(i + 1));

        drawHandles(p, m_redRects[i]);
    }

    p.setBrush(QColor(0, 0, 255, 230));
    p.setPen(QPen(QColor(0, 0, 255), 2));
    p.drawRect(m_blueRect);

    drawHandles(p, m_blueRect);

    if (!feedbackText.isEmpty())
    {
        QFont font;
        font.setBold(true);
        font.setPointSize(18);
        p.setFont(font);
        p.setPen(Qt::white);
        p.drawText(rect(), Qt::AlignCenter, feedbackText);
    }
}

// =========================================================
// CORNER DETECTION
// =========================================================

AtmaZoneCaptureSetup::ResizeCorner
AtmaZoneCaptureSetup::cornerAt(const QRect &zone, const QPoint &pos) const
{
    const int m = RESIZE_MARGIN;

    const bool nearLeft   = qAbs(pos.x() - zone.left())   <= m;
    const bool nearRight  = qAbs(pos.x() - zone.right())  <= m;
    const bool nearTop    = qAbs(pos.y() - zone.top())    <= m;
    const bool nearBottom = qAbs(pos.y() - zone.bottom()) <= m;

    if (nearLeft  && nearTop)    return ResizeCorner::TopLeft;
    if (nearRight && nearTop)    return ResizeCorner::TopRight;
    if (nearLeft  && nearBottom) return ResizeCorner::BottomLeft;
    if (nearRight && nearBottom) return ResizeCorner::BottomRight;

    return ResizeCorner::None;
}


// =========================================================
// MOUSE
// =========================================================

void AtmaZoneCaptureSetup::mousePressEvent(QMouseEvent *event)
{
    const QPoint pos = event->position().toPoint();
    m_lastMousePosition = pos;
    m_dragMode = DragMode::None;
    m_dragRedIndex = -1;
    m_dragCorner = ResizeCorner::None;

    // Prima gli angoli di resize (rossi)
    for (int i = 0; i < RED_COUNT; ++i)
    {
        const ResizeCorner c = cornerAt(m_redRects[i], pos);
        if (c != ResizeCorner::None)
        {
            m_dragMode = DragMode::ResizeRed;
            m_dragCorner = c;
            return;
        }
    }

    // Poi l'angolo del blu
    {
        const ResizeCorner c = cornerAt(m_blueRect, pos);
        if (c != ResizeCorner::None)
        {
            m_dragMode = DragMode::ResizeBlue;
            m_dragCorner = c;
            return;
        }
    }

    // Poi il corpo (spostamento)
    for (int i = 0; i < RED_COUNT; ++i)
    {
        if (m_redRects[i].contains(pos))
        {
            m_dragMode = DragMode::MoveRed;
            m_dragRedIndex = i;
            return;
        }
    }

    if (m_blueRect.contains(pos))
        m_dragMode = DragMode::MoveBlue;
}

// =========================================================
// MOUSE MOVE
// =========================================================

void AtmaZoneCaptureSetup::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragMode == DragMode::None)
        return;

    const QPoint current = event->position().toPoint();
    const QPoint delta = current - m_lastMousePosition;

    switch (m_dragMode)
    {
    case DragMode::MoveRed:
        if (m_dragRedIndex >= 0)
        {
            m_redRects[m_dragRedIndex].translate(delta);
            keepInsideScreen(m_redRects[m_dragRedIndex]);
        }
        break;

    case DragMode::MoveBlue:
        m_blueRect.translate(delta);
        keepInsideScreen(m_blueRect);
        break;

    case DragMode::ResizeRed:
        // Stesso angolo, stesso delta applicato a TUTTI i rossi:
        // ciascuno si ridimensiona identicamente, ancorato al
        // proprio angolo opposto (posizione individuale preservata).
        resizeAllRed(m_dragCorner, delta);
        break;

    case DragMode::ResizeBlue:
        applyCornerResize(m_blueRect, m_dragCorner, delta);
        keepInsideScreen(m_blueRect);
        break;

    default:
        break;
    }

    m_lastMousePosition = current;
    update();
}

void AtmaZoneCaptureSetup::mouseReleaseEvent(QMouseEvent *)
{
    m_dragMode = DragMode::None;
    m_dragRedIndex = -1;
    m_dragCorner = ResizeCorner::None;
}

// =========================================================
// RESIZE DA ANGOLO
// =========================================================

void AtmaZoneCaptureSetup::applyCornerResize(
    QRect &zone, ResizeCorner corner, const QPoint &delta
    )
{
    int left = zone.left();
    int top = zone.top();
    int right = zone.right();
    int bottom = zone.bottom();

    switch (corner)
    {
    case ResizeCorner::TopLeft:
        left += delta.x();
        top += delta.y();
        break;

    case ResizeCorner::TopRight:
        right += delta.x();
        top += delta.y();
        break;

    case ResizeCorner::BottomLeft:
        left += delta.x();
        bottom += delta.y();
        break;

    case ResizeCorner::BottomRight:
        right += delta.x();
        bottom += delta.y();
        break;

    default:
        return;
    }

    // Clamp dimensione minima, mantenendo l'angolo OPPOSTO a quello
    // trascinato come punto fisso.
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

void AtmaZoneCaptureSetup::resizeAllRed(ResizeCorner corner, const QPoint &delta)
{
    for (int i = 0; i < RED_COUNT; ++i)
    {
        applyCornerResize(m_redRects[i], corner, delta);
        keepInsideScreen(m_redRects[i]);
    }
}

void AtmaZoneCaptureSetup::keepInsideScreen(QRect &zone)
{
    const QRect screenRect = rect();

    if (zone.left() < screenRect.left()) zone.moveLeft(screenRect.left());
    if (zone.top() < screenRect.top()) zone.moveTop(screenRect.top());
    if (zone.right() > screenRect.right()) zone.moveRight(screenRect.right());
    if (zone.bottom() > screenRect.bottom()) zone.moveBottom(screenRect.bottom());
}

// =========================================================
// SETTINGS
// =========================================================

void AtmaZoneCaptureSetup::loadSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return;

    const QSize resolution = screen->size();
    const QString suffix =
        QString("_%1x%2").arg(resolution.width()).arg(resolution.height());

    // Dimensione condivisa dai 6 rossi: la leggiamo una volta
    // (associata al primo rosso) e la applichiamo a tutti.
    const int sharedW = settings.value("AtmaZones/RedW" + suffix, defaultZoneSize().width()).toInt();
    const int sharedH = settings.value("AtmaZones/RedH" + suffix, defaultZoneSize().height()).toInt();

    for (int i = 0; i < RED_COUNT; ++i)
    {
        const QString key = QString("AtmaZones/Red%1").arg(i + 1);

        m_redRects[i].setSize(QSize(sharedW, sharedH));
        m_redRects[i].moveTo(
            settings.value(key + "X" + suffix, m_redRects[i].x()).toInt(),
            settings.value(key + "Y" + suffix, m_redRects[i].y()).toInt()
            );

        keepInsideScreen(m_redRects[i]);
    }

    m_blueRect.setWidth(settings.value("AtmaZones/BlueW" + suffix, m_blueRect.width()).toInt());
    m_blueRect.setHeight(settings.value("AtmaZones/BlueH" + suffix, m_blueRect.height()).toInt());
    m_blueRect.moveTo(
        settings.value("AtmaZones/BlueX" + suffix, m_blueRect.x()).toInt(),
        settings.value("AtmaZones/BlueY" + suffix, m_blueRect.y()).toInt()
        );

    keepInsideScreen(m_blueRect);
}

void AtmaZoneCaptureSetup::saveSettings()
{
    QSettings settings(
        QCoreApplication::applicationDirPath() + "/ElsOverlay.ini",
        QSettings::IniFormat
        );

    QScreen *screen = QApplication::primaryScreen();
    if (!screen) return;

    const QSize resolution = screen->size();
    const QString suffix =
        QString("_%1x%2").arg(resolution.width()).arg(resolution.height());

    // Dimensione condivisa: salviamo quella del primo rosso,
    // sono comunque tutte uguali per costruzione (resize agganciato).
    settings.setValue("AtmaZones/RedW" + suffix, m_redRects[0].width());
    settings.setValue("AtmaZones/RedH" + suffix, m_redRects[0].height());

    for (int i = 0; i < RED_COUNT; ++i)
    {
        const QString key = QString("AtmaZones/Red%1").arg(i + 1);
        settings.setValue(key + "X" + suffix, m_redRects[i].x());
        settings.setValue(key + "Y" + suffix, m_redRects[i].y());
    }

    settings.setValue("AtmaZones/BlueX" + suffix, m_blueRect.x());
    settings.setValue("AtmaZones/BlueY" + suffix, m_blueRect.y());
    settings.setValue("AtmaZones/BlueW" + suffix, m_blueRect.width());
    settings.setValue("AtmaZones/BlueH" + suffix, m_blueRect.height());

    settings.sync();
}

// =========================================================
// ACCESSORS
// =========================================================

QRect AtmaZoneCaptureSetup::redZoneRect(int index) const
{
    if (index < 0 || index >= RED_COUNT) return QRect();
    return m_redRects[index];
}

QRect AtmaZoneCaptureSetup::blueZoneRect() const
{
    return m_blueRect;
}

// =========================================================
// CAPTURE REFERENCES (tasto P)
// =========================================================

void AtmaZoneCaptureSetup::captureAllReferences()
{
    // Nascondiamo i rettangoli prima dello screenshot, altrimenti
    // il rosso/blu dell'overlay finirebbe dentro l'immagine salvata.
    hide();

    QTimer::singleShot(
        120, this,
        [this]()
        {
            QScreen *screen = QApplication::primaryScreen();
            if (!screen)
            {
                show();
                showFeedback("ERRORE: schermo non disponibile");
                return;
            }

            QDir dir(QCoreApplication::applicationDirPath() + "/images");
            if (!dir.exists())
                dir.mkpath(".");

            bool allOk = true;

            for (int i = 0; i < RED_COUNT; ++i)
            {
                const QImage img =
                    ScreenCapture::captureRegionReliable(screen, m_redRects[i]);

                if (img.isNull() || !img.save(dir.filePath(QString("ref%1.png").arg(i + 1))))
                    allOk = false;
            }

            const QImage blueImg =
                ScreenCapture::captureRegionReliable(screen, m_blueRect);

            if (blueImg.isNull() || !blueImg.save(dir.filePath("invariant.png")))
                allOk = false;

            show();
            raise();
            activateWindow();
            setFocus();

            showFeedback(allOk ? "7 REFERENCE SALVATE" : "ERRORE: cattura fallita");
        }
        );
}

bool AtmaZoneCaptureSetup::referencesExist()
{
    const QString dir = QCoreApplication::applicationDirPath() + "/images/";

    for (int i = 1; i <= RED_COUNT; ++i)
        if (!QFile::exists(dir + QString("ref%1.png").arg(i)))
            return false;

    return QFile::exists(dir + "invariant.png");
}

// =========================================================
// FEEDBACK / KEYBOARD
// =========================================================

void AtmaZoneCaptureSetup::showFeedback(const QString &text)
{
    feedbackText = text;
    feedbackTimer.start(1500);
    update();
}

void AtmaZoneCaptureSetup::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        hide();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}