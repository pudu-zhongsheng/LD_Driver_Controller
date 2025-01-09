#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <QByteArray>

/**
 * 通讯协议基类
 */
class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Protocol() = default;

    // 基本命令接口
    virtual QByteArray makeReadCommand(int address, int count = 1) = 0;
    virtual QByteArray makeWriteCommand(int address, const QByteArray &data) = 0;
    
    // 解析响应
    virtual bool parseResponse(const QByteArray &response, QByteArray &data) = 0;
    
    // CRC校验
    static quint16 calculateCRC16(const QByteArray &data);

signals:
    void error(const QString &message);
};

#endif // PROTOCOL_H 