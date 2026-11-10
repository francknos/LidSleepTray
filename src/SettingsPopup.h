#pragma once
#include <QWidget>

class QPushButton;
class QLabel;

// Small modern flyout shown on right-click of the tray icon.
class SettingsPopup : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPopup(QWidget *parent = nullptr);

    void setBlocked(bool blocked);
    void setOnAc(bool onAc);
    void setLidAction(bool ac, int action); // 0 = do nothing, 1 = sleep
    void setAutostart(bool enabled);
    void setStatus(const QString &text);
    void showNearTray();

signals:
    void sleepToggled(bool blocked);
    void lidActionChanged(bool ac, int action);
    void autostartToggled(bool enabled);
    void restoreRequested();
    void quitRequested();

protected:
    bool event(QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    struct Section {
        QPushButton *nothing = nullptr;
        QPushButton *sleep = nullptr;
        QLabel *badge = nullptr;
    };
    Section makeSection(class QVBoxLayout *parent, const QString &title, bool ac);

    QPushButton *m_toggle = nullptr;
    QLabel *m_toggleHint = nullptr;
    QPushButton *m_autostart = nullptr;
    QLabel *m_status = nullptr;
    Section m_ac;
    Section m_dc;
};
