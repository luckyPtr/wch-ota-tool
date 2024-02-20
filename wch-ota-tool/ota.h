
#ifndef OTA_H
#define OTA_H


#include <QObject>
#include "ble.h"

class OTA : public QObject
{
    Q_OBJECT

    enum CMD_OTA
    {
        CMD_OTA_PROM = 0x80,
        CMD_OTA_ERASE,
        CMD_OTA_VERIFY,
        CMD_OTA_END,
        CMD_OTA_INFO,
    };

    BLE *ble;
    Characteristic *charOTA;
public:
    explicit OTA(QObject *parent = nullptr);

    enum
    {
        OK,
        ERR,
    };
    void setCharacteristic(Characteristic *c);
    int erase(quint32 addr, quint32 size);
    int program(quint32 startAddr, QByteArray file, bool quick = false);
    int verify(quint32 startAddr, QByteArray file);
    int run();
signals:
    void downloadProgressChange(quint8);
    void verifyProgressChange(quint8);
};

#endif // OTA_H
