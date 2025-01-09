#include "driverprotocol.h"
#include <QDebug>

DriverProtocol::DriverProtocol(QObject *parent) : QObject(parent)
{
    // 初始化通道数据缓存
    m_channelData.resize(8);  // 8通道
}

QByteArray DriverProtocol::makeChannelCommand(int channel, int value, bool highLevel)
{
    // 构造命令数据
    QByteArray data;
    data.append(CMD_CHANNEL);
    data.append(QString::number(channel).toUtf8());
    data.append(',');
    data.append(QString::number(value).toUtf8());
    data.append(',');
    data.append(highLevel ? '1' : '0');
    
    return createMessage(data);
}

QByteArray DriverProtocol::makeAllChannelsCommand(int value, bool highLevel)
{
    // 构造命令数据
    QByteArray data;
    data.append(CMD_ALL);
    data.append(QString::number(value).toUtf8());
    data.append(',');
    data.append(highLevel ? '1' : '0');
    
    return createMessage(data);
}

QByteArray DriverProtocol::makeControlCommand(bool highLevel, int startReg, int regCount,
                                            int value1, int value2, int value3)
{
    // 构造命令数据
    QByteArray data;
    data.append(CMD_MULTI);
    data.append(QString::number(startReg).toUtf8());
    data.append(',');
    data.append(QString::number(regCount).toUtf8());
    data.append(',');
    data.append(highLevel ? '1' : '0');
    data.append(',');
    data.append(QString::number(value1).toUtf8());
    data.append(',');
    data.append(QString::number(value2).toUtf8());
    data.append(',');
    data.append(QString::number(value3).toUtf8());
    
    return createMessage(data);
}

bool DriverProtocol::parseResponse(const QByteArray &response, QByteArray &data)
{
    // 检查基本格式
    if (response.size() < 5 || response[0] != STX || 
        response[response.size()-2] != CR || response[response.size()-1] != LF) {
        qDebug() << "Invalid response format";
        return false;
    }
    
    // 提取数据部分(不包含STX和CR/LF)
    data = response.mid(1, response.size() - 3);
    
    // 验证校验和
    if (!verifyChecksum(data)) {
        qDebug() << "Checksum verification failed";
        return false;
    }
    
    // 移除ETX和校验和
    data = data.left(data.size() - 2);
    
    return true;
}

QByteArray DriverProtocol::createMessage(const QByteArray &data)
{
    QByteArray message;
    message.append(STX);
    message.append(data);
    message.append(ETX);
    
    // 计算并添加校验和
    message.append(calculateChecksum(message));
    
    // 添加结束符
    message.append(CR);
    message.append(LF);
    
    return message;
}

QByteArray DriverProtocol::calculateChecksum(const QByteArray &data)
{
    char sum = 0;
    for (char c : data) {
        sum ^= c;  // 异或校验和
    }
    return QByteArray(1, sum);
}

bool DriverProtocol::verifyChecksum(const QByteArray &data)
{
    if (data.size() < 2) return false;
    
    // 最后一个字节是校验和
    char receivedChecksum = data[data.size() - 1];
    
    // 计算除校验和外的数据的校验和
    QByteArray dataToVerify = data.left(data.size() - 1);
    char calculatedChecksum = calculateChecksum(dataToVerify)[0];
    
    return receivedChecksum == calculatedChecksum;
} 