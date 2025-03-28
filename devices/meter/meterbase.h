#ifndef METERBASE_H
#define METERBASE_H

#include <QWidget>
#include <QDateTime>

class MeterBase : public QWidget
{
    Q_OBJECT
public:
    explicit MeterBase(QWidget *parent = nullptr);
    virtual ~MeterBase() = default;
    
    // 连接/断开串口
    virtual bool connectToPort(const QString &portName) = 0;
    virtual void disconnectPort() = 0;
    
    // 检查是否已连接
    virtual bool isConnected() const = 0;
    
    // 开始/停止测量
    virtual void startMeasurement() = 0;
    virtual void stopMeasurement() = 0;

signals:
    // 连接状态信号
    void serialConnected(const QString &portName);
    void serialDisconnected();
    void serialError(const QString &errorMessage);
    
    // 数据更新信号
    void measurementUpdated(float illuminance, float colorTemp, float r, float g, float b);
};

#endif // METERBASE_H 