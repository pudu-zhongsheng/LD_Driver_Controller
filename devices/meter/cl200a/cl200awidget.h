#ifndef CL200AWIDGET_H
#define CL200AWIDGET_H

#pragma execution_character_set("utf-8")

#include "../meterbase.h"
#include "../../../serial/serialutil.h"
#include "../../../communication/cl_twozerozeroacom.h"
#include <QTimer>
#include <QSerialPort>

class CL200AWidget : public MeterBase
{
    Q_OBJECT
public:
    explicit CL200AWidget(QWidget *parent = nullptr);
    ~CL200AWidget();
    
    // 实现基类接口
    bool connectToPort(const QString &portName) override;
    void disconnectPort() override;
    bool isConnected() const override;
    void startMeasurement() override;
    void stopMeasurement() override;
    
    // CL-200A 特定功能
    void setHold(bool hold);
    void setBacklight(bool on);
    void setRange(int range);
    void setPCMode();
    void setEXTMode(bool on);
    void setMeasurementType(int type);

    // 获取当前测量值
    float getIlluminance() const { return m_illuminance; }
    float getColorTemperature() const { return m_colorTemp; }
    float getRValue() const { return m_r; }
    float getGValue() const { return m_g; }
    float getBValue() const { return m_b; }

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);
    void onMeasurementTimerTimeout();
    
private:
    // 初始化通讯设置
    void setupInitialCommunication();
    // 解析接收到的数据
    void parseReceivedData(const QByteArray &data);
    // 发送命令
    bool sendCommand(const QByteArray &command);
    // 设置连接参数
    void setupSerialParameters();
    
    // 通讯状态枚举
    enum class CommState {
        Idle,                   // 空闲状态
        WaitingForPCModeResponse,  // 等待PC模式设置响应
        WaitingForEXTModeResponse, // 等待EXT模式设置响应
        WaitingForMeasurementData, // 等待测量数据
        Error                   // 错误状态
    };

    // 成员变量
    QSerialPort *m_serialPort;
    QString m_portname;
    CL_TwoZeroZeroACOM *m_protocol;
    QTimer *m_measurementTimer;
    QTimer *m_commandTimeoutTimer;
    
    QByteArray m_receivedData;
    CommState m_commState;
    
    bool m_isMeasuring;
    bool m_isInitialized;
    int m_currentMeasurementType;  // 当前测量类型索引
    
    // 测量值保存
    float m_illuminance;
    float m_colorTemp;
    float m_r;
    float m_g;
    float m_b;

    void initUI();
    void initConnections();
};

#endif // CL200AWIDGET_H 
