#include "ble.h"
#include <QDebug>
#include <QRegularExpression>

QVector<BLE::BleDevInfo> BLE::bleDev;

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
    WCHBLEEnumDevice(scanTime, NULL, devArray, &devNum);
    qDebug() << "scaned" << devNum;
    bleDev.clear();
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
        bleDev << dev;
    }
    foreach(auto dev, bleDev)
    {
        qDebug() << QString("%1 %2 %3").arg(dev.name).arg(dev.devID).arg(dev.rssi);
    }
}

bool BLE::connectDevice(QString devID)
{

    for(int i = 0; i < bleDev.size(); i++)
    {
        if (devID.compare(bleDev[i].devID) == 0)
        {
            QByteArray ba = devID.toLocal8Bit();
            bleDev[i].handle = WCHBLEOpenDevice(ba.data(), devConnChangedCallback);
            return true;
        }
    }

    return false;
}

bool BLE::connectDevice()
{
    if(bleDev.empty())
    {
        return false;
    }

    // 连接扫描到的BLE设备中信号最强的设备
    auto compare = [=](BleDevInfo &a, BleDevInfo &b) -> bool {
        return a.rssi < b.rssi;
    };
    BleDevInfo *maxRssiDev = std::max_element(bleDev.begin(), bleDev.end(), compare);

    return connectDevice(maxRssiDev->devID);
}

void BLE::rssiNotify()
{
    WCHBLERegisterRSSINotify(rssiCallback);
}

void BLE::devConnChangedCallback(void *hDev, UCHAR ConnectStatus)
{
    if(ConnectStatus == 0)
    {
        foreach (auto dev, bleDev)
        {
            if (dev.handle == hDev)
            {
                dev.handle = 0;
                break;
            }
        }
    }
}

void BLE::rssiCallback(PCHAR pMAC, int rssi)
{
    QByteArray macByteArray(reinterpret_cast<const char*>(pMAC), 6);
    // 将 QByteArray 转换为 QString
    QString macAddressString = macByteArray.toHex(':').toUpper(); // 添加冒号并转为大写

    foreach (auto dev, bleDev)
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
