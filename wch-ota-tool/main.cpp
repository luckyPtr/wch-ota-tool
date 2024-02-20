#include <QCoreApplication>
#include <apptask.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("WCH_OTA_Tool");
    QCoreApplication::setApplicationVersion("v1.0.0");

    AppTask task;
    task.run();
    task.quit();

    return a.exec();
}







