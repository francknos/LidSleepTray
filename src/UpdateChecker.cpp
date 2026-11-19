#include "UpdateChecker.h"
#include "UpdateDialog.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTimer>
#include <QVersionNumber>
#include <memory>

namespace {
constexpr auto kLatestRelease = "https://api.github.com/repos/francknos/LidSleepTray/releases/latest";
constexpr auto kInstallerName = "LidSleepTray-Setup.exe";
constexpr qint64 kMaxMetadataBytes = 256 * 1024;
constexpr qint64 kMaxInstallerBytes = 128 * 1024 * 1024;

QVersionNumber parseVersion(const QString &version)
{
    static const QRegularExpression stableVersion(QStringLiteral(R"(\A[0-9]{1,9}\.[0-9]{1,9}\.[0-9]{1,9}\z)"));
    return stableVersion.match(version).hasMatch() ? QVersionNumber::fromString(version)
                                                  : QVersionNumber();
}

QNetworkRequest requestFor(const QUrl &url, QNetworkRequest::RedirectPolicy redirects)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "LidSleepTray/" APP_VERSION);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, redirects);
    request.setMaximumRedirectsAllowed(3);
    request.setTransferTimeout(15000);
    return request;
}

struct Download {
    QTemporaryDir directory{QDir::temp().filePath(QStringLiteral("LidSleepTray-update-XXXXXX"))};
    QFile file{directory.filePath(QString::fromLatin1(kInstallerName))};
    QCryptographicHash hash{QCryptographicHash::Sha256};
    qint64 received = 0;
    bool cancelled = false;
    bool writeFailed = false;
    bool tooLarge = false;
};
}

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent) {}

std::optional<UpdateChecker::Release> UpdateChecker::availableRelease(
    const QByteArray &json, const QString &installedVersion, bool *validResponse)
{
    if (validResponse)
        *validResponse = false;
    const QJsonObject release = QJsonDocument::fromJson(json).object();
    if (release.isEmpty()) {
        qWarning("Invalid GitHub release response");
        return std::nullopt;
    }

    if (release.value("draft").toBool() || release.value("prerelease").toBool()) {
        if (validResponse)
            *validResponse = true;
        return std::nullopt;
    }

    const QString tag = release.value("tag_name").toString();
    const auto current = parseVersion(installedVersion);
    const auto latest = tag.startsWith(u'v') ? parseVersion(tag.mid(1)) : QVersionNumber();
    if (current.isNull() || latest.isNull()) {
        qWarning("Invalid LidSleepTray version in release check");
        return std::nullopt;
    }
    if (latest <= current) {
        if (validResponse)
            *validResponse = true;
        return std::nullopt;
    }

    const QString expectedUrl = QStringLiteral(
        "https://github.com/francknos/LidSleepTray/releases/download/%1/%2")
        .arg(tag, QString::fromLatin1(kInstallerName));
    static const QRegularExpression digestFormat(QStringLiteral(R"(\Asha256:([a-fA-F0-9]{64})\z)"));
    for (const auto &entry : release.value("assets").toArray()) {
        const auto asset = entry.toObject();
        if (asset.value("name").toString() != QLatin1String(kInstallerName))
            continue;
        const auto digest = digestFormat.match(asset.value("digest").toString());
        const qint64 size = asset.value("size").toInteger();
        if (asset.value("browser_download_url").toString() == expectedUrl
            && digest.hasMatch() && size > 0 && size <= kMaxInstallerBytes) {
            if (validResponse)
                *validResponse = true;
            return Release{tag, QUrl(expectedUrl),
                           QByteArray::fromHex(digest.captured(1).toLatin1()), size};
        }
    }

    qWarning("No verifiable installer in the latest LidSleepTray release");
    return std::nullopt;
}

void UpdateChecker::setBusy(bool busy)
{
    m_busy = busy;
    emit busyChanged(busy);
}

void UpdateChecker::check(bool manual)
{
    if (m_busy)
        return;
    setBusy(true);
    auto request = requestFor(QUrl(QLatin1String(kLatestRelease)), QNetworkRequest::ManualRedirectPolicy);
    request.setRawHeader("Accept", "application/vnd.github+json");
    auto *reply = m_network.get(request);
    reply->setReadBufferSize(kMaxMetadataBytes + 1);
    QTimer::singleShot(15000, reply, [reply] {
        if (reply->isRunning())
            reply->abort();
    });
    connect(reply, &QNetworkReply::readyRead, reply, [reply] {
        if (reply->bytesAvailable() > kMaxMetadataBytes)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, manual] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200
            || reply->bytesAvailable() > kMaxMetadataBytes) {
            qWarning() << "LidSleepTray update check failed:" << reply->errorString();
            setBusy(false);
            if (manual)
                showMessage(tr("La vérification a échoué. Vérifie ta connexion Internet et réessaie."));
            return;
        }
        bool validResponse = false;
        const QString installed = QCoreApplication::applicationVersion();
        if (const auto release = availableRelease(reply->readAll(), installed, &validResponse)) {
            offer(*release);
            return;
        }
        setBusy(false);
        if (manual) {
            showMessage(validResponse
                ? tr("LidSleepTray %1 est à jour.").arg(installed)
                : tr("Impossible de vérifier la dernière version sur GitHub. Réessaie plus tard."));
        }
    });
}

