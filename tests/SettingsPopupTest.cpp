#include "SettingsPopup.h"

#include <QApplication>
#include <QPushButton>
#include <QString>

#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    SettingsPopup popup;
    auto *theme = popup.findChild<QPushButton *>("theme");
    auto *toggle = popup.findChild<QPushButton *>("toggle");
    if (!theme || !toggle)
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
