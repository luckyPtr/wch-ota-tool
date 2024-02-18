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
    union
    {
        struct
        {
            quint8 readable : 1;
            quint8 writable : 1;
            quint8 notifyable : 1;
            quint8 : 5;
        };
        quint8 action;
    };

    struct ParamInf
    {
        WCHBLEHANDLE handle;
        USHORT ServiceUUID;
        USHORT CharacteristicUUID;

        bool operator==(const ParamInf &other) const {
            return (handle == other.handle) && (ServiceUUID == other.ServiceUUID) && (CharacteristicUUID == other.CharacteristicUUID);
        }
    };

    ParamInf paramInf;
    QQueue<char> notifyRcvQue;

    explicit Characteristic(QObject *parent = nullptr);
    Characteristic(const Characteristic &other) {
        paramInf = other.paramInf;
        action = other.action;
    }

    void write(const QByteArray &ba, bool response = false);
    void write(const char *buff, quint16 len, bool response = false);
    void write(const char *buff, bool response = false);
    QByteArray read();
    quint16 read(char *buff, quint16 maxlen = 512);
    bool enableNotify(bool enable);
    quint16 readNotify(char *buff, quint16 maxlen = 512);
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

private:
    static QVector<BleDevInfo> scannedDev;  // 扫描获取到的设备
    static QVector<Characteristic*> characteristic; // 连接设备的特征
    BleDevInfo *dev;
    QMap<USHORT, QVector<USHORT>> UUID;
    quint16 MTU = 0;

    static void devConnChangedCallback(void* hDev, UCHAR ConnectStatus);
    static void readCallback(void *paramInf, PCHAR readBuf, ULONG readBufLen);
    static void rssiCallback(PCHAR pMAC, int rssi);
    static bool compareMacAddresses(const QString& macAddress1, const QString& macAddress2);

    void getAllServicesUUID();
    void getAllCharacteristicUUID();
    void getMTU();
public:
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
    quint16 mtu();

    QMap<USHORT, QVector<USHORT>> getAllUUID();
    QVector<USHORT> getServiceUUID();
    QVector<USHORT> getCharacteristicUUID(quint16 service);
    Characteristic *getCharacteristic(quint16 service, quint16 characteraistic);
signals:
    void readyRead();

    friend class Characteristic;
};



#endif // BLE_H
