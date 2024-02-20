
#include "apptask.h"
#include <QCoreApplication>
#include <QRegularExpression>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QFile>
#include <QDebug>
#include "ble.h"
#include "ota.h"

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
        return matchHex.captured(1).toInt(nullptr, 10);
    }

    return 0;
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
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    // 扫描附近的设备
    QCommandLineOption opScan("s");
    parser.addOption(opScan);

    // 扫描的过滤选项
    QCommandLineOption opFliter("fliter");
    opFliter.setValueName("Fliter");
    opFliter.setDefaultValue(NULL);
    parser.addOption(opFliter);

    // 扫描时间
    QCommandLineOption opTimeout("timeout");
    opTimeout.setValueName("Timeout");
    opTimeout.setDefaultValue("1000");
    parser.addOption(opTimeout);

    // 连接设备
    QCommandLineOption opConn("c");
    parser.addOption(opConn);

    // 擦除
    QCommandLineOption opErase("e");
    opErase.setValueName("Erase");
    parser.addOption(opErase);

    // 烧录
    QCommandLineOption opDownload("d");
    opDownload.setValueName("File");
    parser.addOption(opDownload);

    // 烧录地址
    QCommandLineOption opAddr("addr");
    opAddr.setValueName("Addr");
    opAddr.setDefaultValue("0x0000");
    parser.addOption(opAddr);

    // 校验
    QCommandLineOption opVerify("verify");
    parser.addOption(opVerify);

    // 运行
    QCommandLineOption opRun("r");
    parser.addOption(opRun);

    // 快速烧录
    QCommandLineOption opQuick("q");
    parser.addOption(opQuick);

    parser.process(*qApp);
    BLE *ble = new BLE;
    Characteristic *characteristicOTA;
    OTA ota;
    BLE::Init();
    if (parser.isSet(opScan))
    {
        qDebug() << "-------------------------------------";
        qDebug() << "         WCH_OTA_Tool v1.0.0         ";
        qDebug() << "-------------------------------------";
        qDebug() << "Scanning...";
        int timeout = parser.value(opTimeout).toInt();
        QString fliter = parser.value(opFliter);
        BLE::scanDevice(timeout, fliter);

        qDebug() << qUtf8Printable(QString("Found %1 BLE devices!").arg(BLE::devices.size()));
        foreach (auto dev, BLE::devices)
        {
            qDebug() << qUtf8Printable(QString("%1  %2  %3").arg(dev.name).arg(dev.mac).arg(dev.rssi));
        }

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
            qDebug() << "Connect successfully!";
            characteristicOTA = ble->getCharacteristic(0xFEE0, 0xFEE1);
            ota.setCharacteristic(characteristicOTA);
        }
        else
        {
            qDebug() << "Connect failed!";
            goto failure;
        }
    }
    if (parser.isSet(opErase))
    {
        QStringList parts = parser.value(opErase).split(',');
        if (parts.size() == 2)
        {
            quint32 startAddr = StringToInt(parts.at(0));
            quint32 size = StringToInt(parts.at(1));
            qDebug() << "Erasing...";
            if(ota.erase(startAddr, size) == 0)
            {
                qDebug() << "Erase finished!";
            }
            else
            {
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
        QObject::connect(&ota, &OTA::downloadProgressChange, [=](quint8 percent){
            qDebug() << qUtf8Printable(QString::asprintf("Downloading...%3d%", percent));
            if (percent == 100)
            {
                qDebug() << "Download successfully!";
            }
        });

        QFile imgFile(parser.value(opDownload));
        if (imgFile.open(QIODevice::ReadOnly))
        {
            QByteArray image = imgFile.readAll();//读取二进制数据
            qDebug() << qUtf8Printable(QString("Read file:%1    Size:0x%2").arg(imgFile.fileName()).arg(QString::number(image.size(), 16).toUpper()));
            imgFile.close();//关闭文件
            qDebug() << "Start program...";
            bool quick = parser.isSet(opQuick);
            ota.program(StringToInt(parser.value(opAddr)), image, quick);
        }
        else
        {
            goto failure;
        }
    }

    if (parser.isSet(opRun))
    {
        qDebug() << "Application running...";
        ota.run();
    }

failure:
    qDebug() << "Exit.";
}

void AppTask::quit()
{
    timer.start(0);
}

