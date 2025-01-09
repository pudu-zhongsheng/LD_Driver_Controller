#ifndef DRIVER8CH_H
#define DRIVER8CH_H

#include "../driverbase.h"
#include "communication/driverprotocol.h"
#include "../widgets/driverinfowidget.h"
#include "../widgets/controlwidget.h"
#include "../widgets/scanwidget.h"
#include "../widgets/sliderwidget.h"
#include "../widgets/paramtablewidget.h"
#include <QButtonGroup>
#include <QFile>
#include <QJsonObject>

class Driver8CH : public DriverBase
{
    Q_OBJECT
public:
    explicit Driver8CH(QWidget *parent = nullptr);
    ~Driver8CH() override;

    bool isConnected() const override;
    void connectToPort(const QString &portName) override;
    void disconnectPort() override;

    QJsonObject getSettings() const override;
    void applySettings(const QJsonObject &settings) override;

private slots:
    void handleSerialData(const QByteArray &data);
    void handleSerialError(QSerialPort::SerialPortError error);
    
    // 控制区域响应
    void onControlChanged();
    
    // 扫描测试响应
    void onScanValueChanged(int value);
    
    // 滑条控制响应
    void onSliderChannelChanged(int channel, int value);
    void onSliderAllChanged(int value);

private:
    // 协议对象
    DriverProtocol *m_protocol;
    
    // 功能模块
    DriverInfoWidget *m_infoWidget;     // 基本信息
    ControlWidget *m_controlWidget;     // 调控区域
    ScanWidget *m_scanWidget;           // 扫描测试
    SliderWidget *m_sliderWidget;       // 滑条控制
    ParamTableWidget *m_paramTable;     // 参数表
    
    // 配置相关
    void saveConfig();
    void loadConfig();
    
    // 通用方法
    void initUI() override;
    void initConnections() override;
    void updateParamTable() override;
    
    // 命令发送
    void sendChannelCommand(int channel, int value);
    void sendAllChannelsCommand(int value);
    void sendControlCommand(bool highLevel, int startReg, int regCount, 
                          int value1, int value2, int value3);
};

#endif // DRIVER8CH_H 
