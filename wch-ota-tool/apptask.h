
#ifndef APPTASK_H
#define APPTASK_H


#include <QObject>
#include <QTimer>


class AppTask : public QObject
{
    Q_OBJECT

    QTimer timer;
    int StringToInt(QString str);
    void printDevices();
public:
    explicit AppTask(QObject *parent = nullptr);

    void run();
    void quit();
signals:

};

#endif // APPTASK_H
