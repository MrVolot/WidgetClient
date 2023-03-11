#pragma once

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

namespace Ui {
class NotificationWidget;
}

class NotificationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)

    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;
public:
    explicit NotificationWidget(QWidget *parent = nullptr);
    ~NotificationWidget();

private:
    Ui::NotificationWidget *ui;
    QPropertyAnimation animation;
    float popupOpacity;
    QTimer *timer;

private slots:
    void hideAnimation();                   // Slot start the animation hide
    void hide();

public slots:
    void setPopupText(const QString& text); // Setting text notification
    void show();
    void showNotification(const QString& text);

protected:
    void paintEvent(QPaintEvent *event);
};

