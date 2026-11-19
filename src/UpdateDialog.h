#pragma once

#include <QDialog>

class UpdateDialog : public QDialog
{
public:
    UpdateDialog(bool light, const QString &title, const QString &message,
                 const QString &detail = {}, const QString &action = {});
};
