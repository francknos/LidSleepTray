#pragma once

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>
#include <optional>

class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    struct Release {
        QString tag;
        QUrl installer;
        QByteArray sha256;
        qint64 size;
    };

    explicit UpdateChecker(QObject *parent = nullptr);
    void check();

    static std::optional<Release> availableRelease(const QByteArray &json,
                                                   const QString &installedVersion);

private:
    void offer(const Release &release);
    void download(const Release &release);
    void showError(const QString &message);

    QNetworkAccessManager m_network;
};
