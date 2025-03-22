#include "protocol.h"

quint16 Protocol::calculateCRC16(const QByteArray &data)
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

// 基类中的纯虚函数不需要实现
// makeReadCommand, makeWriteCommand, parseResponse 由子类实现 
