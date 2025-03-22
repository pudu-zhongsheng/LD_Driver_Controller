#include "drivergeneral.h"
#include <QtEndian>
#include <QIODevice>
#include <QDataStream>
#include <QDebug>

DriverGeneral::DriverGeneral(QObject *parent)
    :QObject(parent)
{

}

QByteArray DriverGeneral::makeCommand(quint8 commandLength, quint8 actionCategory,
                                          quint8 sendAddress, quint8 receiveAddress,
                                          quint8 actionAddress, QByteArray data)
{
    // 构造命令数据
    QByteArray sendData;
    // 序列化数据
    QDataStream stream(&sendData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);    // 设置数据流为大端序
    stream << qToBigEndian(static_cast<quint16>(STX));
    stream << commandLength;
    stream << actionCategory;
    stream << sendAddress;
    stream << receiveAddress;
    stream << actionAddress;
    if(!data.isEmpty()){
        stream.writeRawData(data.constData(),data.size());
    }
    // 计算并添加校验位
    quint16 crcCode = calculateCRC16(sendData.mid(2));
    stream << qToBigEndian(static_cast<quint16>(crcCode));
    qDebug() << "Send driverGeneral data is : " << sendData.toHex();
    return sendData;
}

QByteArray DriverGeneral::connectInit(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    data.append(static_cast<quint16>(0x0002));
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x08,data);
}

QByteArray DriverGeneral::readTemperature(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    data.append(static_cast<quint16>(0x0002));
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x1C,data);
}

QByteArray DriverGeneral::readLEDOnOff(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    data.append(static_cast<quint16>(0x0002));
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x24,data);
}

QByteArray DriverGeneral::readLEDStrength(quint8 sendAddress, quint8 receiveAddress,
                           quint8 startRegister, quint8 registerCount)
{
    QByteArray data;
    data.append(startRegister);
    data.append(registerCount);
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x26,data);
}

QByteArray DriverGeneral::readLEDModel(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    data.append(static_cast<quint16>(0x0002));
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x50,data);
}

QByteArray DriverGeneral::readLEDWorkTime(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    data.append(static_cast<quint16>(0x0004));
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x52,data);
}

QByteArray DriverGeneral::readVoltageCurrent(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    data.append(static_cast<quint16>(0x0002));
    return makeCommand(0x06,0x81,sendAddress,receiveAddress,0x56,data);
}

QByteArray DriverGeneral::writeDriverAddress(quint8 sendAddress, quint8 receiveAddress,
                                             quint8 newAddress)
{
    QByteArray data;
    data.append(newAddress);
    return makeCommand(0x05,0x80,sendAddress,receiveAddress,0x1B,data);
}

QByteArray DriverGeneral::writeLEDOnOff(quint8 sendAddress, quint8 receiveAddress,
                                        quint16 LEDStatus)
{
    QByteArray data;
    data.append(static_cast<quint16>(LEDStatus));
    return makeCommand(0x06,0x80,sendAddress,receiveAddress,0x24,data);
}

QByteArray DriverGeneral::writeLEDStrength(quint8 sendAddress, quint8 receiveAddress,
                                           quint8 startRegister, quint8 registerCount,
                                           QByteArray valuedata)
{
    QByteArray data;
    quint8 conLength = 4 + 2 + valuedata.size();
    data.append(startRegister);
    data.append(registerCount);
    data.append(valuedata);
    return makeCommand(conLength,0x80,sendAddress,receiveAddress,0x26,data);
}

QByteArray DriverGeneral::writeLEDModel(quint8 sendAddress, quint8 receiveAddress,
                                        quint16 LEDModel)
{
    QByteArray data;
    data.append(static_cast<quint16>(LEDModel));
    return makeCommand(0x06,0x80,sendAddress,receiveAddress,0x50,data);
}

QByteArray DriverGeneral::writeLEDWorkTime(quint8 sendAddress, quint8 receiveAddress,
                                           quint32 LEDWorkTime)
{
    QByteArray data;
    data.append(static_cast<quint32>(LEDWorkTime));
    return makeCommand(0x08,0x80,sendAddress,receiveAddress,0x52,data);
}

QByteArray DriverGeneral::writeLimitVoltageCurrent(quint8 sendAddress, quint8 receiveAddress,
                                                   quint32 MaxVoltage, quint32 MaxCurrent)
{
    QByteArray data;
    data.append(static_cast<quint32>(MaxVoltage));
    data.append(static_cast<quint32>(MaxCurrent));
    return makeCommand(0x0C,0x80,sendAddress,receiveAddress,0x5E,data);
}

QByteArray DriverGeneral::writeClearAlarm(quint8 sendAddress, quint8 receiveAddress)
{
    QByteArray data;
    quint16 clearSign = 0xFFFF;
    data.append(static_cast<quint16>(clearSign));
    return makeCommand(0x06,0x80,sendAddress,receiveAddress,0x60,data);
}

DriverGeneral::ValidAction DriverGeneral::parseValidction(quint16 actionCode)
{
    ValidAction action;
    
    // 按位解析功能标记位
    // 高字节(第二个字节)
    action.SlaveAddress = (actionCode & 0x8000) ? 0x01 : 0x00;  // bit15
    action.LEDOn_Off = (actionCode & 0x4000) ? 0x01 : 0x00;     // bit14
    action.Temperature = (actionCode & 0x2000) ? 0x01 : 0x00;    // bit13
    action.LEDStrength = (actionCode & 0x1000) ? 0x01 : 0x00;   // bit12
    action.Power = (actionCode & 0x0800) ? 0x01 : 0x00;         // bit11
    action.LEDModer = (actionCode & 0x0400) ? 0x01 : 0x00;      // bit10
    action.LEDWorkTime = (actionCode & 0x0200) ? 0x01 : 0x00;   // bit9
    // bit8 预留
    
    // 低字节(第一个字节)
    // bit7-bit0 预留
    
    return action;
}

