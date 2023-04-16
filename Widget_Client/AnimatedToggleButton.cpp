#include "AnimatedToggleButton.h"
#include <QPainter>
#include <QStyleOption>

AnimatedToggleButton::AnimatedToggleButton(QWidget *parent)
    : QAbstractButton(parent), m_indicatorOffset(2) {
    setCheckable(true);
    setFixedSize(50, 25);

    animation = new QPropertyAnimation(this, "indicatorOffset", this);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(this, &QAbstractButton::toggled, [this](bool checked) {
        animation->setStartValue(checked ? 2 : width() - height() + 2);
        animation->setEndValue(checked ? width() - height() + 2 : 2);
        animation->start();
    });
}

void AnimatedToggleButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOption opt;
    opt.initFrom(this);
    QRectF buttonRect = opt.rect.adjusted(1, 1, -1, -1);

    // Draw button background
    painter.setBrush(isChecked() ? QColor("#4CAF50") : QColor("#ddd"));
    painter.setPen(QColor("#999"));
    painter.drawRoundedRect(buttonRect, buttonRect.height() / 2, buttonRect.height() / 2);

    // Draw indicator
    QRectF indicatorRect(0, 0, buttonRect.height() - 4, buttonRect.height() - 4);
    indicatorRect.moveCenter(buttonRect.center());
    indicatorRect.moveLeft(m_indicatorOffset);
    painter.setBrush(Qt::white);
    painter.drawEllipse(indicatorRect);
}

int AnimatedToggleButton::indicatorOffset() const {
    return m_indicatorOffset;
}

void AnimatedToggleButton::setIndicatorOffset(int offset) {
    m_indicatorOffset = offset;
    update();
}

void AnimatedToggleButton::setToggleState(bool state) {
    if (isChecked() != state) {
        setChecked(state);
        // Emit the toggled signal, which will trigger the connected animation
        emit toggled(state);
    }
}
