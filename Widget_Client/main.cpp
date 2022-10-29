//#include "Mainwindow.h"

#include <QApplication>
#include "IoServiceWorker.h"
#include "WindowManager.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WindowManager w{IoServiceWorker::getInstance().getService()};
    //w.show();
    return a.exec();
}
