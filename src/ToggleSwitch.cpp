#include "ToggleSwitch.h"

#include <QPainter>

ToggleSwitch::ToggleSwitch(QWidget *parent) : QAbstractButton(parent), m_anim(this, "pos", this)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(sizeHint());
    m_anim.setDuration(140);
    m_anim.setEasingCurve(QEasingCurve::InOutQuad);
}

void ToggleSwitch::checkStateSet()
{
    m_anim.stop();
    m_anim.setStartValue(m_pos);
    m_anim.setEndValue(isChecked() ? 1.0 : 0.0);
    if (isVisible())
        m_anim.start();
    else
        m_pos = isChecked() ? 1.0 : 0.0;
    update();
}

void ToggleSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(rect()).adjusted(1, 1, -1, -1);
    const qreal h = r.height();

    QColor off(0x3a, 0x42, 0x59), on(0x3b, 0x82, 0xf6);
    auto mix = [&](int a, int b) { return int(a + (b - a) * m_pos); };
    QColor bg(mix(off.red(), on.red()), mix(off.green(), on.green()), mix(off.blue(), on.blue()));
    if (underMouse())
        bg = bg.lighter(110);

    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawRoundedRect(r, h / 2, h / 2);

    const qreal d = h - 6;
    const qreal x = r.left() + 3 + (r.width() - d - 6) * m_pos;
    p.setBrush(Qt::white);
    p.drawEllipse(QRectF(x, r.top() + 3, d, d));
}
