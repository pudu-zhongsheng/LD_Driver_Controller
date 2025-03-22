#ifndef DRIVERGENERAL_H
#define DRIVERGENERAL_H

#include <QObject>
#include <QByteArray>

class DriverGeneral : public QObject
{
    Q_OBJECT
public:
    explicit DriverGeneral(QObject *parent = nullptr);

    struct ValidAction {    // 两个字节的bit对应十六种功能
        char SlaveAddress = 0x00;
        char LEDOn_Off = 0x00;
        char Temperature = 0x00;
        char LEDStrength = 0x00;
        char Power = 0x00;
        char LEDModer = 0x00;
        char LEDWorkTime = 0x00;
        // ...
    };

    struct DriverMessage {
        char ChannelCount = 0x00;
        ValidAction actionSign;
        quint32 minV = 0x00000000;
        quint32 maxV = 0x00000000;
        quint32 minA = 0x00000000;
        quint32 maxA = 0x00000000;
    };

    struct Temperatures {
        quint32 LEDTemperature = 0x00000000;
        quint32 PCBTemperature = 0x00000000;
    };

    struct ChannelValue {
        char startRegister = 0x00;
        char countRegister = 0x00;
        QList<quint16> chValue;
    };

    struct CurrentPower {
        quint32 nowVoltage;
        quint32 nowCurrent;
    };

    // 命令生成
    QByteArray makeCommand(quint8 commandLength, quint8 actionCategory,
                               quint8 sendAddress, quint8 receiveAddress,
                               quint8 actionAddress, QByteArray data);

    // 读指令
    QByteArray connectInit(quint8 sendAddress, quint8 receiveAddress);   // 0x08
    QByteArray readTemperature(quint8 sendAddress, quint8 receiveAddress);   // 0x1C
    QByteArray readLEDOnOff(quint8 sendAddress, quint8 receiveAddress);  // 0x24
    QByteArray readLEDStrength(quint8 sendAddress, quint8 receiveAddress,
                               quint8 startRegister, quint8 registerCount); // 0x26
    QByteArray readLEDModel(quint8 sendAddress, quint8 receiveAddress);  // 0x50
    QByteArray readLEDWorkTime(quint8 sendAddress, quint8 receiveAddress);   // 0x52
    QByteArray readVoltageCurrent(quint8 sendAddress, quint8 receiveAddress);// 0x56

    // 写指令
    QByteArray writeDriverAddress(quint8 sendAddress, quint8 receiveAddress,
                                  quint8 newAddress);   // 0x1B
    QByteArray writeLEDOnOff(quint8 sendAddress, quint8 receiveAddress,
                             quint16 LEDStatus);   // 0x24
    QByteArray writeLEDStrength(quint8 sendAddress, quint8 receiveAddress,
                                quint8 startRegister, quint8 registerCount,
                                QByteArray valuedata);   // 0x26
    QByteArray writeLEDModel(quint8 sendAddress, quint8 receiveAddress,
                             quint16 LEDModel);   // 0x50
    QByteArray writeLEDWorkTime(quint8 sendAddress, quint8 receiveAddress,
                                quint32 LEDWorkTime);   // 0x52
    QByteArray writeLimitVoltageCurrent(quint8 sendAddress, quint8 receiveAddress,
                                        quint32 MaxVoltage, quint32 MaxCurrent);   // 0x5E
    QByteArray writeClearAlarm(quint8 sendAddress, quint8 receiveAddress);  // 0x60

    // 数据解析
    ValidAction parseValidction(quint16 actionCode);
    DriverMessage parseInit(QByteArray data);
    char parseWrite1Byte(QByteArray data);
    quint16 parseWrite2Byte(QByteArray data);
    quint32 parseWrite4Byte(QByteArray data);
    Temperatures parseTemperature(QByteArray data);
    ChannelValue parseChannelValue(QByteArray data);
    CurrentPower parsePower(QByteArray data);

    // CRC校验
    static quint16 calculateCRC16(const QByteArray &data);


private:
    static const quint16 STX = 0x4C44;  // 起始字符
    static const char W_COM = 0x80;     // 写指令
    static const char R_COM = 0x81;     // 读指令

};

#endif // DRIVERGENERAL_H
