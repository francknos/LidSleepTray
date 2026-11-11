#pragma once
#include <QAbstractButton>
#include <QPropertyAnimation>

// Modern pill-shaped on/off switch with a sliding knob.
class ToggleSwitch : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(qreal pos READ pos WRITE setPos)
public:
    explicit ToggleSwitch(QWidget *parent = nullptr);
    QSize sizeHint() const override { return {40, 22}; }
    qreal pos() const { return m_pos; }
    void setPos(qreal p) { m_pos = p; update(); }

protected:
    void paintEvent(QPaintEvent *) override;
    void checkStateSet() override;
    void nextCheckState() override { setChecked(!isChecked()); }

private:
    qreal m_pos = 0.0;
    QPropertyAnimation m_anim;
};
