#ifndef BLE_H
#define BLE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QQueue>
#include "WCHBLEDLL.h"

class Characteristic : public QObject
{
    Q_OBJECT

public:
    USHORT serviceUUID;
    USHORT UUID;

    union
    {
        struct
        {

            bool readable;
            bool writable;
            bool notifyable;
            quint8 : 5;
        };
        quint8 action;
    };

     QQueue<char> recvData;


    explicit Characteristic(QObject *parent = nullptr);
    Characteristic(const Characteristic &other) {
        UUID = other.UUID;
        action = other.action;
    }
    QByteArray readAll();
    void write(QByteArray ba);
 signals:
    void readyRead();

// public slots:
//     void on_ReceiveData(QByteArray);
};

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
    void getUUID();
    int rssi();
    QString name();
    QString devID();
    QString mac();
    void write(QByteArray ba);
    QByteArray readAll();
private:
    static QVector<BleDevInfo> scannedDev;  // 扫描获取到的设备
    BleDevInfo *dev;
    QMap<USHORT, QVector<Characteristic*>> Service;

    static void devConnChangedCallback(void* hDev, UCHAR ConnectStatus);
    static void rssiCallback(PCHAR pMAC, int rssi);
    static bool compareMacAddresses(const QString& macAddress1, const QString& macAddress2);

    void getAllServicesUUID();
    void getAllCharacteristicUUID();
signals:
    void readyRead();
};



#endif // BLE_H
