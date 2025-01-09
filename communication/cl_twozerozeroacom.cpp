#include "cl_twozerozeroacom.h"
#include <QDataStream>

CL_TwoZeroZeroACOM::CL_TwoZeroZeroACOM(QObject *parent) : Protocol(parent)
{
}

QByteArray CL_TwoZeroZeroACOM::makeReadCommand(int address, int count)
{
    Q_UNUSED(address)
    Q_UNUSED(count)
    // 照度计使用特定命令而不是地址读取
    return makeQueryCommand();
}

QByteArray CL_TwoZeroZeroACOM::makeWriteCommand(int address, const QByteArray &data)
{
    Q_UNUSED(address)
    Q_UNUSED(data)
    // 照度计使用特定命令而不是地址写入
    return QByteArray();
}

QByteArray CL_TwoZeroZeroACOM::makeCommand(char cmd, const QString &param)
{
    QByteArray data;
    data.append(STX);
    data.append(cmd);
    if (!param.isEmpty()) {
        data.append(param.toUtf8());
    }
    data.append(ETX);
    
    // 添加校验和
    data.append(calculateChecksum(data));
    
    // 添加结束符
    data.append(CR);
    data.append(LF);
    
    return data;
}

QByteArray CL_TwoZeroZeroACOM::makeHoldCommand(bool hold)
{
    return makeCommand(CMD_HOLD, hold ? "1" : "0");
}

QByteArray CL_TwoZeroZeroACOM::makeBacklightCommand(bool on)
{
    return makeCommand(CMD_BACKLIGHT, on ? "1" : "0");
}

QByteArray CL_TwoZeroZeroACOM::makeRangeCommand(int range)
{
    if (range < 0 || range > 4) {
        emit error("无效的量程设置");
        return QByteArray();
    }
    return makeCommand(CMD_RANGE, QString::number(range));
}

QByteArray CL_TwoZeroZeroACOM::makeQueryCommand()
{
    return makeCommand(CMD_QUERY);
}

QByteArray CL_TwoZeroZeroACOM::makeMaxMinCommand(const QString &mode)
{
    if (mode != "MAX" && mode != "MIN" && mode != "OFF") {
        emit error("无效的最大最小值模式");
        return QByteArray();
    }
    QString param = mode == "MAX" ? "1" : (mode == "MIN" ? "2" : "0");
    return makeCommand(CMD_MAXMIN, param);
}

QByteArray CL_TwoZeroZeroACOM::makePeakHoldCommand(bool on)
{
    return makeCommand(CMD_PEAK, on ? "1" : "0");
}

QByteArray CL_TwoZeroZeroACOM::makeRelativeCommand(bool on)
{
    return makeCommand(CMD_RELATIVE, on ? "1" : "0");
}

QByteArray CL_TwoZeroZeroACOM::makeEXTCommand(bool on)
{
    QByteArray data;
    if (on) {
        // 设置EXT模式
        data.append("00");  // receptor head
        data.append("40"); // 功能码
        data.append("1020"); // 参数
    } else {
        // 执行EXT测量
        data.append("99");  // receptor head
        data.append("40"); // 功能码
        data.append("2120"); // 参数
    }
    return createMessage(data);
}

QByteArray CL_TwoZeroZeroACOM::makePCConnectCommand()
{
    QByteArray data;
    data.append("00");  // receptor head
    data.append("54"); // 功能码
    data.append("1"); // 参数
    data.append("   "); // 空格填充
    return createMessage(data);
}

QByteArray CL_TwoZeroZeroACOM::makeCalibrationCommand(int row, const QString &data1,
                                                     const QString &data2, const QString &data3)
{
    if (row < 1 || row > 3) {
        emit error("校准系数行号无效");
        return QByteArray();
    }

    QByteArray data;
    data.append("00");  // receptor head
    data.append("48"); // 功能码
    data.append('0' + row); // 矩阵行号
    data.append("1  "); // 参数
    data.append(data1.toUtf8());
    data.append(data2.toUtf8());
    data.append(data3.toUtf8());
    
    return createMessage(data);
}

bool CL_TwoZeroZeroACOM::parseResponse(const QByteArray &response, QByteArray &data)
{
    // 检查最小长度
    if (response.size() < 6) {  // STX(1) + CMD(1) + DATA(1+) + ETX(1) + CHK(1) + CR(1)
        emit error("响应数据长度错误");
        return false;
    }

    // 检查起始和结束字符
    if (response[0] != STX || response[response.size()-2] != CR || 
        response[response.size()-1] != LF) {
        emit error("响应数据格式错误");
        return false;
    }

    // 检查校验和
    if (!verifyChecksum(response)) {
        emit error("校验和错误");
        return false;
    }

    // 提取数据部分（不包括STX、ETX、校验和和结束符）
    data = response.mid(2, response.size() - 6);
    return true;
}

