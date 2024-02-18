#include <QCoreApplication>
#include <QDebug>
#include <WCHBLEDLL.h>
#include "ble.h"
#include <QTime>



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    BLE::Init();
    BLE::scanDevice(500);
    BLE::rssiNotify();

    BLE *ble = new BLE;
    qDebug() << ble->connect();
    Characteristic *cNotify = ble->getCharacteristic(0xFFF0, 0xFFF1);
    Characteristic *cWrite = ble->getCharacteristic(0xFFF0, 0xFFF2);
    QObject::connect(cNotify, &Characteristic::readyRead, [&](){
        char ss[513] = {0};cNotify->readNotify(ss);
        qDebug() << "readyRead" << ss;
    });
    cNotify->enableNotify(true);
    cWrite->write("Hello");

    while(1)
    {
        //qDebug() << ble->rssi();
    }

    //BLE::connectDevice("BluetoothLE#BluetoothLE00:1a:7d:da:71:11-38:3b:26:93:14:58");

    // BLE::connectDevice();

//    WCHBLEInit();
//    bool bleIsOpen = WCHBLEIsBluetoothOpened();
//    qDebug() << WCHBLEGetBluetoothVer();
//    qDebug() << "BLE " << WCHBLEIsCentralRoleSupported();

//    BLENameDevID devs[128];
//    ULONG devNum;
//    QString str = "ch583_ble_ua";
//    char* pChar = new char[str.length() + 1];
//    strcpy_s(pChar, str.length() + 1, str.toLocal8Bit().data());
//    WCHBLEEnumDevice(500, pChar, devs, &devNum);
//    qDebug() << "dev nums:" << devNum;

//    for (int i = 0; i < devNum; i++)
//    {
//        BLENameDevID name = devs[i];
//        qDebug() << QString::fromLocal8Bit(reinterpret_cast<const char*>(name.Name));
//        qDebug() << QString::fromLocal8Bit(reinterpret_cast<const char*>(name.DevID));
//        qDebug() << name.Rssi;

//        qDebug() << WCHBLEOpenDevice((PCHAR)name.DevID, BleCallback);
//    }

    while(1);


    return a.exec();
}