void UpdateChecker::offer(const Release &release)
{
    auto *dialog = new UpdateDialog(m_lightTheme, tr("Mise à jour disponible"),
        tr("La version %1 est disponible (version installée : %2).")
            .arg(release.tag.mid(1), QCoreApplication::applicationVersion()),
        tr("LidSleepTray se fermera à l’ouverture de l’installateur."),
        tr("Télécharger et installer"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QDialog::finished, this, [this, release](int result) {
        if (result == QDialog::Accepted)
            download(release);
        else
            setBusy(false);
    });
    dialog->show();
}

void UpdateChecker::download(const Release &release)
{
    auto transfer = std::make_shared<Download>();
    if (!transfer->directory.isValid() || !transfer->file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
        setBusy(false);
        showMessage(tr("Impossible de créer le fichier temporaire de l’installateur."));
        return;
    }

    auto *reply = m_network.get(requestFor(release.installer, QNetworkRequest::UserVerifiedRedirectPolicy));
    reply->setReadBufferSize(256 * 1024);
    auto *progress = new QProgressDialog(tr("Téléchargement de LidSleepTray %1…").arg(release.tag.mid(1)),
                                        tr("Annuler"), 0, static_cast<int>(release.size));
    progress->setWindowTitle(tr("Mise à jour de LidSleepTray"));
    progress->setAutoClose(false);
    progress->setAutoReset(false);
    progress->setMinimumDuration(0);
    progress->setValue(0);
    progress->show();

    connect(progress, &QProgressDialog::canceled, reply, [reply, transfer] {
        transfer->cancelled = true;
        reply->abort();
    });
    connect(reply, &QNetworkReply::redirected, reply, [reply](const QUrl &url) {
        if (url.scheme() == QStringLiteral("https")
            && url.host() == QStringLiteral("release-assets.githubusercontent.com")
            && url.port(443) == 443 && url.userInfo().isEmpty()) {
            reply->redirectAllowed();
        } else {
            reply->abort();
        }
    });
    QTimer::singleShot(5 * 60 * 1000, reply, [reply] {
        if (reply->isRunning())
            reply->abort();
    });

    const auto writeAvailable = [reply, progress, transfer, release] {
        if (transfer->cancelled || transfer->writeFailed || transfer->tooLarge
            || reply->bytesAvailable() == 0)
            return;
        const auto bytes = reply->readAll();
        if (bytes.size() > release.size - transfer->received)
            transfer->tooLarge = true;
        else
            transfer->writeFailed = transfer->file.write(bytes) != bytes.size();
        if (transfer->tooLarge || transfer->writeFailed) {
            if (reply->isRunning())
                reply->abort();
            return;
        }
        transfer->hash.addData(bytes);
        transfer->received += bytes.size();
        progress->setValue(static_cast<int>(transfer->received));
    };
    connect(reply, &QNetworkReply::readyRead, this, writeAvailable);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, progress, transfer, release, writeAvailable] {
        reply->deleteLater();
        progress->deleteLater();
        setBusy(false);
        if (transfer->cancelled)
            return;
        writeAvailable();
        progress->hide();
        if (transfer->writeFailed || !transfer->file.flush()) {
            showMessage(tr("Impossible d’enregistrer l’installateur. Vérifie l’espace disque disponible."));
            return;
        }
        transfer->file.close();
        const auto unverified = tr("L’installateur téléchargé n’a pas pu être vérifié. Il n’a pas été lancé.");
        if (transfer->tooLarge) {
            showMessage(unverified);
            return;
        }
        if (reply->error() != QNetworkReply::NoError
            || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
            qWarning() << "Installer download failed:" << reply->errorString();
            showMessage(tr("Le téléchargement a échoué. Vérifie la connexion Internet et relance la vérification."));
            return;
        }
        if (transfer->received != release.size || transfer->hash.result() != release.sha256) {
            showMessage(unverified);
            return;
        }

        if (!QProcess::startDetached(transfer->file.fileName(), {}, transfer->directory.path())) {
            showMessage(tr("Windows n’a pas pu ouvrir l’installateur. Tu peux le récupérer sur GitHub."));
            return;
        }
        // The detached installer still needs the file after LidSleepTray exits.
        transfer->directory.setAutoRemove(false);
        QCoreApplication::quit();
    });
}

void UpdateChecker::showMessage(const QString &message)
{
    auto *dialog = new UpdateDialog(m_lightTheme, tr("Mises à jour de LidSleepTray"), message);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}
