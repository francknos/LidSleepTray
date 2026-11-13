#include "SettingsPopup.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

#include "ToggleSwitch.h"

static const char *kStyle = R"(
#card { background: #1b1f2a; border: 1px solid #2c3347; border-radius: 16px; }
QLabel { color: #e6e9f2; font-family: "Segoe UI Variable", "Segoe UI", sans-serif; font-size: 13px; }
#title { font-size: 15px; font-weight: 600; }
#credit { color: #8b93a7; font-size: 12px; font-weight: 300; }
#muted { color: #8b93a7; font-size: 12px; }
#sectionTitle { font-weight: 600; }
#badge { background: #1f3a2e; color: #4ade80; border-radius: 8px; padding: 1px 8px; font-size: 11px; }
#status { color: #f59e0b; font-size: 11px; }
QFrame#section { background: #232938; border-radius: 12px; }
QPushButton { font-family: "Segoe UI Variable", "Segoe UI", sans-serif; font-size: 13px; border: none; }
QPushButton#seg { background: #2b3245; color: #aeb6ca; padding: 7px 10px; }
QPushButton#seg:hover { background: #343c53; }
QPushButton#seg:checked { background: #3b82f6; color: white; font-weight: 600; }
QPushButton#segLeft  { border-top-left-radius: 8px; border-bottom-left-radius: 8px; }
QPushButton#segRight { border-top-right-radius: 8px; border-bottom-right-radius: 8px; }
QPushButton#toggle { background: #2b3245; color: #e6e9f2; border-radius: 12px; padding: 12px; text-align: left; font-size: 14px; font-weight: 600; }
QPushButton#toggle:hover { background: #343c53; }
QPushButton#toggle:checked { background: #16a34a; color: white; }
QPushButton#link { background: transparent; color: #8b93a7; text-align: left; padding: 4px 2px; font-size: 12px; }
QPushButton#link:hover { color: #e6e9f2; }
QPushButton#quit { background: #2b3245; color: #fca5a5; border-radius: 10px; padding: 8px; }
QPushButton#quit:hover { background: #3b2a30; }
)";

// Power-mode icons drawn with QPainter (24x24 design grid): no resource files, crisp at any DPI.
// kind: 0 = leaf (economy), 1 = half-filled circle (balanced), 2 = bolt (performance)
static QPixmap drawModeIcon(int kind, const QColor &color, int size, qreal dpr)
{
    QPixmap pm(QSize(size, size) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(size / 24.0, size / 24.0);
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    if (kind == 0) {
        QPainterPath leaf;
        leaf.moveTo(4, 20);
        leaf.cubicTo(3, 10, 10, 4, 20, 4);
        leaf.cubicTo(21, 14, 14, 21, 4, 20);
        p.drawPath(leaf);
        p.setCompositionMode(QPainter::CompositionMode_Clear); // vein
        QPen vein(Qt::black, 1.6, Qt::SolidLine, Qt::RoundCap);
        p.setPen(vein);
        p.drawLine(QPointF(6.5, 17.5), QPointF(14, 10));
    } else if (kind == 1) {
        p.setPen(QPen(color, 2.0));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(12, 12), 8.5, 8.5);
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        QPainterPath half;
        half.moveTo(12, 3.5);
        half.arcTo(QRectF(3.5, 3.5, 17, 17), 90, 180);
        half.closeSubpath();
        p.drawPath(half);
    } else {
        QPainterPath bolt;
        bolt.moveTo(14, 2);
        bolt.lineTo(5, 13);
        bolt.lineTo(11, 13);
        bolt.lineTo(10, 22);
        bolt.lineTo(19, 10);
        bolt.lineTo(13, 10);
        bolt.closeSubpath();
        p.drawPath(bolt);
    }
    return pm;
}

static QIcon modeIcon(int kind)
{
    QIcon ic;
    const qreal dpr = 3; // high-res source, Qt downsamples for 100-300% scaling
    ic.addPixmap(drawModeIcon(kind, QColor("#aeb6ca"), 18, dpr), QIcon::Normal, QIcon::Off);
    ic.addPixmap(drawModeIcon(kind, QColor("#ffffff"), 18, dpr), QIcon::Normal, QIcon::On);
    return ic;
}

