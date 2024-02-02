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

void BLE::scanDevice(int scanTime, QString filter = NULL)
{
    BLENameDevID devArray[128];
    ULONG devNum;
    char* pChar = new char[filter.length() + 1];
    strcpy_s(pChar, filter.length() + 1, filter.toLocal8Bit().data());
    WCHBLEEnumDevice(scanTime, pChar, devArray, &devNum);

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

        qDebug() << dev.rssi;
    }
}

bool BLE::connectDevice(QString devID)
{
    WCHBLEHANDLE handle = 0;
    for(int i = 0; i < bleDev.size(); i++)
    {
        QByteArray ba = devID.toLocal8Bit();
        handle = WCHBLEOpenDevice(ba.data(), devConnChangedCallback);
        bleDev[i].handle = handle;
        break;
    }
    return handle != 0;
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
    qDebug() << "handle:" << hDev << "   conn:" << ConnectStatus;
}

void BLE::rssiCallback(PCHAR pMAC, int rssi)
{
    qDebug() << rssi;
}
