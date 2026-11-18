#include "SettingsPopup.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QString>

#include <iostream>

static int captureScreenshots(const QString &directory)
{
    if (!QDir().mkpath(directory))
        return 1;

    QFontDatabase::addApplicationFont("C:/Windows/Fonts/segoeui.ttf");
    QFontDatabase::addApplicationFont("C:/Windows/Fonts/segoeuib.ttf");
    QFontDatabase::addApplicationFont("C:/Windows/Fonts/seguiemj.ttf");
    QFontDatabase::addApplicationFont("C:/Windows/Fonts/seguisym.ttf");

    SettingsPopup popup;
    popup.setOnAc(true);
    popup.setLidAction(true, 1);
    popup.setLidAction(false, 0);
    popup.setPowerMode(1);
    popup.adjustSize();
    popup.show();
    QApplication::processEvents();

    if (!popup.grab().save(QDir(directory).filePath("popup-dark.png")))
        return 2;

    popup.setLightTheme(true);
    QApplication::processEvents();
    if (!popup.grab().save(QDir(directory).filePath("popup-light.png")))
        return 3;

    return 0;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(APP_VERSION);
    if (argc == 3 && QString::fromLocal8Bit(argv[1]) == "--capture-screenshots")
        return captureScreenshots(QString::fromLocal8Bit(argv[2]));

    SettingsPopup popup;
    auto *theme = popup.findChild<QPushButton *>("theme");
    auto *toggle = popup.findChild<QPushButton *>("toggle");
    auto *version = popup.findChild<QLabel *>("version");
    if (!theme || !toggle || !version
        || version->text() != QStringLiteral("v%1").arg(QCoreApplication::applicationVersion()))
        return 1;

    int changes = 0;
    bool selectedLight = false;
    QObject::connect(&popup, &SettingsPopup::themeChanged, [&](bool light) {
        ++changes;
        selectedLight = light;
    });

    popup.setBlocked(true);
    const QString darkAction = theme->accessibleName();
    if (!popup.styleSheet().contains("#1b1f2a") || theme->icon().isNull()
        || darkAction.isEmpty() || theme->toolTip() != darkAction)
        return 2;

    theme->click();
    if (changes != 1 || !selectedLight || !toggle->isChecked()
        || !popup.styleSheet().contains("#f8faff") || theme->icon().isNull()
        || theme->accessibleName().isEmpty() || theme->accessibleName() == darkAction
        || theme->toolTip() != theme->accessibleName())
        return 3;

    theme->click();
    if (changes != 2 || selectedLight || !toggle->isChecked()
        || !popup.styleSheet().contains("#1b1f2a"))
        return 4;

    popup.setLightTheme(true);
    if (changes != 2 || !popup.styleSheet().contains("#f8faff"))
        return 5;

    std::cout << "Theme switching passed\n";
    return 0;
}
