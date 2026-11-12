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

// Power mode "overlay" schemes (the Settings > Power mode slider). The functions are
// exported by powrprof.dll but absent from the public SDK headers, so resolve them lazily.
static const GUID kOverlays[4] = {
    {0x961cc777, 0x2547, 0x4f9d, {0x81, 0x74, 0x7d, 0x86, 0x18, 0x1b, 0x8a, 0x7a}}, // efficiency
    {0x00000000, 0x0000, 0x0000, {0, 0, 0, 0, 0, 0, 0, 0}},                         // balanced
    {0x3af9b8d9, 0x7c97, 0x431d, {0xad, 0x78, 0x34, 0xa8, 0xbf, 0xea, 0x43, 0x9f}}, // performance
    {0xded574b5, 0x45a0, 0x4f42, {0x87, 0x37, 0x46, 0x34, 0x5c, 0x09, 0xc2, 0x38}}, // max performance
};
typedef DWORD(WINAPI *GetOverlayFn)(GUID *);
typedef DWORD(WINAPI *SetOverlayFn)(GUID);
#endif

PowerManager::PowerManager(QObject *parent) : QObject(parent) {}

PowerManager::~PowerManager()
{
    setSleepBlocked(false);
}

void PowerManager::setSleepBlocked(bool blocked)
{
#ifdef Q_OS_WIN
    SetThreadExecutionState(blocked ? (ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED) : ES_CONTINUOUS);
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

int PowerManager::powerMode() const
{
#ifdef Q_OS_WIN
    HMODULE lib = LoadLibraryW(L"powrprof.dll");
    if (!lib)
        return -1;
    auto get = reinterpret_cast<GetOverlayFn>(GetProcAddress(lib, "PowerGetEffectiveOverlayScheme"));
    int result = -1;
    GUID cur;
    if (get && get(&cur) == ERROR_SUCCESS) {
        for (int i = 0; i < 4; ++i) {
            if (IsEqualGUID(cur, kOverlays[i])) {
                result = i;
                break;
            }
        }
    }
    FreeLibrary(lib);
    return result;
#else
    return -1;
#endif
}

bool PowerManager::setPowerMode(int mode)
{
#ifdef Q_OS_WIN
    if (mode < 0 || mode > 3)
        return false;
    HMODULE lib = LoadLibraryW(L"powrprof.dll");
    if (!lib)
        return false;
    auto set = reinterpret_cast<SetOverlayFn>(GetProcAddress(lib, "PowerSetActiveOverlayScheme"));
    const bool ok = set && set(kOverlays[mode]) == ERROR_SUCCESS;
    FreeLibrary(lib);
    return ok;
#else
    Q_UNUSED(mode)
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
