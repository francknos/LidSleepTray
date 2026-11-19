#include "SettingsPopup.h"
#include "UpdateDialog.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>
#include <QLabel>
#include <QKeyEvent>
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

    popup.hide();
    for (bool light : {false, true}) {
        UpdateDialog dialog(light, QStringLiteral("Mise à jour disponible"),
                            QStringLiteral("La version 1.3.2 est disponible (version installée : 1.3.1)."),
                            QStringLiteral("LidSleepTray se fermera à l’ouverture de l’installateur."),
                            QStringLiteral("Télécharger et installer"));
        dialog.show();
        QApplication::processEvents();
        if (!dialog.grab().save(QDir(directory).filePath(light ? "update-light.png" : "update-dark.png")))
            return 4;
    }

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

    auto *updateButton = popup.findChild<QPushButton *>("checkUpdates");
    int checks = 0;
    QObject::connect(&popup, &SettingsPopup::updateCheckRequested, [&] { ++checks; });
    if (!updateButton || updateButton->accessibleName().isEmpty())
        return 6;
    popup.setUpdateBusy(true);
    updateButton->click();
    popup.setUpdateBusy(false);
    if (checks != 0)
        return 7;
    updateButton->click();
    if (checks != 1)
        return 8;

    UpdateDialog offer(true, "Mise à jour", "Une nouvelle version est disponible.", {}, "Installer");
    auto *install = offer.findChild<QPushButton *>("updatePrimary");
    auto *later = offer.findChild<QPushButton *>("updateSecondary");
    offer.show();
    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(&offer, &enter);
    if (!install || !later || offer.isVisible() || offer.result() != QDialog::Rejected)
        return 9;
    offer.show();
    install->click();
    if (offer.result() != QDialog::Accepted)
        return 10;

    std::cout << "Theme switching passed\n";
    return 0;
}
