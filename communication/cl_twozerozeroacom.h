#ifndef CL_TWOZEROZEROACOM_H
#define CL_TWOZEROZEROACOM_H

#include "protocol.h"
#include <QDateTime>
#include <chrono>
#include <cmath>

class CL_TwoZeroZeroACOM : public Protocol
{
    Q_OBJECT
public:
    explicit CL_TwoZeroZeroACOM(QObject *parent = nullptr);

    // 实现基本命令接口
    QByteArray makeReadCommand(int address, int count = 1) override;
    QByteArray makeWriteCommand(int address, const QByteArray &data) override;
    bool parseResponse(const QByteArray &response, QByteArray &data) override;

    // 数据结构定义
    struct MeasurementData {
        QString registerCode;   // 功能码
        char fixedValue;       // 固定值
        char err;             // 错误信息
        char rng;             // 范围状态
        char ba;              // 电池电量
        float data1;          // 测量值1
        float data2;          // 测量值2
        float data3;          // 测量值3
        std::chrono::system_clock::time_point time;
    };

    // 照度计特定命令
    QByteArray makeHoldCommand(bool hold);          // HOLD功能
    QByteArray makeBacklightCommand(bool on);       // 背光控制
    QByteArray makeRangeCommand(int range);         // 量程切换
    QByteArray makeQueryCommand();                  // 数据查询
    QByteArray makeMaxMinCommand(const QString &mode); // 最大最小值模式
    QByteArray makePeakHoldCommand(bool on);        // 峰值保持
    QByteArray makeRelativeCommand(bool on);        // 相对值测量
    QByteArray makeEXTCommand(bool on);             // EXT模式
    QByteArray makePCConnectCommand();              // PC连接模式
    QByteArray makeCalibrationCommand(int row, const QString &data1,
                                    const QString &data2, const QString &data3); // 校准系数设置

    // 数据解析
    bool parseMeasurementData(const QByteArray &data, MeasurementData &result);
    float parseDataBlock(const QByteArray &block);

private:
    static constexpr char STX = 0x02;      // 起始字符
    static constexpr char ETX = 0x03;      // 结束字符
    static constexpr char CR = 0x0D;       // 回车符
    static constexpr char LF = 0x0A;       // 换行符

    // 命令代码
    static constexpr char CMD_QUERY = 'D';      // 数据查询
    static constexpr char CMD_HOLD = 'H';       // HOLD功能
    static constexpr char CMD_BACKLIGHT = 'L';  // 背光控制
    static constexpr char CMD_RANGE = 'R';      // 量程切换
    static constexpr char CMD_MAXMIN = 'M';     // 最大最小值
    static constexpr char CMD_PEAK = 'P';       // 峰值保持
    static constexpr char CMD_RELATIVE = 'V';   // 相对值测量

    // 辅助函数
    QByteArray makeCommand(char cmd, const QString &param = QString());
    bool verifyChecksum(const QByteArray &response);
    QByteArray calculateChecksum(const QByteArray &data);
    QByteArray createMessage(const QByteArray &data);

    // 错误代码定义
    enum ErrorCode {
        NoError = 0,
        InvalidResponse = 1,
        ChecksumError = 2,
        DataFormatError = 3,
        ValueRangeError = 4,
        DeviceError = 5
    };

    // 辅助方法
    bool validateResponse(const QByteArray &response);
    QString getErrorString(char errorCode);
    bool isValidRange(int range) const;
    void handleDeviceError(char errorCode);
};

#endif // CL_TWOZEROZEROACOM_H

/**
 * 使用示例

 // 创建实例
auto *meter = new CL_TwoZeroZeroACOM(this);

// 设置量程
QByteArray cmd = meter->makeRangeCommand(2);
serialPort->write(cmd);

// 打开背光
cmd = meter->makeBacklightCommand(true);
serialPort->write(cmd);

// 查询数据
cmd = meter->makeQueryCommand();
serialPort->write(cmd);

// 处理响应
connect(serialPort, &QSerialPort::readyRead, this, [=]() {
    QByteArray response = serialPort->readAll();
    QByteArray data;
    if (meter->parseResponse(response, data)) {
        CL_TwoZeroZeroACOM::MeterStatus status;
        if (meter->parseStatus(data, status)) {
            qDebug() << "照度:" << status.illuminance
                     << "色温:" << status.colorTemp
                     << "RGB:" << status.red << status.green << status.blue;
        }
    }
});
 */