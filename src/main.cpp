#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QSystemTrayIcon>

#include "TrayApp.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LidSleepTray");
    app.setApplicationVersion(APP_VERSION);
    app.setQuitOnLastWindowClosed(false);

    // Single instance.
    QLockFile lock(QDir::temp().filePath("LidSleepTray.lock"));
    lock.setStaleLockTime(0);
    if (!lock.tryLock(100))
        return 0;

    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return 1;

    TrayApp tray;
    return app.exec();
}
