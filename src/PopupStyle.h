#pragma once

#include <QString>

inline QString popupStyle(bool light)
{
    return QStringLiteral(R"(
#card { background: %1; border: 1px solid %2; border-radius: 16px; }
QLabel { color: %3; font-family: "Segoe UI Variable", "Segoe UI", sans-serif; font-size: 13px; }
#title { font-size: 15px; font-weight: 600; }
#muted { color: %4; font-size: 12px; }
QPushButton { font-family: "Segoe UI Variable", "Segoe UI", sans-serif; font-size: 13px; border: none; }
QPushButton#theme, QPushButton#checkUpdates { background: transparent; border-radius: 8px; }
QPushButton#checkUpdates { color: %4; font-size: 15px; }
QPushButton#checkUpdates:disabled { color: %4; background: %6; }
QPushButton#theme:hover, QPushButton#theme:focus,
QPushButton#checkUpdates:hover, QPushButton#checkUpdates:focus { background: %5; }
QPushButton#updateSecondary, QPushButton#updatePrimary {
    border: 1px solid transparent; border-radius: 10px; padding: 8px 12px;
}
QPushButton#updateSecondary { background: %6; color: %3; }
QPushButton#updateSecondary:hover { background: %5; }
QPushButton#updatePrimary { background: #2563c7; color: white; font-weight: 600; }
QPushButton#updatePrimary:hover { background: #1d53ad; }
QPushButton#updateSecondary:focus, QPushButton#updatePrimary:focus { border-color: %7; }
)")
        .arg(light ? "#f8faff" : "#1b1f2a", light ? "#d5dce9" : "#2c3347",
             light ? "#202a3d" : "#e6e9f2", light ? "#59677e" : "#8b93a7",
             light ? "#cbd7e9" : "#343c53", light ? "#dbe3f0" : "#2b3245",
             light ? "#202a3d" : "#b9d5ff");
}