SettingsPopup::SettingsPopup(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet(kStyle);
    setFixedWidth(330);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(14, 14, 14, 14); // room for the shadow

    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 140));
    card->setGraphicsEffect(shadow);
    outer->addWidget(card);

    auto *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 16, 16, 14);
    lay->setSpacing(10);

    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);
    auto *title = new QLabel(tr("LidSleepTray"), card);
    title->setObjectName("title");
    titleRow->addWidget(title, 0, Qt::AlignBaseline);
    auto *credit = new QLabel(tr("design by Franck"), card);
    credit->setObjectName("credit");
    titleRow->addWidget(credit, 0, Qt::AlignBaseline);
    titleRow->addStretch();
    lay->addLayout(titleRow);

    m_toggle = new QPushButton(card);
    m_toggle->setObjectName("toggle");
    m_toggle->setCheckable(true);
    m_toggle->setCursor(Qt::PointingHandCursor);
    lay->addWidget(m_toggle);
    m_toggleHint = new QLabel(tr("Astuce : un clic gauche sur l'icône bascule aussi."), card);
    m_toggleHint->setObjectName("muted");
    m_toggleHint->setWordWrap(true);
    lay->addWidget(m_toggleHint);

    m_modeFrame = new QFrame(card);
    m_modeFrame->setObjectName("section");
    auto *mv = new QVBoxLayout(m_modeFrame);
    mv->setContentsMargins(12, 10, 12, 12);
    mv->setSpacing(8);
    auto *mhead = new QHBoxLayout;
    auto *mt = new QLabel(tr("⚡  Mode d'alimentation"), m_modeFrame);
    mt->setObjectName("sectionTitle");
    m_modeSource = new QLabel(m_modeFrame);
    m_modeSource->setObjectName("badge");
    mhead->addWidget(mt);
    mhead->addStretch();
    mhead->addWidget(m_modeSource);
    mv->addLayout(mhead);
    auto *mseg = new QHBoxLayout;
    mseg->setSpacing(1);
    const QString labels[3] = {tr("Économie"), tr("Équilibré"), tr("Performance")};
    const char *radii[3] = {"border-top-left-radius:8px;border-bottom-left-radius:8px;", "",
                            "border-top-right-radius:8px;border-bottom-right-radius:8px;"};
    for (int i = 0; i < 3; ++i) {
        m_modeBtn[i] = new QPushButton(labels[i], m_modeFrame);
        m_modeBtn[i]->setObjectName("seg");
        m_modeBtn[i]->setCheckable(true);
        m_modeBtn[i]->setIcon(modeIcon(i));
        m_modeBtn[i]->setIconSize(QSize(18, 18));
        m_modeBtn[i]->setCursor(Qt::PointingHandCursor);
        m_modeBtn[i]->setStyleSheet(radii[i]);
        mseg->addWidget(m_modeBtn[i]);
        connect(m_modeBtn[i], &QPushButton::clicked, this, [this, i]() {
            setPowerMode(i);
            emit powerModeChanged(i);
        });
    }
    mv->addLayout(mseg);
    lay->addWidget(m_modeFrame);

    auto *capot = new QLabel(tr("À la fermeture du capot"), card);
    capot->setObjectName("muted");
    lay->addSpacing(4);
    lay->addWidget(capot);

    m_ac = makeSection(lay, tr("🔌  Sur secteur"), true);
    m_dc = makeSection(lay, tr("🔋  Sur batterie"), false);

    m_status = new QLabel(card);
    m_status->setObjectName("status");
    m_status->setWordWrap(true);
    m_status->hide();
    lay->addWidget(m_status);

    auto *autoRow = new QHBoxLayout;
    autoRow->setContentsMargins(2, 4, 2, 4);
    auto *autoLabel = new QLabel(tr("Lancer au démarrage de Windows"), card);
    autoRow->addWidget(autoLabel, 1);
    m_autostart = new ToggleSwitch(card);
    autoRow->addWidget(m_autostart);
    lay->addLayout(autoRow);

    auto *restore = new QPushButton(tr("Rétablir les réglages Windows d'origine"), card);
    restore->setObjectName("link");
    restore->setCursor(Qt::PointingHandCursor);
    lay->addWidget(restore);

    auto *quit = new QPushButton(tr("Quitter"), card);
    quit->setObjectName("quit");
    quit->setCursor(Qt::PointingHandCursor);
    lay->addWidget(quit);

    connect(m_toggle, &QPushButton::clicked, this, [this](bool on) {
        setBlocked(on);
        emit sleepToggled(on);
    });
    connect(m_autostart, &QAbstractButton::clicked, this, [this](bool on) {
        setAutostart(on);
        emit autostartToggled(on);
    });
    connect(restore, &QPushButton::clicked, this, &SettingsPopup::restoreRequested);
    connect(quit, &QPushButton::clicked, this, &SettingsPopup::quitRequested);

    setBlocked(false);
    setAutostart(false);
}

