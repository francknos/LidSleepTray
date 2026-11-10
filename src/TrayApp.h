#pragma once
#include <QObject>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>

#include "PowerManager.h"

class SettingsPopup;

class TrayApp : public QObject
{
    Q_OBJECT
public:
    explicit TrayApp(QObject *parent = nullptr);

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void pollPower();

private:
    void setBlocked(bool blocked);
    void applyConfig();
    void refreshUi();
    void restoreOriginal();
    static QIcon makeIcon(bool active);

    PowerManager m_power;
    QSettings m_settings;
    QSystemTrayIcon *m_tray = nullptr;
    SettingsPopup *m_popup = nullptr;
    QTimer m_timer;
    bool m_onAc = true;
    bool m_blocked = false;
    int m_acAction = 0; // 0 = do nothing, 1 = sleep
    int m_dcAction = 1;
};
