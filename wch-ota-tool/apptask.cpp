
#include "apptask.h"
#include <QCoreApplication>
#include <QRegularExpression>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "ble.h"
#include "ota.h"
#include <iostream>
#include <QThread>

int AppTask::StringToInt(QString str)
{
    QRegularExpression reHex("0x([0-9A-Fa-f]+)");
    QRegularExpression reDec("[0-9]+");
    QRegularExpressionMatch matchHex = reHex.match(str);
    QRegularExpressionMatch matchDec = reDec.match(str);

    // 输入16进制
    if (matchHex.hasMatch())
    {
        return matchHex.captured(1).toInt(nullptr, 16);
    }
    else if (matchDec.hasMatch())
    {
        return matchDec.captured(0).toInt(nullptr, 10);
    }

    return 0;
}

void AppTask::printDevices()
{
    if (BLE::devices.size() == 0)
    {
        qDebug() << "No BLE Device Found";
    }
    else
    {
        qDebug() << qUtf8Printable(QString("Found %1 BLE device(s):").arg(BLE::devices.size()));
        int maxNameLen = 0;
        foreach (auto dev, BLE::devices)
        {
            if (dev.name.size() > maxNameLen)
                maxNameLen = dev.name.size();
        }

        foreach (auto dev, BLE::devices)
        {
            QString space(maxNameLen - dev.name.size(), ' ');
            qDebug() << qUtf8Printable(QString("  %1%2   %3  %4dBm").arg(dev.name).arg(space).arg(dev.mac).arg(dev.rssi));
        }
    }
    qDebug().noquote() << "";
}

AppTask::AppTask(QObject *parent)
    : QObject{parent}
{
    connect(&timer, &QTimer::timeout, this, [&](){
        qApp->exit(0);
    });
}

void AppTask::run()
{
    QElapsedTimer elapsedTimer;

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    elapsedTimer.start();

    // 扫描附近的设备
    QCommandLineOption opScan("s", "Scan nearby BLE devices.");
    parser.addOption(opScan);

    // 扫描的过滤选项
    QCommandLineOption opFilter("filter", "Filter scanned BLE devices.");
    opFilter.setValueName("Filter");
    opFilter.setDefaultValue(NULL);
    parser.addOption(opFilter);

    // 扫描时间
    QCommandLineOption opTimeout("timeout", "Connection timeout.");
    opTimeout.setValueName("Timeout");
    opTimeout.setDefaultValue("1000");
    parser.addOption(opTimeout);

    // 连接设备
    QCommandLineOption opConn("c", "Establish connection to the device.");
    parser.addOption(opConn);

    // 擦除
    QCommandLineOption opErase("e", "Erase the specified sectors identified by sectors codes. ex: -e 0,4 to erase sectors 0 to 4.");
    opErase.setValueName("Erase");
    parser.addOption(opErase);

    // 烧录
    QCommandLineOption opDownload("d", "Download the content of a file into device memory.");
    opDownload.setValueName("FilePath");
    parser.addOption(opDownload);

    // 烧录地址
    QCommandLineOption opAddr("addr", "Start address of download.");
    opAddr.setValueName("Address");
    opAddr.setDefaultValue("0x0000");
    parser.addOption(opAddr);

    // 校验
    QCommandLineOption opVerify("verify", "Verify if the programming operation is achieved successfully.");
    parser.addOption(opVerify);

    // 运行
    QCommandLineOption opRun("r", "Run the code.");
    parser.addOption(opRun);


    parser.process(*qApp);
    BLE *ble = new BLE;
    Characteristic *characteristicOTA;
    OTA ota;
    QByteArray image;
    BLE::Init();

    qDebug() << "-------------------------------------";
    qDebug() << "         WCH_OTA_Tool v1.0.0         ";
    qDebug() << "-------------------------------------";

    if (parser.isSet(opDownload))
    {
        QFile imgFile(parser.value(opDownload));
        qDebug() << "Opening and parsing file:" << imgFile.fileName();
        if (imgFile.open(QIODevice::ReadOnly))
        {
            image = imgFile.readAll();//读取二进制数据
            imgFile.close();//关闭文件

            qDebug() << qUtf8Printable(QString("  File   : %1").arg(QFileInfo(imgFile).fileName()));
            qDebug() << qUtf8Printable(QString("  Size   : %1 KB").arg(QString::number((qreal)image.size() / 1024, 'f', 2)));
            qDebug() << qUtf8Printable(QString("  Adress : 0x%1").arg(QString::number(StringToInt(parser.value(opAddr)), 16).toUpper()));
        }
        else
        {
            qDebug() << "Open file failed";
            goto failure;
        }
        qDebug().noquote() << "";
    }

    if (parser.isSet(opScan))
    {
        qDebug() << "Scanning BLE devices...";
        int timeout = parser.value(opTimeout).toInt();
        QString filter = parser.value(opFilter);
        BLE::scanDevice(timeout, filter);
        printDevices();

        if (BLE::devices.size() == 0)
        {
            goto failure;
        }
    }
    if (parser.isSet(opConn))
    {
        qDebug() << "Connecting...";
        if (ble->connect())
        {
            qDebug() << "Connect successfully\n";
            characteristicOTA = ble->getCharacteristic(0xFEE0, 0xFEE1);
            ota.setCharacteristic(characteristicOTA);
        }
        else
        {
            qDebug() << "Connect failed\n";
            goto failure;
        }
    }
    if (parser.isSet(opErase))
    {
        QStringList parts = parser.value(opErase).split(',');
        if (parts.size() == 2 || parts.size() == 1)
        {
            quint32 startAddr = StringToInt(parts.at(0));
            quint32 size = (image.size() + 4095) / 4096;
            if (parts.size() == 2)  // 如果指定了擦除范围
            {
                size = StringToInt(parts.at(1)) - startAddr;
            }

            qDebug() << qUtf8Printable(QString("Erasing sectors [%1,%2]").arg(startAddr).arg(startAddr + size));
            if(ota.erase(startAddr, size) == 0)
            {
                qDebug() << "Erase successfully\n";
            }
            else
            {
                qDebug() << "Erase failed\n";
                goto failure;
            }
        }
        else
        {
            goto failure;
        }
    }
    if (parser.isSet(opDownload))
    {
        QObject::connect(&ota, &OTA::downloadProgressChange, [&](quint8 percent){
            qDebug() << qUtf8Printable(QString::asprintf("  Downloading...%3d%", percent));
            if (percent == 100)
            {
                qDebug() << "File download complete\n";
            }
        });

        qDebug() << "Download in Progress:";
        int ret = ota.program(StringToInt(parser.value(opAddr)), image);
        if (ret == OTA::ERR)
        {
            qDebug() << "Download failed";
            goto failure;
        }
    }

    if (parser.isSet(opVerify))
    {
        QObject::connect(&ota, &OTA::verifyProgressChange, [&](quint8 percent){
            qDebug() << qUtf8Printable(QString::asprintf("  Verifying...%3d%", percent));
            if (percent == 100)
            {
                qDebug() << "File verify complete\n";
            }
        });

        qDebug() << "Verify in Progress:";
        int ret = ota.verify(StringToInt(parser.value(opAddr)), image);
        if (ret == OTA::ERR)
        {
            qDebug() << "Verify failed";
            goto failure;
        }
    }

    if (parser.isSet(opRun))
    {
        qDebug() << "Application is running...\n";
        ota.run();
    }

failure:
    qDebug() << qUtf8Printable(QString::asprintf("Time elasped %d ms", elapsedTimer.elapsed()));
    qDebug() << "Exit\n";
}

void AppTask::quit()
{
    timer.start(0);
}

