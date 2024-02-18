#include "ble.h"
#include "qglobal.h"
#include <QDebug>
#include <QRegularExpression>

QVector<BLE::BleDevInfo> BLE::scannedDev;



Characteristic::Characteristic(QObject *parent)
    : QObject{parent}
{

}

void Characteristic::write(const QByteArray &ba, bool response)
{
    write(ba.data(), ba.size(), response);
}

void Characteristic::write(const char *buff, char len, bool response)
{
    WCHBLEHANDLE handle = static_cast<BLE*>(parent())->dev->handle;
    WCHBLEWriteCharacteristic(handle, serviceUUID, characteristicUUID, response, (PCHAR)buff, len);
}

void Characteristic::write(const char *buff, bool response)
{
    write(buff, std::strlen(buff), response);
}

QByteArray Characteristic::read()
{
    char buff[512] = {0};
    quint16 len = read(buff);
    QByteArray ba(buff, len);
    return ba;
}

quint16 Characteristic::read(char *buff, quint16 maxlen)
{
    UINT len = maxlen;
    WCHBLEHANDLE handle = static_cast<BLE*>(parent())->dev->handle;
    WCHBLEReadCharacteristic(handle, serviceUUID, characteristicUUID, buff, &len);
    return len;
}


bool Characteristic::enableNotify(bool enable)
{
    WCHBLEHANDLE handle = static_cast<BLE*>(parent())->dev->handle;
    ParamInf paramInf;
    paramInf.handle = handle;
    paramInf.ServiceUUID = serviceUUID;
    paramInf.CharacteristicUUID = characteristicUUID;
    int ret;

    if (enable)
    {
        ret = WCHBLERegisterReadNotify(handle, serviceUUID, characteristicUUID, BLE::readCallback, (void*)&paramInf);
    }
    else
    {
        ret = WCHBLERegisterReadNotify(handle, serviceUUID, characteristicUUID, NULL, (void*)&paramInf);
    }

    return ret == 0;
}

BLE::BLE(QObject *parent)
    : QObject{parent}
{

}

void BLE::Init()
{
    WCHBLEInit();
}

bool BLE::isBleOpened()
{
    return WCHBLEIsBluetoothOpened();
}

QString BLE::getBleVer()
{
    const QString VER_ARRAY[] = {"V1.0", "V1.1", "V1.2", "V2.0", "V2.1", "V3.0", "V4.0", "V4.1", "V4.2", "V5.0", "V5.1", "V5.2"};
    int ver = WCHBLEGetBluetoothVer();

    if(ver >= 0 && ver < sizeof(VER_ARRAY)/sizeof(VER_ARRAY[0]))
    {
        return VER_ARRAY[ver];
    }
    return NULL;
}

bool BLE::isSupportBle()
{
    return WCHBLEIsLowEnergySupported();
}

void BLE::scanDevice(int scanTime, QString filter)
{
    BLENameDevID devArray[MAX_PATH];
    ULONG devNum = 16;
    char* pChar = new char[filter.length() + 1];
    strcpy_s(pChar, filter.length() + 1, filter.toLocal8Bit().data());
    memset(devArray, 0, sizeof(devArray));
    WCHBLEEnumDevice(scanTime, pChar, devArray, &devNum);
    scannedDev.clear();
    for(ULONG i = 0; i < devNum; i++)
    {
        BleDevInfo dev;
        dev.name =  QString::fromLocal8Bit(reinterpret_cast<const char*>(devArray[i].Name));
        dev.devID = QString::fromLocal8Bit(reinterpret_cast<const char*>(devArray[i].DevID));
        dev.rssi = devArray[i].Rssi;
        QRegularExpression re(".*-(.*)");
        QRegularExpressionMatch match = re.match(dev.devID);
        if (match.hasMatch()) {
            dev.mac = match.captured(1);
        }
        scannedDev << dev;
    }
    foreach(auto dev, scannedDev)
    {
        qDebug() << QString("%1 %2 %3dB").arg(dev.name).arg(dev.devID).arg(dev.rssi);
    }
}


void BLE::rssiNotify()
{
    WCHBLERegisterRSSINotify(rssiCallback);
}

bool BLE::connect()
{
    if(scannedDev.empty())
    {
        return false;
    }

    // 连接扫描到的BLE设备中信号最强的设备
    auto compare = [=](BleDevInfo &a, BleDevInfo &b) -> bool {
        return a.rssi < b.rssi;
    };
    BleDevInfo *maxRssiDev = std::max_element(scannedDev.begin(), scannedDev.end(), compare);

    return connect(maxRssiDev->devID);
}

