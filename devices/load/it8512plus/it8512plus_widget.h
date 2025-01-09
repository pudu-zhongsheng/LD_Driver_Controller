#ifndef IT8512PLUS_WIDGET_H
#define IT8512PLUS_WIDGET_H

#include "../load_base.h"
#include "communication/eleload_itplus.h"
#include "../../../serial/serialutil.h"
#include <QTimer>
#include <QSound>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSerialPort>

class IT8512Plus_Widget : public LoadBase
{
    Q_OBJECT

public:
    explicit IT8512Plus_Widget(EleLoad_ITPlus *protocol, QWidget *parent = nullptr);
    ~IT8512Plus_Widget();

    void connectToPort(const QString &portName) override;
    void disconnectPort() override;
    bool isConnected() const override;

    QJsonObject saveSettings() const;
    void loadSettings(const QJsonObject &settings);

signals:
    void serialConnected(const QString &portName);
    void serialDisconnected();
    void statusUpdated(float voltage, float current, float power);

private slots:
    void updateDeviceInfo();        // 更新设备信息
    void updateStatus();            // 更新状态
    void onControlModeChanged();    // 控制模式改变
    void onOutputStateChanged();    // 输出状态改变
    void onWorkModeChanged();       // 工作模式改变
    void onValueSettingChanged();   // 定值设置改变
    void onDynamicSettingChanged(); // 动态设置改变
    void onTriggerSignal();         // 发送触发信号
    void checkTargetValues();       // 检查目标值
    void onMaxVoltageChanged(double value);

private:
    void setupUi();                 // 设置UI
    void initConnections();         // 初始化信号槽
    void updateTargetStatus();      // 更新目标值状态

    EleLoad_ITPlus *m_protocol;     // 通信协议
    QTimer *m_statusTimer;          // 状态更新定时器
    QSound *m_alertSound;           // 提示音

    // 目标值状态标志
    bool m_voltageTargetReached = false;
    bool m_currentTargetReached = false;
    bool m_powerTargetReached = false;

    // UI组件声明
    // 设备信息区
    QLabel *m_modelLabel;
    QLabel *m_versionLabel;
    QLabel *m_serialLabel;

    // 基本设置区
    QDoubleSpinBox *m_setMaxVoltageSpinBox;
    QDoubleSpinBox *m_setMCurrentSpinBox;
    QDoubleSpinBox *m_setMPowerSpinBox;
    QPushButton *m_controlModeBtn;    // PC/面板控制切换
    QPushButton *m_outputStateBtn;    // ON/OFF切换

    // 状态显示区
    QLabel *m_workModeLabel;          // 定值/动态模式
    QLabel *m_workTypeLabel;          // CC/CV/CR/CW
    QLabel *m_tempLabel;              // 温度
    QLabel *m_voltageLabel;           // 电压
    QLabel *m_currentLabel;           // 电流
    QLabel *m_powerLabel;             // 功率

    // 工作模式设置区
    QGroupBox *m_basicModeGroup;      // 基础模式组
    QGroupBox *m_dynamicModeGroup;    // 动态模式组
    
    // 基础模式控件
    QRadioButton *m_ccModeRadio;
    QRadioButton *m_cvModeRadio;
    QRadioButton *m_cwModeRadio;
    QRadioButton *m_crModeRadio;
    QDoubleSpinBox *m_valueSpinBox;

    // 动态模式控件
    QRadioButton *m_dcModeRadio;
    QRadioButton *m_dvModeRadio;
    QRadioButton *m_dwModeRadio;
    QRadioButton *m_drModeRadio;
    QDoubleSpinBox *m_value1SpinBox;
    QDoubleSpinBox *m_time1SpinBox;
    QDoubleSpinBox *m_value2SpinBox;
    QDoubleSpinBox *m_time2SpinBox;
    QComboBox *m_triggerModeCombo;
    QPushButton *m_triggerBtn;

    // 目标值范围设置控件
    QDoubleSpinBox *m_minVoltageSpinBox;
    QDoubleSpinBox *m_maxVoltageSpinBox;
    QDoubleSpinBox *m_minCurrentSpinBox;
    QDoubleSpinBox *m_maxCurrentSpinBox;
    QDoubleSpinBox *m_minPowerSpinBox;
    QDoubleSpinBox *m_maxPowerSpinBox;

    SerialUtil *m_serial;

    void handleSerialData(const QByteArray &data);
    void handleSerialError(const QString &error);
};

#endif // IT8512PLUS_WIDGET_H 
