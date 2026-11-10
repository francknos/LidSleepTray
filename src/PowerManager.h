#pragma once
#include <QObject>

// Thin wrapper around the Windows power APIs (stubs on other platforms so the
// project still compiles for development).
class PowerManager : public QObject
{
    Q_OBJECT
public:
    enum LidAction { DoNothing = 0, Sleep = 1 };

    explicit PowerManager(QObject *parent = nullptr);
    ~PowerManager() override;

    // Prevent automatic (idle) sleep. No admin rights needed.
    void setSleepBlocked(bool blocked);
    bool isSleepBlocked() const { return m_blocked; }

    bool isOnAcPower() const;

    // Raw Windows lid-close value (0 nothing, 1 sleep, 2 hibernate, 3 shutdown)
    // for the active power plan. Returns -1 on failure.
    int lidActionRaw(bool ac) const;
    bool setLidActionRaw(bool ac, int value);

    static bool autostartEnabled();
    static bool setAutostart(bool enabled);

private:
    bool m_blocked = false;
};
