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
    static void scanDevice(int scanTime = 1000, QString filter = NULL);   // 扫描BLE设备
    static void rssiNotify();

    bool connect();
    bool connect(QString devID);
    void disconnect();
    bool isConnect();
    int rssi();
    QString name();
    QString devID();
    QString mac();
    void write(QByteArray ba);
    QByteArray readAll();
private:
    static QVector<BleDevInfo> scannedDev;  // 扫描获取到的设备
    BleDevInfo *dev;

    static void devConnChangedCallback(void* hDev, UCHAR ConnectStatus);
    static void rssiCallback(PCHAR pMAC, int rssi);
    static bool compareMacAddresses(const QString& macAddress1, const QString& macAddress2);


signals:
    void readyRead();
};



#endif // BLE_H
