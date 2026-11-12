#include "TrayApp.h"

#include <QCoreApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include "SettingsPopup.h"

TrayApp::TrayApp(QObject *parent)
    : QObject(parent), m_settings(QSettings::NativeFormat, QSettings::UserScope, "LidSleepTray", "LidSleepTray")
{
    // First run: remember what Windows had, so it can be restored later, and start from it.
    if (!m_settings.contains("origAc")) {
        m_settings.setValue("origAc", m_power.lidActionRaw(true));
        m_settings.setValue("origDc", m_power.lidActionRaw(false));
        const int ac = m_power.lidActionRaw(true);
        const int dc = m_power.lidActionRaw(false);
        m_settings.setValue("acAction", ac == 0 ? 0 : 1);
        m_settings.setValue("dcAction", dc == 0 ? 0 : 1);
    }
    m_acAction = m_settings.value("acAction", 0).toInt();
    m_dcAction = m_settings.value("dcAction", 1).toInt();
    m_onAc = m_power.isOnAcPower();

    m_popup = new SettingsPopup;
    connect(m_popup, &SettingsPopup::sleepToggled, this, &TrayApp::setBlocked);
    connect(m_popup, &SettingsPopup::lidActionChanged, this, [this](bool ac, int action) {
        (ac ? m_acAction : m_dcAction) = action;
        m_settings.setValue(ac ? "acAction" : "dcAction", action);
        applyConfig();
        refreshUi();
    });
    connect(m_popup, &SettingsPopup::powerModeChanged, this, [this](int mode) {
        if (!m_power.setPowerMode(mode))
            m_popup->setStatus(tr("Windows a refusé le changement de mode d'alimentation."));
        else
            m_popup->setStatus(QString());
        m_popup->setPowerMode(m_power.powerMode());
    });
    connect(m_popup, &SettingsPopup::autostartToggled, this, [this](bool on) {
        if (!PowerManager::setAutostart(on)) {
            m_popup->setStatus(tr("Impossible de modifier le démarrage automatique."));
            m_popup->setAutostart(PowerManager::autostartEnabled());
        }
    });
    connect(m_popup, &SettingsPopup::restoreRequested, this, &TrayApp::restoreOriginal);
    connect(m_popup, &SettingsPopup::quitRequested, qApp, &QCoreApplication::quit);

    m_tray = new QSystemTrayIcon(this);
    connect(m_tray, &QSystemTrayIcon::activated, this, &TrayApp::onActivated);

    m_popup->setAutostart(PowerManager::autostartEnabled());
    applyConfig();
    restoreSleepState();
    refreshUi();
    m_tray->show();

    // Detect AC <-> battery switches and swap configuration accordingly.
    m_timer.setInterval(2000);
    connect(&m_timer, &QTimer::timeout, this, &TrayApp::pollPower);
    m_timer.start();
}

void TrayApp::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        pollPower();
        setBlocked(!m_blocked);
    } else if (reason == QSystemTrayIcon::Context) {
        pollPower();
        m_popup->setPowerMode(m_power.powerMode()); // may have been changed from Windows Settings
        m_popup->showNearTray();
    }
}

void TrayApp::setBlocked(bool blocked)
{
    pollPower();
    m_blocked = blocked;
    m_power.setSleepBlocked(blocked);
    m_settings.setValue(m_onAc ? "acSleepBlocked" : "dcSleepBlocked", blocked);
    m_settings.sync();
    refreshUi();
    if (m_settings.status() != QSettings::NoError)
        m_popup->setStatus(tr("Impossible de sauvegarder l'état du blocage de la veille."));
}

void TrayApp::restoreSleepState()
{
    m_blocked = m_settings.value(m_onAc ? "acSleepBlocked" : "dcSleepBlocked", false).toBool();
    m_power.setSleepBlocked(m_blocked);
}

void TrayApp::pollPower()
{
    const bool onAc = m_power.isOnAcPower();
    if (onAc != m_onAc) {
        m_onAc = onAc;
        applyConfig();
        restoreSleepState();
        refreshUi();
    }
}

// Write both profiles into the active power plan: Windows then switches on its own
// between them, and we re-apply on every source change in case the plan was altered.
void TrayApp::applyConfig()
{
    bool ok = true;
    if (m_power.lidActionRaw(true) != m_acAction)
        ok &= m_power.setLidActionRaw(true, m_acAction);
    if (m_power.lidActionRaw(false) != m_dcAction)
        ok &= m_power.setLidActionRaw(false, m_dcAction);
    m_popup->setStatus(ok ? QString() : tr("Windows a refusé la modification du plan d'alimentation."));
}

void TrayApp::restoreOriginal()
{
    const int ac = m_settings.value("origAc", -1).toInt();
    const int dc = m_settings.value("origDc", -1).toInt();
    bool ok = true;
    if (ac >= 0)
        ok &= m_power.setLidActionRaw(true, ac);
    if (dc >= 0)
        ok &= m_power.setLidActionRaw(false, dc);
    if (ac >= 0) {
        m_acAction = ac == 0 ? 0 : 1;
        m_settings.setValue("acAction", m_acAction);
    }
    if (dc >= 0) {
        m_dcAction = dc == 0 ? 0 : 1;
        m_settings.setValue("dcAction", m_dcAction);
    }
    refreshUi();
    m_popup->setStatus(ok ? tr("Réglages Windows d'origine rétablis.") : tr("Échec du rétablissement."));
}

void TrayApp::refreshUi()
{
    m_popup->setBlocked(m_blocked);
    m_popup->setOnAc(m_onAc);
    m_popup->setPowerMode(m_power.powerMode());
    m_popup->setLidAction(true, m_acAction);
    m_popup->setLidAction(false, m_dcAction);
    m_tray->setIcon(makeIcon(m_blocked));

    const int cur = m_onAc ? m_acAction : m_dcAction;
    m_tray->setToolTip(tr("LidSleepTray — veille %1\nCapot (%2) : %3")
                           .arg(m_blocked ? tr("bloquée") : tr("autorisée"))
                           .arg(m_onAc ? tr("secteur") : tr("batterie"))
                           .arg(cur == 0 ? tr("ne rien faire") : tr("veille")));
}

QIcon TrayApp::makeIcon(bool active)
{
    QPixmap pm(128, 128);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(active ? "#16a34a" : "#4b5263"));
    p.drawEllipse(QRectF(6, 6, 116, 116));

    p.setBrush(Qt::white);
    if (active) { // sun: awake
        p.drawEllipse(QPointF(64, 64), 22, 22);
        QPen pen(Qt::white, 8, Qt::SolidLine, Qt::RoundCap);
        p.setPen(pen);
        for (int i = 0; i < 8; ++i) {
            p.save();
            p.translate(64, 64);
            p.rotate(i * 45);
            p.drawLine(QPointF(0, -34), QPointF(0, -44));
            p.restore();
        }
    } else { // crescent moon: sleep allowed
        QPainterPath moon;
        moon.addEllipse(QPointF(62, 64), 34, 34);
        QPainterPath cut;
        cut.addEllipse(QPointF(80, 52), 30, 30);
        p.drawPath(moon.subtracted(cut));
    }
    p.end();
    return QIcon(pm);
}
