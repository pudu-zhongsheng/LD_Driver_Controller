#include "cl_twozerozeroacom.h"
#include <QDebug>
#include <cmath>

CL_TwoZeroZeroACOM::CL_TwoZeroZeroACOM(QWidget *parent) 
    : QWidget(parent)
{}

// Short通讯模式，获取设置指令的响应结果
CL_TwoZeroZeroACOM::ShortMeasurementData CL_TwoZeroZeroACOM::handleReceivedSettingResult(const QByteArray &data)
{
    ShortMeasurementData receivedData;
    receivedData.registerCode = data.mid(3,2);
    receivedData.err = data.at(6);

    return receivedData;
}

// Long通讯模式，处理接收到的数据
CL_TwoZeroZeroACOM::MeasurementData CL_TwoZeroZeroACOM::handleReceivedMeasurementData(const QByteArray &data)
{
    MeasurementData receivedData;
    receivedData.registerCode = data.mid(3,2);
    receivedData.fixedValue = data.at(5);
    receivedData.err = data.at(6);
    receivedData.rng = data.at(7);
    receivedData.ba = data.at(8);
    receivedData.data1 = parseLongDataBlock(data.mid(9,6));
    receivedData.data2 = parseLongDataBlock(data.mid(15,6));
    receivedData.data3 = parseLongDataBlock(data.mid(21,6));

    return receivedData;
}

// special通讯模式，处理接收到的测量数据
CL_TwoZeroZeroACOM::MeasurementData CL_TwoZeroZeroACOM::handleReceivedMeasurementDataSpecial(const QByteArray &data)
{
    MeasurementData receivedData;
    receivedData.registerCode = data.mid(3,2);
    receivedData.fixedValue = data.at(5);
    receivedData.err = data.at(6);
    receivedData.rng = data.at(7);
    receivedData.ba = data.at(8);
    receivedData.data1 = parseSpecialDataBlock(data.mid(9,8));
    receivedData.data2 = parseSpecialDataBlock(data.mid(17,8));
    receivedData.data3 = parseSpecialDataBlock(data.mid(25,8));

    return receivedData;
}

// 解析长报文的数据块
float CL_TwoZeroZeroACOM::parseLongDataBlock(const QByteArray &block)
{
    if(block.size() != 6) return 0.0f;  // 确保数据块长度正确

    // 解析符号
    char signChar = block.at(0);
    int sign = (signChar == '-') ? -1 : 1;

    // 解析有效数据
    QString valueStr = block.mid(1,4).trimmed();    // 提取并去除空格
    float value = valueStr.toFloat();

    // 解析指数并计算结果
    char exponentChar = block.at(5);
    int exponent = exponentChar - '0';   // 转换为0-9的整数
    float multiplier = std::pow(10,exponent - 4);   // 10的(exponent - 4)次方

    return sign * value * multiplier;
}

// 封装特殊报文
QByteArray createSpecialFormatMessage(float value1, float value2, float value3)
{
    QByteArray message;
    message.append(0x02);   // STX

    // 将每个浮点值转化为8字节的ASCII字符表示
    auto floatToAscii = [](float val) -> QByteArray {
        union{
            float f;
            uint32_t i;
        } converter;
        converter.f = val;
        QByteArray result;
        result.append(QString::number((converter.i >> 31) & 0x1, 16).toUpper().toLatin1()); // 符号位
        result.append(QString::number((converter.i >> 23) & 0xFF, 16).rightJustified(2,'0').toUpper().toLatin1());  // 指数
        result.append(QString::number(converter.i & 0x7FFFFF, 16).rightJustified(6,'0').toUpper().toLatin1());    // 尾数
        return  result;
    };

    message.append(floatToAscii(value1));
    message.append(floatToAscii(value2));
    message.append(floatToAscii(value3));

    return message;
}

// 解析特殊报文的数据块
float CL_TwoZeroZeroACOM::parseSpecialDataBlock(const QByteArray &block)
{
    if(block.size() != 8) return 0.0f;

    // 符号位
    int sign = (block[0] == '1') ? -1 : 1;

    // 指数部分（第2、3位）
    int exponent = block.mid(1,2).toInt(nullptr,16);

    // 尾数部分（第4-8位）
    int mantissa = block.mid(3,5).toInt(nullptr,16);

    // 构建浮点数
    float result = sign * mantissa * std::pow(2,exponent -127); // 2的(exponent - 127)次方
    return  result;
}

// 封装数据报文
QByteArray CL_TwoZeroZeroACOM::createMessage(const QByteArray &data)
{
    QByteArray sendData;
    // 序列化数据

    QDataStream stream(&sendData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);    // 设置数据流为大端序

    // STX 固定02h
    stream << quint8(0x02);

    // 数据
    stream.writeRawData(data.constData(),data.size());

    // ETX 固定03h
    stream << quint8(0x03);

    // BBC校验
    QByteArray dataToXOR;
    dataToXOR.append(data);
    dataToXOR.append(0x03);
    QString bcc = calculateBCC(dataToXOR);
//    qDebug() << "bccbccbccbccbccbccbcc:" << bcc.toUtf8().toHex();
    stream << quint8(bcc.toUtf8().at(0));
    stream << quint8(bcc.toUtf8().at(1));

    // 结尾CRLF
    stream << quint8(0x0D);
    stream << quint8(0x0A);

    qDebug() << "发送的报文为：" << sendData.toHex();
    return sendData;
}

