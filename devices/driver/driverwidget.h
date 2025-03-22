#ifndef DRIVERWIDGET_H
#define DRIVERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QTableWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QLineEdit>

class DriverWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DriverWidget(int channelCount, QWidget *parent = nullptr);
    ~DriverWidget();

private:
    // 通道数
    int m_channelCount;
    
    // 基本信息控件
    QLineEdit* m_addressEdit;      // 从机地址
    QPushButton* m_addressConfirmBtn; // 从机地址确认按钮
    QLabel* m_ledStatusLabel;      // LED总开关状态
    QLabel* m_ledTempLabel;        // LED温度
    QLabel* m_pcbTempLabel;        // PCB温度
    QLabel* m_ratedVoltageLabel;   // 额定电压
    QLabel* m_ratedCurrentLabel;   // 额定电流
    QLabel* m_controlModeLabel;    // 控制模式
    QLabel* m_alarmStatusLabel;    // 告警状态
    
    // 调控区控件
    QComboBox* m_modeCombo;        // 模式切换
    QPushButton* m_ledSwitch;      // LED总开关
    bool m_ledStatus;              // LED开关状态
    QSpinBox* m_startRegBox;       // 起始寄存器
    QSpinBox* m_regCountBox;       // 寄存器数量
    QSpinBox* m_ledTimeEdit;       // 灯时长
    QPushButton* m_ledTimeConfirmBtn; // 灯时长确认按钮
    QDoubleSpinBox* m_maxVoltageEdit; // 最大电压
    QPushButton* m_maxVoltageConfirmBtn; // 最大电压确认按钮
    QDoubleSpinBox* m_maxCurrentEdit; // 最大电流
    QPushButton* m_maxCurrentConfirmBtn; // 最大电流确认按钮
    
    // 扫描测试控件
    QComboBox* m_scanTargetCombo;  // 扫描目标
    QSpinBox* m_startValueBox;     // 测试起始值
    QSpinBox* m_stepValueBox;      // 步长值
    QSpinBox* m_intervalBox;       // 时间间隔
    QSpinBox* m_endValueBox;       // 终止值
    QPushButton* m_increaseBtn;    // 递增按钮
    QPushButton* m_decreaseBtn;    // 递减按钮
    QPushButton* m_stopScanBtn;    // 停止扫描按钮
    QTimer* m_scanTimer;           // 扫描定时器
    
    // 参数表控件
    QTableWidget* m_paramTable;    // 参数表格
    
    // 滑条控制控件
    QComboBox* m_sliderTargetCombo;    // 控制目标
    QSlider* m_masterSlider;           // 总控滑条
    QSpinBox* m_masterValueEdit;       // 总控数值
    QVector<QSlider*> m_channelSliders;      // 单控滑条列表
    QVector<QSpinBox*> m_channelValueSpins;  // 单控数值列表

private:
    void initUI();                 // 初始化界面
    void setupConnections();       // 建立信号槽连接
    void updateParamTable();       // 更新参数表
    void startScan(bool isIncrease); // 开始扫描
    void stopScan();              // 停止扫描
    
private slots:
    void onMasterValueChanged(int value);    // 总控值改变
    void onChannelValueChanged(int index, int value); // 单控值改变
    void onScanTimeout();         // 扫描定时器超时
    
signals:
    void channelValuesChanged(const QVector<int>& values); // 通道值变化信号
};

#endif // DRIVERWIDGET_H 
