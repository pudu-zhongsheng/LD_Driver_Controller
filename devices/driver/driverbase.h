#ifndef DRIVERBASE_H
#define DRIVERBASE_H

#include <QWidget>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include "serial/serialutil.h"

class DriverBase : public QWidget
{
    Q_OBJECT
public:
    explicit DriverBase(QWidget *parent = nullptr);
    virtual ~DriverBase();

    virtual bool isConnected() const = 0;
    virtual void connectToPort(const QString &portName) = 0;
    virtual void disconnectPort() = 0;

    // 获取当前设置
    virtual QJsonObject getSettings() const = 0;
    // 应用设置
    virtual void applySettings(const QJsonObject &settings) = 0;

signals:
    void settingsChanged();  // 设置发生变化时发出信号
    void serialConnected(const QString &portName);
    void serialDisconnected();
    void serialError(const QString &error);

protected:
    SerialUtil *m_serial = nullptr;
    
    // 通用控制组件
    QSpinBox *m_startRegister;    // 起始寄存器
    QSpinBox *m_registerCount;    // 寄存器数量
    
    // 扫描测试组件
    QComboBox *m_scanTarget;      // 扫描目标选择
    QSpinBox *m_scanStep;         // 扫描步长
    QSpinBox *m_scanInterval;     // 扫描间隔
    QSpinBox *m_scanEndValue;     // 扫描终止值
    QPushButton *m_scanStartBtn;  // 扫描开始按钮
    QPushButton *m_scanStopBtn;   // 扫描停止按钮
    
    // 参数表
    QTableWidget *m_paramTable;   // 参数表格
    
    // 通用初始化函数
    virtual void initUI() = 0;
    virtual void initConnections() = 0;
    virtual void updateParamTable() = 0;
    
    // 通用工具函数
    void startScan();
    void stopScan();
    void sendCommand(const QByteArray &cmd);
};

#endif // DRIVERBASE_H 