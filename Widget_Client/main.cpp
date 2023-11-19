#include <QApplication>
#include "IoServiceWorker.h"
#include "WindowManager.h"
int main(int argc, char *argv[])
{
    if (argc > 1) {
        try{
        // Set an environment variable to the value of the first argument
        _putenv_s("MY_ID", argv[1]);
        _putenv_s("RECEIVER_ID", argv[2]);
        _putenv_s("MESSAGE_TO_SENT", argv[3]);
        }
        catch(...){
            std::cout<<"ERROR IN MAIN";
        }
    } else {
        std::cout << "No argument provided to set as environment variable." << std::endl;
    }
    QApplication a(argc, argv);
    WindowManager w{IoServiceWorker::getInstance().getService()};
    return 0;
//    return a.exec();
}
