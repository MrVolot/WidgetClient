#include "NotificationWidget.h"
#include "ui_NotificationWidget.h"
#include <QApplication>
#include <QDesktopServices>
#include <QPainter>
#include <QScreen>

NotificationWidget::NotificationWidget(QWidget *parent) :
    QWidget(parent),
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

    ui->notificationMessage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->notificationMessage->setStyleSheet("QLabel { color : white; "
                        "margin-top: 6px;"
                        "margin-bottom: 6px;"
                        "margin-left: 10px;"
                        "margin-right: 10px; }");

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

void NotificationWidget::showNotification(const QString &text)
{
    setPopupText(text);
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

void NotificationWidget::setPopupText(const QString &text)
{
    ui->notificationMessage->setText(text);    // Set the text in the Label
    adjustSize();           // With the recalculation notice sizes
}
