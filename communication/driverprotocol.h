#ifndef DRIVERPROTOCOL_H
#define DRIVERPROTOCOL_H

#include <QObject>
#include <QByteArray>

class DriverProtocol : public QObject
{
    Q_OBJECT
public:
    explicit DriverProtocol(QObject *parent = nullptr);

    // 命令生成
    QByteArray makeChannelCommand(int channel, int value, bool highLevel);
    QByteArray makeAllChannelsCommand(int value, bool highLevel);
    QByteArray makeControlCommand(bool highLevel, int startReg, int regCount,
                                int value1, int value2, int value3);
    
    // 响应解析
    bool parseResponse(const QByteArray &response, QByteArray &data);
    
    // 数据结构
    struct ChannelData {
        int value1 = 0;
        int value2 = 0;
        int value3 = 0;
        bool highLevel = false;
    };

private:
    // 协议常量
    static constexpr char STX = 0x02;      // 起始字符
    static constexpr char ETX = 0x03;      // 结束字符
    static constexpr char CR = 0x0D;       // 回车符
    static constexpr char LF = 0x0A;       // 换行符
    
    // 命令代码
    static constexpr char CMD_CHANNEL = 'C';    // 单通道控制
    static constexpr char CMD_ALL = 'A';        // 全通道控制
    static constexpr char CMD_MULTI = 'M';      // 多通道控制
    
    // 辅助方法
    QByteArray createMessage(const QByteArray &data);
    QByteArray calculateChecksum(const QByteArray &data);
    bool verifyChecksum(const QByteArray &data);
    
    // 数据缓存
    QVector<ChannelData> m_channelData;  // 各通道数据缓存
};

#endif // DRIVERPROTOCOL_H 