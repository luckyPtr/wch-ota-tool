#ifndef BLE_H
#define BLE_H

#include <QObject>
#include <QVector>
#include "WCHBLEDLL.h"

class BLE : public QObject
{
    Q_OBJECT


public:
    explicit BLE(QObject *parent = nullptr);

    struct BleDevInfo
    {
        QString name;
        QString devID;
        QString mac;
        int rssi;
        WCHBLEHANDLE handle = 0;    // 设备句柄，连接上不为0
    };


    static void Init();
    static bool isBleOpened();
    static QString getBleVer();
    static bool isSupportBle();
    static void scanDevice(int scanTime, QString filter);   // 扫描BLE设备
    static bool connectDevice(QString devID);
    static bool connectDevice();    // 默认连接搜索到的信号最强的BLE设备
    static void rssiNotify();

private:
    static QVector<BleDevInfo> bleDev;

    static void devConnChangedCallback(void* hDev, UCHAR ConnectStatus);
    static void rssiCallback(PCHAR pMAC, int rssi);
signals:

};

#endif // BLE_H