bool CL_TwoZeroZeroACOM::parseMeasurementData(const QByteArray &data, MeasurementData &result)
{
    if (data.size() < 8) {
        emit error("测量数据长度错误");
        return false;
    }

    result.registerCode = data.mid(0, 2);
    result.fixedValue = data[2];
    result.err = data[3];
    result.rng = data[4];
    result.ba = data[5];

    // 处理设备错误
    if (result.err != ' ' && result.err != '0') {
        handleDeviceError(result.err);
        if (result.err >= '1' && result.err <= '3') {
            return false;
        }
    }

    // 检查电池状态
    if (result.ba == '1') {
        emit error("设备电池电量不足");
    }

    // 解析三个数据块
    QByteArray block1 = data.mid(6, 6);
    QByteArray block2 = data.mid(12, 6);
    QByteArray block3 = data.mid(18, 6);

    result.data1 = parseDataBlock(block1);
    result.data2 = parseDataBlock(block2);
    result.data3 = parseDataBlock(block3);
    
    result.time = std::chrono::system_clock::now();

    return true;
}

float CL_TwoZeroZeroACOM::parseDataBlock(const QByteArray &block)
{
    if (block.size() != 6) return 0.0f;

    // 解析符号
    char signChar = block[0];
    int sign = (signChar == '-') ? -1 : 1;

    // 解析有效数据
    QString valueStr = block.mid(1, 4).trimmed();
    float value = valueStr.toFloat();

    // 解析指数并计算结果
    char exponentChar = block[5];
    int exponent = exponentChar - '0';
    float multiplier = std::pow(10, exponent - 4);

    return sign * value * multiplier;
}

QByteArray CL_TwoZeroZeroACOM::createMessage(const QByteArray &data)
{
    QByteArray message;
    message.append(STX);
    message.append(data);
    message.append(ETX);

    // 计算并添加校验和
    QByteArray dataToXOR = message;
    message.append(calculateChecksum(dataToXOR));

    // 添加结束符
    message.append(CR);
    message.append(LF);

    return message;
}

QByteArray CL_TwoZeroZeroACOM::calculateChecksum(const QByteArray &data)
{
    // 计算校验和：从STX到ETX的所有字节的异或值
    char checksum = 0;
    for (char byte : data) {
        checksum ^= byte;
    }
    return QByteArray(1, checksum);
}

bool CL_TwoZeroZeroACOM::verifyChecksum(const QByteArray &response)
{
    // 提取数据部分（包括STX到ETX）
    QByteArray data = response.left(response.size() - 3);
    // 提取校验和
    char receivedChecksum = response[response.size() - 3];
    // 计算校验和
    char calculatedChecksum = calculateChecksum(data)[0];
    
    return receivedChecksum == calculatedChecksum;
}

bool CL_TwoZeroZeroACOM::validateResponse(const QByteArray &response)
{
    if (response.size() < 6) {
        emit error("响应数据长度不足");
        return false;
    }

    if (response[0] != STX) {
        emit error("无效的起始字符");
        return false;
    }

    if (response[response.size()-2] != CR || response[response.size()-1] != LF) {
        emit error("无效的结束字符");
        return false;
    }

    return true;
}

QString CL_TwoZeroZeroACOM::getErrorString(char errorCode)
{
    switch (errorCode) {
        case '0':
        case ' ':
            return "正常";
        case '1':
        case '2':
        case '3':
            return "设备需要重启";
        case '4':
            return "请先设置为保持状态";
        case '5':
            return "测量值超出范围";
        case '6':
        case '7':
            return "可能存在异常";
        default:
            return "未知错误";
    }
}

bool CL_TwoZeroZeroACOM::isValidRange(int range) const
{
    return range >= 0 && range <= 4;
}

void CL_TwoZeroZeroACOM::handleDeviceError(char errorCode)
{
    QString errorMsg = getErrorString(errorCode);
    emit error(QString("设备错误: %1").arg(errorMsg));
    
    // 对于需要重启的错误，发送特殊信号
    if (errorCode >= '1' && errorCode <= '3') {
        emit error("设备需要重启，请重新启动CL-200A");
    }
}
