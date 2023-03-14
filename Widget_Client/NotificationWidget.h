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
    QString text_;
    unsigned long long senderId_;
    QString senderName_;
    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;
public:
    explicit NotificationWidget(const QString& text, unsigned long long senderId, const QString& senderName, QWidget *parent = nullptr);
    ~NotificationWidget();

private:
    Ui::NotificationWidget *ui;
    QPropertyAnimation animation;
    float popupOpacity;
    QTimer *timer;

private slots:
    void hideAnimation();
    void hide();
    void show();

public slots:
    void showNotification();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void showMainWindow();
    void reactOnNotification(unsigned long long id);
};

