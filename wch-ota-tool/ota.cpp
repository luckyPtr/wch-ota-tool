
#include "ota.h"
#include <QDebug>
#include <QElapsedTimer>

OTA::OTA(QObject *parent)
    : QObject{parent}
{

}

void OTA::setCharacteristic(Characteristic *c)
{
    charOTA = c;
}


int OTA::erase(quint32 addr, quint32 size)
{
    QByteArray ba;
    ba.append((quint8)CMD_OTA_ERASE);
    ba.append(0x04);
    ba.append((quint8)(addr / 16));
    ba.append((quint8)(addr / 16 >> 8));
    ba.append((quint8)(size / 4096));
    ba.append((quint8)((size / 4096) >> 8));
    QByteArray ret = charOTA->writeAndRead(ba);
    if (ret.at(0) == 0)
    {
        return OK;
    }
    return ERR;
}

int OTA::program(quint32 startAddr, QByteArray file, bool quick)
{
    quint32 index = 0;  // 文件数据地址
    quint8 percent = 0;

    QElapsedTimer timer;
    timer.start();
    while(index < file.size())
    {
        quint32 size = 240;
        if (file.size() - index < size)
        {
            size = file.size() - index;
        }

        QByteArray ba;
        ba.append((quint8)CMD_OTA_PROM);
        ba.append(size);
        ba.append((quint8)((startAddr + index) / 16));
        ba.append((quint8)((startAddr + index) / 16 >> 8));
        ba.append(file.mid(index, size));

        //QByteArray ret =
            charOTA->write(ba, !quick);
 //       if (ret.at(0) == 0)
        {
            index += size;
        }
//        else
//        {
//            return ERR;
//        }
        if (timer.elapsed() >= 500)
        {
            timer.restart();
            percent = static_cast<quint8>(((qreal)index / file.size() * 100));
            emit downloadProgressChange(percent);
        }
    }

    if (percent != 100)
    {
        percent = 100;
        emit downloadProgressChange(percent);
    }

    return OK;
}

int OTA::verify(quint32 startAddr, QByteArray file)
{
    quint32 index = 0;  // 文件数据地址
    quint8 percent = 0;

    QElapsedTimer timer;
    timer.start();
    while(index < file.size())
    {
        quint32 size = 240;
        if (file.size() - index < size)
        {
            size = file.size() - index;
        }

        QByteArray ba;
        ba.append((quint8)CMD_OTA_VERIFY);
        ba.append(size);
        ba.append((quint8)((startAddr + index) / 16));
        ba.append((quint8)((startAddr + index) / 16 >> 8));
        ba.append(file.mid(index, size));

        QByteArray ret = charOTA->writeAndRead(ba);
        if (ret.at(0) == 0)
        {
            index += size;
        }
        else
        {
            return ERR;
        }
        if (timer.elapsed() >= 500)
        {
            timer.restart();
            percent = static_cast<quint8>(((qreal)index / file.size() * 100));
            emit verifyProgressChange(percent);
        }
    }

    if (percent != 100)
    {
        percent = 100;
        emit verifyProgressChange(percent);
    }

    return OK;
}

int OTA::run()
{
    QByteArray ba;
    ba.append((quint8)CMD_OTA_END);
    ba.append('\0');
    ba.append('\0');
    ba.append('\0');
    charOTA->write(ba);
    return OK;
}

