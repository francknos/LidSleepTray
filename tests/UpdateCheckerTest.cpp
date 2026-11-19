#include "UpdateChecker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>

int main()
{
    const QByteArray hex(64, 'a');
    const QJsonObject asset{
        {"name", "LidSleepTray-Setup.exe"},
        {"size", 32000000},
        {"digest", QString::fromLatin1("sha256:" + hex)},
        {"browser_download_url", "https://github.com/francknos/LidSleepTray/releases/download/v1.10.0/LidSleepTray-Setup.exe"}
    };
    const QJsonObject release{
        {"tag_name", "v1.10.0"},
        {"draft", false},
        {"prerelease", false},
        {"assets", QJsonArray{asset}}
    };
    const auto parse = [](const QJsonObject &object, const QString &installed = QStringLiteral("1.2.1")) {
        return UpdateChecker::availableRelease(QJsonDocument(object).toJson(), installed);
    };
    const auto update = parse(release);
    if (!update || update->tag != QStringLiteral("v1.10.0")
        || update->size != 32000000 || update->sha256 != QByteArray::fromHex(hex)) {
        std::fputs("A newer stable version with a valid installer must be offered\n", stderr);
        return 1;
    }

    if (parse(release, "1.10.0") || parse(release, "2.0.0")) {
        std::fputs("Equal and older releases must not be offered\n", stderr);
        return 1;
    }

    for (const auto &flag : {"draft", "prerelease"}) {
        auto candidate = release;
        candidate[QLatin1String(flag)] = true;
        if (parse(candidate)) {
            std::fputs("Draft and prerelease versions must not be offered\n", stderr);
            return 1;
        }
    }

    for (const auto &field : {"digest", "browser_download_url", "size"}) {
        auto invalid = asset;
        invalid.remove(QLatin1String(field));
        auto candidate = release;
        candidate["assets"] = QJsonArray{invalid};
        if (parse(candidate)) {
            std::fputs("Unverifiable installers must not be offered\n", stderr);
            return 1;
        }
    }

    auto wrongOrigin = asset;
    wrongOrigin["browser_download_url"] = "https://example.com/LidSleepTray-Setup.exe";
    auto candidate = release;
    candidate["assets"] = QJsonArray{wrongOrigin};
    if (parse(candidate) || UpdateChecker::availableRelease("not json", "1.2.1")) {
        std::fputs("Invalid metadata and off-site installers must be rejected\n", stderr);
        return 1;
    }
}