// 计算校验位
QString CL_TwoZeroZeroACOM::calculateBCC(const QByteArray &data)
{
    quint8 bcc = 0;

    // 对QByteArray中的每个字节进行异或操作
    for(char byte : data){
        bcc ^= static_cast<quint8>(byte);
    }

    // 提取高低4位
    char highNibble = (bcc >> 4) < 10 ? (bcc >> 4) + '0' : (bcc >> 4) - 10 + 'A';
    char lowNibble = (bcc & 0x0F) < 10 ? (bcc & 0x0F) + '0' : (bcc & 0x0F) - 10 + 'A';

    QString bccResult;
    bccResult.append(highNibble);
    bccResult.append(lowNibble);

    return bccResult;
}

// 读取测量数据
QByteArray CL_TwoZeroZeroACOM::readMeasure(const QString &registCode, quint8 header, char CFable, char calibrationMode)
{
    QByteArray data;

    // header受体头“00”到“29”
    // 分别取到header的高低4位
    char highNibble = (header >> 4) < 10 ? (header >> 4) + '0' : (header >> 4) - 10 + 'A';
    char lowNibble = (header & 0x0F) < 10 ? (header & 0x0F) + '0' : (header & 0x0F) - 10 + 'A';

    // receptor head
    data.append(highNibble);
    data.append(lowNibble);

    // 功能码
    data.append(registCode.at(0));
    data.append(registCode.at(1));

    // 参数
    data.append(0x31);
    data.append(CFable);    // CF函数启用状态，2禁用，3启用
    data.append(0x30);
    data.append(calibrationMode);   // 校准模式，0规范，1多

    QByteArray sendData = createMessage(data);

    return sendData;
}

// 设置EXT模式（有应答）
QByteArray CL_TwoZeroZeroACOM::setEXT40(quint8 header)
{
    QByteArray data;
    char highNibble = (header >> 4) < 10 ? (header >> 4) + '0' : (header >> 4) - 10 + 'A';
    char lowNibble = (header & 0x0F) < 10 ? (header & 0x0F) + '0' : (header & 0x0F) - 10 + 'A';

    // receptor head
    data.append(highNibble);
    data.append(lowNibble);

    // 功能码
    data.append(0x34);
    data.append(0x30);

    // 参数
    data.append(0x31);
    data.append(0x30);
    data.append(0x20);
    data.append(0x20);

    QByteArray sendData = createMessage(data);

    return sendData;
}

// 进行EXT测量（无应答）
QByteArray CL_TwoZeroZeroACOM::takeEXT40()
{
    QByteArray data;

    // receptor head
    data.append(0x39);
    data.append(0x39);

    // 功能码
    data.append(0x34);
    data.append(0x30);

    // 参数
    data.append(0x32);
    data.append(0x31);
    data.append(0x20);
    data.append(0x20);

    QByteArray sendData = createMessage(data);

    return sendData;
}

// 读取用户校准系数
QByteArray CL_TwoZeroZeroACOM::readCalibrationFactor47(quint8 header, char MCF)
{
    QByteArray data;
    char highNibble = (header >> 4) < 10 ? (header >> 4) + '0' : (header >> 4) - 10 + 'A';
    char lowNibble = (header & 0x0F) < 10 ? (header & 0x0F) + '0' : (header & 0x0F) - 10 + 'A';

    // receptor head
    data.append(highNibble);
    data.append(lowNibble);

    // 功能码
    data.append(0x34);
    data.append(0x37);

    // 用户校准系数矩阵行号1-3
    // 第一行：A11 A12 A13；第二行：A21 A22 A23；第三行：A31 A32 A33；
    data.append(MCF);

    // 参数
    data.append(0x30);
    data.append(0x20);
    data.append(0x20);

    QByteArray sendData = createMessage(data);

    return sendData;
}

// 写入用户校准系数
QByteArray CL_TwoZeroZeroACOM::setCalibrationFactor48(quint8 header, char MCF,
                                           const QString &data1,
                                           const QString &data2,
                                           const QString &data3)
{
    QByteArray data;
    char highNibble = (header >> 4) < 10 ? (header >> 4) + '0' : (header >> 4) - 10 + 'A';
    char lowNibble = (header & 0x0F) < 10 ? (header & 0x0F) + '0' : (header & 0x0F) - 10 + 'A';

    // receptor head
    data.append(highNibble);
    data.append(lowNibble);

    // 功能码
    data.append(0x34);
    data.append(0x38);

    // 用户校准系数矩阵行号1-3
    // 第一行：A11 A12 A13；第二行：A21 A22 A23；第三行：A31 A32 A33；
    data.append(MCF);

    // 参数
    data.append(0x31);
    data.append(0x20);
    data.append(0x20);

    data.append(data1.toUtf8());
    data.append(data2.toUtf8());
    data.append(data3.toUtf8());

    QByteArray sendData = createMessage(data);

    return sendData;
}

// 设置PC连接模式
QByteArray CL_TwoZeroZeroACOM::setPCConnect54()
{
    QByteArray data;
    // receptor head
    data.append(0x30);
    data.append(0x30);

    // 功能码
    data.append(0x35);
    data.append(0x34);

    // 参数
    data.append(0x31);
    data.append(0x20);
    data.append(0x20);
    data.append(0x20);

    QByteArray sendData = createMessage(data);

    return sendData;
}

// 设置保持状态
QByteArray CL_TwoZeroZeroACOM::setHoldState55()
{
    QByteArray data;
    // receptor head
    data.append(0x39);
    data.append(0x39);

    // 功能码
    data.append(0x35);
    data.append(0x35);

    // 参数
    data.append(0x31);
    data.append(0x20);
    data.append(0x20);
    data.append(0x30);

    QByteArray sendData = createMessage(data);

    return sendData;
}
