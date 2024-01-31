#include <QCoreApplication>
#include <QDebug>
#include <WCHBLEDLL.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << argv;

    WCHBLEInit();
    bool bleIsOpen = WCHBLEIsBluetoothOpened();
    qDebug() << WCHBLEGetBluetoothVer();
    qDebug() << "BLE " << WCHBLEIsCentralRoleSupported();

    while(1)
    {
    BLENameDevID devs[128];
    ULONG devNum;
    QString str = "ch583_ble_ua";
    char* pChar = new char[str.length() + 1];
    strcpy_s(pChar, str.length() + 1, str.toLocal8Bit().data());
    WCHBLEEnumDevice(500, pChar, devs, &devNum);
    qDebug() << "dev nums:" << devNum;

    for (int i = 0; i < devNum; i++)
    {
        BLENameDevID name = devs[i];
        qDebug() << QString::fromLocal8Bit(reinterpret_cast<const char*>(name.Name));
        qDebug() << QString::fromLocal8Bit(reinterpret_cast<const char*>(name.DevID));
        qDebug() << name.Rssi;
    }
    }

    return a.exec();
}