bool BLE::connect(QString devID)
{
    for(int i = 0; i < scannedDev.size(); i++)
    {
        if (devID.compare(scannedDev[i].devID) == 0)
        {
            QByteArray ba = devID.toLocal8Bit();
            scannedDev[i].handle = WCHBLEOpenDevice(ba.data(), devConnChangedCallback);
            if(scannedDev[i].handle)
            {
                dev = &scannedDev[i];
                getAllServicesUUID();
                getMTU();
                getAllCharacteristicUUID();
                return true;
            }
            return false;
        }
    }

    return false;
}

void BLE::disconnect()
{
    if (isConnect())
    {
        WCHBLECloseDevice(dev->handle);
    }
}

bool BLE::isConnect()
{
    return dev->handle != NULL;
}


int BLE::rssi()
{
    return dev->rssi;
}

QString BLE::name()
{
    return dev->name;
}

QString BLE::devID()
{
    return dev->devID;
}

QString BLE::mac()
{
    return dev->mac;
}

quint16 BLE::mtu()
{
    return MTU;
}



QMap<USHORT, QVector<USHORT> > BLE::getAllUUID()
{
    return UUID;
}

QVector<USHORT> BLE::getServiceUUID()
{
    return UUID.keys().toVector();
}

QVector<USHORT> BLE::getCharacteristicUUID(quint16 service)
{
    return UUID.value(service);
}

Characteristic *BLE::getCharacteristic(quint16 serviceUUID, quint16 characteraisticUUID)
{
    for (int i = 0; i < characteristic.size(); i++)
    {
        if (characteristic[i]->serviceUUID == serviceUUID && characteristic[i]->characteristicUUID == characteraisticUUID)
        {
            return characteristic[i];
        }
    }
    return nullptr;
}



void BLE::devConnChangedCallback(void *hDev, UCHAR ConnectStatus)
{
    if(ConnectStatus == 0)
    {
        foreach (auto dev, scannedDev)
        {
            if (dev.handle == hDev)
            {
                dev.handle = 0;
                break;
            }
        }
    }
}

void BLE::readCallback(void *raramInf, PCHAR readBuf, ULONG readBufLen)
{
    foreach (auto c, characteristic) {

    }
}

void BLE::rssiCallback(PCHAR pMAC, int rssi)
{
    QByteArray macByteArray(reinterpret_cast<const char*>(pMAC), 6);
    // 将 QByteArray 转换为 QString
    QString macAddressString = macByteArray.toHex(':').toUpper(); // 添加冒号并转为大写

    foreach (auto dev, scannedDev)
    {
        if (compareMacAddresses(macAddressString, dev.mac))
        {
            dev.rssi = rssi;
            break;
        }
    }
}

bool BLE::compareMacAddresses(const QString &macAddress1, const QString &macAddress2)
{
    return macAddress1.compare(macAddress2, Qt::CaseInsensitive) == 0;
}

void BLE::getAllServicesUUID()
{
    USHORT UUIDArray[64];
    USHORT UUIDArrayLen = 64;
    int ret = WCHBLEGetAllServicesUUID(dev->handle, UUIDArray, &UUIDArrayLen);
    if (ret == 0)
    {
        for (int i = 0; i < UUIDArrayLen; i++)
        {
            QVector<USHORT> c;
            UUID.insert(UUIDArray[i], c);
        }
    }
}

void BLE::getAllCharacteristicUUID()
{
    foreach (auto &s, UUID.keys())
    {
        USHORT UUIDArray[64];
        USHORT UUIDArrayLen = 64;
        int ret = WCHBLEGetCharacteristicByUUID(dev->handle, s, UUIDArray, &UUIDArrayLen);
        if (ret == 0)
        {
            for (int i = 0; i < UUIDArrayLen; i++)
            {
                Characteristic *c = new Characteristic(this);
                c->characteristicUUID = UUIDArray[i];
                c->serviceUUID = s;
                ULONG action = 0;
                WCHBLEGetCharacteristicAction(dev->handle, s, c->characteristicUUID, &action);
                c->action = action;
                characteristic.append(c);
            }
        }
    }
}

void BLE::getMTU()
{
    WCHBLEGetMtu(dev->handle, &MTU);
}


