#include "UpdateDialog.h"

#include <QCoreApplication>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "PopupStyle.h"

UpdateDialog::UpdateDialog(bool light, const QString &title, const QString &message,
                           const QString &detail, const QString &action)
    : QDialog(nullptr, Qt::Dialog | Qt::FramelessWindowHint)
{
    setWindowTitle(title);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setStyleSheet(popupStyle(light));
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(14, 14, 14, 14);
    outer->setSizeConstraint(QLayout::SetFixedSize);

    auto *card = new QFrame(this);
    card->setObjectName("card");
    auto *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, light ? 85 : 140));
    card->setGraphicsEffect(shadow);
    outer->addWidget(card);

    auto *content = new QVBoxLayout(card);
    content->setContentsMargins(16, 16, 16, 14);
    content->setSpacing(10);
    auto *heading = new QLabel(title, card);
    heading->setObjectName("title");
    content->addWidget(heading);

    const auto addText = [card, content](const QString &text, const char *name) {
        auto *label = new QLabel(text, card);
        label->setObjectName(name);
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
        content->addWidget(label);
    };
    addText(message, "updateMessage");
    if (!detail.isEmpty())
        addText(detail, "muted");

    auto *actions = new QHBoxLayout;
    actions->setSpacing(8);
    actions->addStretch();
    auto *close = new QPushButton(action.isEmpty()
        ? QCoreApplication::translate("UpdateDialog", "Fermer")
        : QCoreApplication::translate("UpdateDialog", "Plus tard"), card);
    close->setObjectName(action.isEmpty() ? "updatePrimary" : "updateSecondary");
    close->setCursor(Qt::PointingHandCursor);
    close->setDefault(true);
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    actions->addWidget(close);
    if (!action.isEmpty()) {
        auto *primary = new QPushButton(action, card);
        primary->setObjectName("updatePrimary");
        primary->setCursor(Qt::PointingHandCursor);
        primary->setAutoDefault(false);
        connect(primary, &QPushButton::clicked, this, &QDialog::accept);
        actions->addWidget(primary);
    }
    content->addSpacing(4);
    content->addLayout(actions);
    close->setFocus();
}