SettingsPopup::Section SettingsPopup::makeSection(QVBoxLayout *parent, const QString &title, bool ac)
{
    Section s;
    auto *frame = new QFrame;
    frame->setObjectName("section");
    auto *v = new QVBoxLayout(frame);
    v->setContentsMargins(12, 10, 12, 12);
    v->setSpacing(8);

    auto *head = new QHBoxLayout;
    auto *t = new QLabel(title, frame);
    t->setObjectName("sectionTitle");
    s.badge = new QLabel(tr("Actuel"), frame);
    s.badge->setObjectName("badge");
    s.badge->hide();
    head->addWidget(t);
    head->addStretch();
    head->addWidget(s.badge);
    v->addLayout(head);

    auto *seg = new QHBoxLayout;
    seg->setSpacing(1);
    s.nothing = new QPushButton(tr("Ne rien faire"), frame);
    s.sleep = new QPushButton(tr("Veille"), frame);
    s.nothing->setObjectName("seg");
    s.sleep->setObjectName("seg");
    s.nothing->setProperty("class", "left");
    for (auto *b : {s.nothing, s.sleep}) {
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
    }
    // Rounded outer corners only (object-name based style can't do per-position classes).
    s.nothing->setStyleSheet("border-top-left-radius:8px;border-bottom-left-radius:8px;");
    s.sleep->setStyleSheet("border-top-right-radius:8px;border-bottom-right-radius:8px;");
    seg->addWidget(s.nothing);
    seg->addWidget(s.sleep);
    v->addLayout(seg);
    parent->addWidget(frame);

    connect(s.nothing, &QPushButton::clicked, this, [this, ac]() {
        setLidAction(ac, 0);
        emit lidActionChanged(ac, 0);
    });
    connect(s.sleep, &QPushButton::clicked, this, [this, ac]() {
        setLidAction(ac, 1);
        emit lidActionChanged(ac, 1);
    });
    return s;
}

void SettingsPopup::setBlocked(bool blocked)
{
    m_toggle->setChecked(blocked);
    m_toggle->setText(blocked ? tr("☀️  Veille bloquée") : tr("🌙  Veille autorisée"));
}

void SettingsPopup::setOnAc(bool onAc)
{
    m_modeSource->setText(onAc ? tr("sur secteur") : tr("sur batterie"));
    m_ac.badge->setVisible(onAc);
    m_dc.badge->setVisible(!onAc);
}

void SettingsPopup::setLidAction(bool ac, int action)
{
    Section &s = ac ? m_ac : m_dc;
    s.nothing->setChecked(action == 0);
    s.sleep->setChecked(action != 0);
}

void SettingsPopup::setPowerMode(int mode)
{
    m_modeFrame->setVisible(mode >= 0);
    for (int i = 0; i < 3; ++i)
        m_modeBtn[i]->setChecked(i == mode); // mode 3 (max) leaves all unchecked
}

void SettingsPopup::setAutostart(bool enabled)
{
    m_autostart->setChecked(enabled);
}

void SettingsPopup::setStatus(const QString &text)
{
    m_status->setText(text);
    m_status->setVisible(!text.isEmpty());
}

void SettingsPopup::showNearTray()
{
    adjustSize();
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    const QRect avail = screen->availableGeometry();
    move(avail.right() - width() - 6, avail.bottom() - height() - 6);
    show();
    raise();
    activateWindow();
}

bool SettingsPopup::event(QEvent *e)
{
    if (e->type() == QEvent::WindowDeactivate)
        hide(); // behave like a flyout: click elsewhere to close
    return QWidget::event(e);
}

void SettingsPopup::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
        hide();
    else
        QWidget::keyPressEvent(e);
}
