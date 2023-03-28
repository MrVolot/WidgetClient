#include "NotificationWidget.h"
#include "ui_NotificationWidget.h"
#include <QApplication>
#include <QDesktopServices>
#include <QPainter>
#include <QScreen>
#include <QMouseEvent>

NotificationWidget::NotificationWidget(const QString& text, unsigned long long senderId, const QString& senderName, QWidget *parent) :
    QWidget(parent),
    text_{text},
    senderId_{senderId},
    senderName_{senderName},
    ui(new Ui::NotificationWidget)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint |        // Disable window decoration
                   Qt::Tool |                       // Discard display in a separate window
                   Qt::WindowStaysOnTopHint);       // Set on top of all windows
    setAttribute(Qt::WA_TranslucentBackground);     // Indicates that the background will be transparent
    setAttribute(Qt::WA_ShowWithoutActivating);     // At the show, the widget does not get the focus automatically

    animation.setTargetObject(this);                // Set the target animation
    animation.setPropertyName("popupOpacity");      //
    connect(&animation, &QAbstractAnimation::finished, this, &NotificationWidget::hide);

    QString styleSheet{"QLabel { color : white; "
                       "margin-top: 6px;"
                       "margin-bottom: 6px;"
                       "margin-left: 10px;"
                       "margin-right: 10px; }"};
    ui->senderLabel->setStyleSheet(styleSheet);
    ui->notificationMessage->setStyleSheet(styleSheet);

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &NotificationWidget::hideAnimation);
}

NotificationWidget::~NotificationWidget()
{
    delete ui;
}

void NotificationWidget::show()
{
    setWindowOpacity(0.0);  // Set the transparency to zero

    animation.setDuration(150);     // Configuring the duration of the animation
    animation.setStartValue(0.0);   // The start value is 0 (fully transparent widget)
    animation.setEndValue(1.0);     // End - completely opaque widget
    setGeometry(QGuiApplication::primaryScreen()->geometry().width() - 50 - width() + QGuiApplication::primaryScreen()->geometry().x(),
                QGuiApplication::primaryScreen()->geometry().height() - 50 - height() + QGuiApplication::primaryScreen()->geometry().y(),
                width(),
                height());
    QWidget::show();

    animation.start();
    timer->start(3000);
}

void NotificationWidget::showNotification()
{
    if(text_.length()>50){
        auto textToShow{text_.left(50)};
        textToShow.append("...");
        ui->notificationMessage->setText(textToShow);
    }else{
        ui->notificationMessage->setText(text_);
    }
    ui->senderLabel->setText(senderName_);
    adjustSize();
    show();
}

void NotificationWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QRect roundedRect;
        roundedRect.setX(rect().x() + 5);
        roundedRect.setY(rect().y() + 5);
        roundedRect.setWidth(rect().width() - 10);
        roundedRect.setHeight(rect().height() - 10);

        painter.setBrush(QBrush(QColor(0,0,0,180)));
        painter.setPen(Qt::NoPen);

        painter.drawRoundedRect(roundedRect, 10, 10);
}

void NotificationWidget::mousePressEvent(QMouseEvent *event)
{
    //TODO understand how to free memory, since we won't use the same notification widget ever again
    //Call dest?
    emit reactOnNotification(senderId_);
    emit showMainWindow();
}

void NotificationWidget::hideAnimation()
{
    timer->stop();
    animation.setDuration(1000);
    animation.setStartValue(1.0);
    animation.setEndValue(0.0);
    animation.start();
}

void NotificationWidget::hide()
{
    // If the widget is transparent, then hide it
    if(getPopupOpacity() == 0.0){
        QWidget::hide();
    }
}

void NotificationWidget::setPopupOpacity(float opacity)
{
    popupOpacity = opacity;

    setWindowOpacity(opacity);
}

float NotificationWidget::getPopupOpacity() const
{
    return popupOpacity;
}
