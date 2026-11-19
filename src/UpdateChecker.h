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
    void check(bool manual = false);
    void setLightTheme(bool light) { m_lightTheme = light; }

    static std::optional<Release> availableRelease(const QByteArray &json,
                                                   const QString &installedVersion,
                                                   bool *validResponse = nullptr);

signals:
    void busyChanged(bool busy);

private:
    void offer(const Release &release);
    void download(const Release &release);
    void showMessage(const QString &message);
    void setBusy(bool busy);

    QNetworkAccessManager m_network;
    bool m_busy = false;
    bool m_lightTheme = false;
};