DriverGeneral::DriverMessage DriverGeneral::parseInit(QByteArray data)
{
    DriverMessage mes;
    
    if (data.size() < 19) {
        qDebug() << "Invalid init data length:" << data.size();
        return mes;
    }
    
    // 解析通道数 (1字节)
    mes.ChannelCount = static_cast<char>(data[0]);
    
    // 解析功能位 (2字节)
    quint16 actionCode = (static_cast<quint8>(data[1]) << 8) | static_cast<quint8>(data[2]);
    mes.actionSign = parseValidction(actionCode);
    
    // 解析最小电压 (4字节)
    mes.minV = (static_cast<quint32>(data[3]) << 24) |
               (static_cast<quint32>(data[4]) << 16) |
               (static_cast<quint32>(data[5]) << 8) |
               static_cast<quint32>(data[6]);
               
    // 解析最大电压 (4字节)
    mes.maxV = (static_cast<quint32>(data[7]) << 24) |
               (static_cast<quint32>(data[8]) << 16) |
               (static_cast<quint32>(data[9]) << 8) |
               static_cast<quint32>(data[10]);
               
    // 解析最小电流 (4字节)
    mes.minA = (static_cast<quint32>(data[11]) << 24) |
               (static_cast<quint32>(data[12]) << 16) |
               (static_cast<quint32>(data[13]) << 8) |
               static_cast<quint32>(data[14]);
               
    // 解析最大电流 (4字节)
    mes.maxA = (static_cast<quint32>(data[15]) << 24) |
               (static_cast<quint32>(data[16]) << 16) |
               (static_cast<quint32>(data[17]) << 8) |
               static_cast<quint32>(data[18]);
    
    return mes;
}

char DriverGeneral::parseWrite1Byte(QByteArray data)
{
    if (data.size() < 1) {
        qDebug() << "Invalid data length for 1 byte parsing";
        return 0x00;
    }
    return static_cast<char>(data[0]);
}

quint16 DriverGeneral::parseWrite2Byte(QByteArray data)
{
    if (data.size() < 2) {
        qDebug() << "Invalid data length for 2 bytes parsing";
        return 0x0000;
    }
    
    // 高字节在前,低字节在后
    return (static_cast<quint16>(data[0]) << 8) | 
            static_cast<quint16>(data[1]);
}

quint32 DriverGeneral::parseWrite4Byte(QByteArray data)
{
    if (data.size() < 4) {
        qDebug() << "Invalid data length for 4 bytes parsing";
        return 0x00000000;
    }
    
    // 高字节在前,低字节在后
    return (static_cast<quint32>(data[0]) << 24) |
           (static_cast<quint32>(data[1]) << 16) |
           (static_cast<quint32>(data[2]) << 8) |
            static_cast<quint32>(data[3]);
}

DriverGeneral::Temperatures DriverGeneral::parseTemperature(QByteArray data)
{
    Temperatures temps;
    if (data.size() < 8) {
        qDebug() << "Invalid data length for temperature parsing";
        return temps;
    }
    
    // 解析LED温度(前4字节)
    temps.LEDTemperature = (static_cast<quint32>(data[0]) << 24) |
                          (static_cast<quint32>(data[1]) << 16) |
                          (static_cast<quint32>(data[2]) << 8) |
                           static_cast<quint32>(data[3]);
                           
    // 解析PCB温度(后4字节)                       
    temps.PCBTemperature = (static_cast<quint32>(data[4]) << 24) |
                          (static_cast<quint32>(data[5]) << 16) |
                          (static_cast<quint32>(data[6]) << 8) |
                           static_cast<quint32>(data[7]);
                           
    return temps;
}

DriverGeneral::ChannelValue DriverGeneral::parseChannelValue(QByteArray data)
{
    ChannelValue chValue;
    if (data.size() < 2) {
        qDebug() << "Invalid data length for channel value parsing";
        return chValue;
    }
    
    // 解析起始寄存器和寄存器数量
    chValue.startRegister = static_cast<char>(data[0]);
    chValue.countRegister = static_cast<char>(data[1]);
    
    // 解析通道值(每个通道2字节)
    int valueCount = (data.size() - 2) / 2;  // 计算有多少个通道值
    for (int i = 0; i < valueCount; i++) {
        quint16 value = (static_cast<quint16>(data[2 + i*2]) << 8) |
                        static_cast<quint16>(data[3 + i*2]);
        chValue.chValue.append(value);
    }
    
    return chValue;
}

DriverGeneral::CurrentPower DriverGeneral::parsePower(QByteArray data)
{
    CurrentPower power;
    if (data.size() < 8) {
        qDebug() << "Invalid data length for power parsing";
        return power;
    }
    
    // 解析电压值(前4字节)
    power.nowVoltage = (static_cast<quint32>(data[0]) << 24) |
                      (static_cast<quint32>(data[1]) << 16) |
                      (static_cast<quint32>(data[2]) << 8) |
                       static_cast<quint32>(data[3]);
                       
    // 解析电流值(后4字节)
    power.nowCurrent = (static_cast<quint32>(data[4]) << 24) |
                      (static_cast<quint32>(data[5]) << 16) |
                      (static_cast<quint32>(data[6]) << 8) |
                       static_cast<quint32>(data[7]);
                       
    return power;
}

quint16 DriverGeneral::calculateCRC16(const QByteArray &data)
{
    quint16 crc = 0x4c44;
    for (char byte : data) {
        crc ^= (quint8)byte;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}