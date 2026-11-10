#include "PowerManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#include <powrprof.h>

// Power setting subgroup "Power buttons and lid" and setting "Lid close action".
static const GUID kButtonsSubgroup = {0x4f971e89, 0xeebd, 0x4455, {0xa8, 0xde, 0x9e, 0x59, 0x04, 0x0e, 0x73, 0x47}};
static const GUID kLidAction = {0x5ca83367, 0x6e45, 0x459f, {0xa2, 0x7b, 0x47, 0x6b, 0x1d, 0x01, 0xc9, 0x36}};
#endif

PowerManager::PowerManager(QObject *parent) : QObject(parent) {}

PowerManager::~PowerManager()
{
    setSleepBlocked(false);
}

void PowerManager::setSleepBlocked(bool blocked)
{
#ifdef Q_OS_WIN
    SetThreadExecutionState(blocked ? (ES_CONTINUOUS | ES_SYSTEM_REQUIRED) : ES_CONTINUOUS);
#endif
    m_blocked = blocked;
}

bool PowerManager::isOnAcPower() const
{
#ifdef Q_OS_WIN
    SYSTEM_POWER_STATUS s;
    if (GetSystemPowerStatus(&s))
        return s.ACLineStatus != 0; // 0 = battery, 1 = AC, 255 = unknown (treat as AC)
#endif
    return true;
}

int PowerManager::lidActionRaw(bool ac) const
{
#ifdef Q_OS_WIN
    GUID *scheme = nullptr;
    if (PowerGetActiveScheme(nullptr, &scheme) != ERROR_SUCCESS)
        return -1;
    DWORD value = 0;
    DWORD rc = ac ? PowerReadACValueIndex(nullptr, scheme, &kButtonsSubgroup, &kLidAction, &value)
                  : PowerReadDCValueIndex(nullptr, scheme, &kButtonsSubgroup, &kLidAction, &value);
    LocalFree(scheme);
    return rc == ERROR_SUCCESS ? static_cast<int>(value) : -1;
#else
    Q_UNUSED(ac)
    return -1;
#endif
}

bool PowerManager::setLidActionRaw(bool ac, int value)
{
#ifdef Q_OS_WIN
    GUID *scheme = nullptr;
    if (PowerGetActiveScheme(nullptr, &scheme) != ERROR_SUCCESS)
        return false;
    DWORD rc = ac ? PowerWriteACValueIndex(nullptr, scheme, &kButtonsSubgroup, &kLidAction, static_cast<DWORD>(value))
                  : PowerWriteDCValueIndex(nullptr, scheme, &kButtonsSubgroup, &kLidAction, static_cast<DWORD>(value));
    bool ok = rc == ERROR_SUCCESS;
    if (ok)
        ok = PowerSetActiveScheme(nullptr, scheme) == ERROR_SUCCESS; // re-apply the plan
    LocalFree(scheme);
    return ok;
#else
    Q_UNUSED(ac)
    Q_UNUSED(value)
    return false;
#endif
}

#ifdef Q_OS_WIN
static const char *kRunKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char *kRunValue = "LidSleepTray";
#endif

bool PowerManager::autostartEnabled()
{
#ifdef Q_OS_WIN
    QSettings reg(kRunKey, QSettings::NativeFormat);
    return reg.contains(kRunValue);
#else
    return false;
#endif
}

bool PowerManager::setAutostart(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings reg(kRunKey, QSettings::NativeFormat);
    if (enabled) {
        const QString exe = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        reg.setValue(kRunValue, QStringLiteral("\"%1\"").arg(exe));
    } else {
        reg.remove(kRunValue);
    }
    reg.sync();
    return reg.status() == QSettings::NoError;
#else
    Q_UNUSED(enabled)
    return false;
#endif
}
