
#pragma once

#include <QAbstractButton>
#include <QPropertyAnimation>



class AnimatedToggleButton: public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int indicatorOffset READ indicatorOffset WRITE setIndicatorOffset)
public:
    AnimatedToggleButton(QWidget *parent = nullptr);

    int indicatorOffset() const;
    void setIndicatorOffset(int offset);
    void setToggleState(bool state);
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPropertyAnimation *animation;
    int m_indicatorOffset;
};

