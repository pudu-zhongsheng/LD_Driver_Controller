#include "it8512plus_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QComboBox>
#include <QDebug>
#include <QButtonGroup>
#include <QJsonObject>

IT8512Plus_Widget::IT8512Plus_Widget(EleLoad_ITPlus *protocol, QWidget *parent)
    : LoadBase(parent)
    , m_protocol(protocol)
{
    // 创建串口对象
    m_serial = new SerialUtil(this);
    
    // 连接串口信号
    connect(m_serial, &SerialUtil::dataReceived,
            this, &IT8512Plus_Widget::handleSerialData);
    connect(m_serial, &SerialUtil::portDisconnected,
            this, [this]() {
                emit serialDisconnected();
                m_statusTimer->stop();
            });
            
    setupUi();
    initConnections();

    // 创建状态更新定时器
    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(1000); // 1秒更新一次
    connect(m_statusTimer, &QTimer::timeout, this, &IT8512Plus_Widget::updateStatus);
    
    // 初始化提示音
    m_alertSound = new QSound(":/sounds/dingding.wav", this);
}

void IT8512Plus_Widget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *topLayout = new QHBoxLayout();
    auto *bottomLayout = new QHBoxLayout();

    // 1. 创建设备信息区
    auto *infoGroup = new QGroupBox("设备信息", this);
    auto *infoLayout = new QVBoxLayout(infoGroup);
    m_modelLabel = new QLabel( "产品型号：",infoGroup);
    m_versionLabel = new QLabel( "软件版本：",infoGroup);
    m_serialLabel = new QLabel( "产品序列号：",infoGroup);
    infoLayout->addWidget(m_modelLabel);
    infoLayout->addWidget(m_versionLabel);
    infoLayout->addWidget(m_serialLabel);

    // 2. 创建基本设置区
    auto *basicGroup = new QGroupBox("基本设置", this);
    auto *basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel("最大电压(V):"), 0, 0);
    m_setMaxVoltageSpinBox = new QDoubleSpinBox(basicGroup);
    m_setMaxVoltageSpinBox->setRange(0, 120);
    basicLayout->addWidget(m_setMaxVoltageSpinBox, 0, 1);

    basicLayout->addWidget(new QLabel("最大电流(A):"), 1, 0);
    m_setMCurrentSpinBox = new QDoubleSpinBox(basicGroup);
    m_setMCurrentSpinBox->setRange(0, 30);
    basicLayout->addWidget(m_setMCurrentSpinBox, 1, 1);

    basicLayout->addWidget(new QLabel("最大功率(W):"), 2, 0);
    m_setMPowerSpinBox = new QDoubleSpinBox(basicGroup);
    m_setMPowerSpinBox->setRange(0, 300);
    basicLayout->addWidget(m_setMPowerSpinBox, 2, 1);

    m_controlModeBtn = new QPushButton("切换到面板控制", basicGroup);
    m_outputStateBtn = new QPushButton("输出 OFF", basicGroup);
    basicLayout->addWidget(m_controlModeBtn, 3, 0);
    basicLayout->addWidget(m_outputStateBtn, 3, 1);

    // 3. 创建状态显示区
    auto *statusGroup = new QGroupBox("运行状态", this);
    auto *statusLayout = new QGridLayout(statusGroup);
    
    statusLayout->addWidget(new QLabel("工作模式:"), 0, 0);
    m_workModeLabel = new QLabel("定值模式", statusGroup);
    statusLayout->addWidget(m_workModeLabel, 0, 1);

    statusLayout->addWidget(new QLabel("工作类型:"), 1, 0);
    m_workTypeLabel = new QLabel("CC", statusGroup);
    statusLayout->addWidget(m_workTypeLabel, 1, 1);

    statusLayout->addWidget(new QLabel("温度(℃):"), 2, 0);
    m_tempLabel = new QLabel("25", statusGroup);
    statusLayout->addWidget(m_tempLabel, 2, 1);

    statusLayout->addWidget(new QLabel("电压(V):"), 3, 0);
    m_voltageLabel = new QLabel("0.000", statusGroup);
    statusLayout->addWidget(m_voltageLabel, 3, 1);

    statusLayout->addWidget(new QLabel("电流(A):"), 4, 0);
    m_currentLabel = new QLabel("0.000", statusGroup);
    statusLayout->addWidget(m_currentLabel, 4, 1);

    statusLayout->addWidget(new QLabel("功率(W):"), 5, 0);
    m_powerLabel = new QLabel("0.000", statusGroup);
    statusLayout->addWidget(m_powerLabel, 5, 1);

    // 在状态显示区添加工作模式选择
    auto *workModeBox = new QGroupBox("工作模式选择", this);
    auto *workModeLayout = new QHBoxLayout(workModeBox);
    m_workModeSelect = new QComboBox(this);
    m_workModeSelect->addItem("基础模式");
    m_workModeSelect->addItem("动态测试");
    workModeLayout->addWidget(m_workModeSelect);
    
    // 将工作模式选择添加到布局中
    statusLayout->addWidget(workModeBox, 0, 0, 1, 2);  // 跨两列显示

    // 4. 创建目标值设置区
    auto *targetGroup = new QGroupBox("目标值范围设置", this);
    auto *targetLayout = new QGridLayout(targetGroup);

    // 电压范围设置
    targetLayout->addWidget(new QLabel("目标电压范围(V):"), 0, 0);
    m_minVoltageSpinBox = new QDoubleSpinBox(targetGroup);
    m_minVoltageSpinBox->setRange(0, 150);
    m_maxVoltageSpinBox = new QDoubleSpinBox(targetGroup);
    m_maxVoltageSpinBox->setRange(0, 150);
    targetLayout->addWidget(m_minVoltageSpinBox, 0, 1);
    targetLayout->addWidget(new QLabel("~"), 0, 2);
    targetLayout->addWidget(m_maxVoltageSpinBox, 0, 3);

    // 电流范围设置
    targetLayout->addWidget(new QLabel("目标电流范围(A):"), 1, 0);
    m_minCurrentSpinBox = new QDoubleSpinBox(targetGroup);
    m_minCurrentSpinBox->setRange(0, 30);
    m_maxCurrentSpinBox = new QDoubleSpinBox(targetGroup);
    m_maxCurrentSpinBox->setRange(0, 30);
    targetLayout->addWidget(m_minCurrentSpinBox, 1, 1);
    targetLayout->addWidget(new QLabel("~"), 1, 2);
    targetLayout->addWidget(m_maxCurrentSpinBox, 1, 3);

    // 功率范围设置
    targetLayout->addWidget(new QLabel("目标功率范围(W):"), 2, 0);
    m_minPowerSpinBox = new QDoubleSpinBox(targetGroup);
    m_minPowerSpinBox->setRange(0, 300);
    m_maxPowerSpinBox = new QDoubleSpinBox(targetGroup);
    m_maxPowerSpinBox->setRange(0, 300);
    targetLayout->addWidget(m_minPowerSpinBox, 2, 1);
    targetLayout->addWidget(new QLabel("~"), 2, 2);
    targetLayout->addWidget(m_maxPowerSpinBox, 2, 3);

    // 5. 创建工作模式设置区
    auto *modeGroup = new QGroupBox("工作模式设置", this);
    auto *modeLayout = new QVBoxLayout(modeGroup);
    
    // 基础模式组
    m_basicModeGroup = new QGroupBox("基础模式", modeGroup);
    auto *basicModeLayout = new QVBoxLayout(m_basicModeGroup);
    
    m_ccModeRadio = new QRadioButton("CC模式", m_basicModeGroup);
    m_cvModeRadio = new QRadioButton("CV模式", m_basicModeGroup);
    m_cwModeRadio = new QRadioButton("CW模式", m_basicModeGroup);
    m_crModeRadio = new QRadioButton("CR模式", m_basicModeGroup);
    
    basicModeLayout->addWidget(m_ccModeRadio);
    basicModeLayout->addWidget(m_cvModeRadio);
    basicModeLayout->addWidget(m_cwModeRadio);
    basicModeLayout->addWidget(m_crModeRadio);
    
    auto *valueLayout = new QHBoxLayout();
    valueLayout->addWidget(new QLabel("设定值:"));
    m_valueSpinBox = new QDoubleSpinBox(m_basicModeGroup);
    valueLayout->addWidget(m_valueSpinBox);
    basicModeLayout->addLayout(valueLayout);

    // 动态模式组
    m_dynamicModeGroup = new QGroupBox("动态测试", modeGroup);
    auto *dynamicModeLayout = new QVBoxLayout(m_dynamicModeGroup);
    
    auto *dynamicRadioLayout = new QHBoxLayout();
    m_dcModeRadio = new QRadioButton("DC", m_dynamicModeGroup);
    m_dvModeRadio = new QRadioButton("DV", m_dynamicModeGroup);
    m_dwModeRadio = new QRadioButton("DW", m_dynamicModeGroup);
    m_drModeRadio = new QRadioButton("DR", m_dynamicModeGroup);
    
    dynamicRadioLayout->addWidget(m_dcModeRadio);
    dynamicRadioLayout->addWidget(m_dvModeRadio);
    dynamicRadioLayout->addWidget(m_dwModeRadio);
    dynamicRadioLayout->addWidget(m_drModeRadio);
    dynamicModeLayout->addLayout(dynamicRadioLayout);

    // 动态参数设置
    auto *dynamicParamsLayout = new QGridLayout();
    dynamicParamsLayout->addWidget(new QLabel("Value1:"), 0, 0);
    m_value1SpinBox = new QDoubleSpinBox(m_dynamicModeGroup);
    dynamicParamsLayout->addWidget(m_value1SpinBox, 0, 1);
    
    dynamicParamsLayout->addWidget(new QLabel("Time1(ms):"), 0, 2);
    m_time1SpinBox = new QDoubleSpinBox(m_dynamicModeGroup);
    dynamicParamsLayout->addWidget(m_time1SpinBox, 0, 3);
    
    dynamicParamsLayout->addWidget(new QLabel("Value2:"), 1, 0);
    m_value2SpinBox = new QDoubleSpinBox(m_dynamicModeGroup);
    dynamicParamsLayout->addWidget(m_value2SpinBox, 1, 1);
    
    dynamicParamsLayout->addWidget(new QLabel("Time2(ms):"), 1, 2);
    m_time2SpinBox = new QDoubleSpinBox(m_dynamicModeGroup);
    dynamicParamsLayout->addWidget(m_time2SpinBox, 1, 3);
    
    dynamicModeLayout->addLayout(dynamicParamsLayout);

    // 触发设置
    auto *triggerLayout = new QHBoxLayout();
    triggerLayout->addWidget(new QLabel("触发方式:"));
    m_triggerModeCombo = new QComboBox(m_dynamicModeGroup);
    m_triggerModeCombo->addItems({"连续", "脉冲", "触发"});
    triggerLayout->addWidget(m_triggerModeCombo);
    
    m_triggerBtn = new QPushButton("发送触发", m_dynamicModeGroup);
    triggerLayout->addWidget(m_triggerBtn);
    dynamicModeLayout->addLayout(triggerLayout);

    modeLayout->addWidget(m_basicModeGroup);
    modeLayout->addWidget(m_dynamicModeGroup);

    // 组装布局
    topLayout->addWidget(infoGroup);
    topLayout->addWidget(basicGroup);
    topLayout->addWidget(statusGroup);

    bottomLayout->addWidget(targetGroup);
    bottomLayout->addWidget(modeGroup);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
}

void IT8512Plus_Widget::initConnections()
{
    // 基本设置区连接
    connect(m_setMaxVoltageSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &IT8512Plus_Widget::onMaxVoltageChanged);

    connect(m_setMCurrentSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double value) {
        if (isConnected()) {
            QByteArray cmd = m_protocol->createSetMaxCurrentCommand(value);
            m_serial->write(cmd);
        }
    });

    connect(m_setMPowerSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double value) {
        if (isConnected()) {
            QByteArray cmd = m_protocol->createSetMaxPowerCommand(value);
            m_serial->write(cmd);
        }
    });

    connect(m_controlModeBtn, &QPushButton::clicked, this, &IT8512Plus_Widget::onControlModeChanged);
    connect(m_outputStateBtn, &QPushButton::clicked, this, &IT8512Plus_Widget::onOutputStateChanged);

    // 工作模式连接
    auto modeGroup = new QButtonGroup(this);
    modeGroup->addButton(m_ccModeRadio);
    modeGroup->addButton(m_cvModeRadio);
    modeGroup->addButton(m_cwModeRadio);
    modeGroup->addButton(m_crModeRadio);
    connect(modeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &IT8512Plus_Widget::onWorkModeChanged);

    // 动态模式连接
    auto dynamicGroup = new QButtonGroup(this);
    dynamicGroup->addButton(m_dcModeRadio);
    dynamicGroup->addButton(m_dvModeRadio);
    dynamicGroup->addButton(m_dwModeRadio);
    dynamicGroup->addButton(m_drModeRadio);
    connect(dynamicGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &IT8512Plus_Widget::onWorkModeChanged);

    // 定值设置连接
    connect(m_valueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &IT8512Plus_Widget::onValueSettingChanged);

    // 动态测试参数变化连接
    connect(m_value1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &IT8512Plus_Widget::onDynamicSettingChanged);
    connect(m_time1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &IT8512Plus_Widget::onDynamicSettingChanged);
    connect(m_value2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &IT8512Plus_Widget::onDynamicSettingChanged);
    connect(m_time2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &IT8512Plus_Widget::onDynamicSettingChanged);

    // 触发相关连接
    connect(m_triggerBtn, &QPushButton::clicked, this, &IT8512Plus_Widget::onTriggerSignal);

    // 添加工作模式选择的信号处理
    connect(m_workModeSelect, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &IT8512Plus_Widget::onWorkModeSelectChanged);
}

void IT8512Plus_Widget::updateDeviceInfo()
{
    if (!isConnected()) return;

    // 获取产品信息
    m_serial->write(m_protocol->createGetProductMessage());
    
    // 获取最大值设置
    m_serial->write(m_protocol->createGetMaxVoltageCommand());
    m_serial->write(m_protocol->createGetMaxCurrentCommand());
    m_serial->write(m_protocol->createGetMaxPowerCommand());
    
    // 获取工作模式
    m_serial->write(m_protocol->createGetWorkModel());
    m_serial->write(m_protocol->createGetLoadModeCommand());
    
    // 获取当前值设置
    m_serial->write(m_protocol->createGetConstantCurrentCommand());
    m_serial->write(m_protocol->createGetDynamicCurrentParamsCommand());
}

void IT8512Plus_Widget::updateStatus()
{
    if (!isConnected()) {
        m_statusTimer->stop();  // 如果连接断开，停止定时器
        return;
    }
    
    QByteArray cmd = m_protocol->createGetInputCommand();
    m_serial->write(cmd);
}

void IT8512Plus_Widget::onControlModeChanged()
{
    if (!isConnected()) return;

    // 切换控制模式
    QByteArray cmd = m_protocol->createControlModeCommand(m_isRemoteMode ? 0 : 1);
    m_serial->write(cmd);
}

void IT8512Plus_Widget::onOutputStateChanged()
{
    if (!isConnected()) return;

    // 切换输出状态
    QByteArray cmd = m_protocol->createLoadStateCommand(m_isOutput ? 0 : 1);
    m_serial->write(cmd);
}

void IT8512Plus_Widget::onWorkModeChanged()
{
    if (!isConnected()) return;

    // 判断当前工作模式
    bool isDynamic = !m_isBasicMode;
    
    // 设置工作类型
    QString type;
    uint8_t mode = 0;
    uint8_t workMode = 0;
    
    if (isDynamic) {
        mode = 0x02;
        QByteArray cmd = m_protocol->createSetWorkModel(mode);
        m_serial->write(cmd);

        // 暂时阻断信号连接
        m_value1SpinBox->blockSignals(true);
        m_time1SpinBox->blockSignals(true);
        m_value2SpinBox->blockSignals(true);
        m_time2SpinBox->blockSignals(true);

        // 动态测试模式
        if (m_dcModeRadio->isChecked()) {
            type = "DC";
            // 读取动态电流参数
            m_serial->write(m_protocol->createGetDynamicCurrentParamsCommand());
        } else if (m_dvModeRadio->isChecked()) {
            type = "DV";
            // 读取动态电压参数
            m_serial->write(m_protocol->createGetDynamicVoltageParamsCommand());
        } else if (m_dwModeRadio->isChecked()) {
            type = "DW";
            // 读取动态功率参数
            m_serial->write(m_protocol->createGetDynamicPowerParamsCommand());
        } else if (m_drModeRadio->isChecked()) {
            type = "DR";
            // 读取动态电阻参数
            m_serial->write(m_protocol->createGetDynamicResistanceParamsCommand());
        }
        
    } else {
        mode = 0x00;
        QByteArray cmd = m_protocol->createSetWorkModel(mode);
        m_serial->write(cmd);
        // 基础模式
        if (m_ccModeRadio->isChecked()) {
            type = "CC";
            workMode = 0x00;
            // 读取定值电流
            m_serial->write(m_protocol->createGetConstantCurrentCommand());
        } else if (m_cvModeRadio->isChecked()) {
            type = "CV";
            workMode = 0x01;
            // 读取定值电压
            m_serial->write(m_protocol->createGetConstantVoltageCommand());
        } else if (m_cwModeRadio->isChecked()) {
            type = "CW";
            workMode = 0x02;
            // 读取定值功率
            m_serial->write(m_protocol->createGetConstantPowerCommand());
        } else if (m_crModeRadio->isChecked()) {
            type = "CR";
            workMode = 0x03;
            // 读取定值电阻
            m_serial->write(m_protocol->createGetConstantResistanceCommand());
        }
        // 设置负载模式
        m_serial->write(m_protocol->createSetLoadModeCommand(workMode));
    }
    
    m_workTypeLabel->setText(type);
    
    // 更新输入限制
    updateInputLimits();
}

void IT8512Plus_Widget::onValueSettingChanged()
{
    if (!isConnected() || !m_isBasicMode) return;  // 只在基础模式下处理

    double value = m_valueSpinBox->value();
    QByteArray cmd;
    
    if (m_ccModeRadio->isChecked()) {
        cmd = m_protocol->createSetConstantCurrentCommand(value);
    } else if (m_cvModeRadio->isChecked()) {
        cmd = m_protocol->createSetConstantVoltageCommand(value);
    } else if (m_cwModeRadio->isChecked()) {
        cmd = m_protocol->createSetConstantPowerCommand(value);
    } else if (m_crModeRadio->isChecked()) {
        cmd = m_protocol->createSetConstantResistanceCommand(value);
    }

    if (!cmd.isEmpty()) {
        m_serial->write(cmd);
    }
}

void IT8512Plus_Widget::onDynamicSettingChanged()
{
    if (!isConnected() || m_isBasicMode) return;  // 只在动态测试模式下处理

    double value1 = m_value1SpinBox->value();
    double time1 = m_time1SpinBox->value();
    double value2 = m_value2SpinBox->value();
    double time2 = m_time2SpinBox->value();
    uint8_t mode = m_triggerModeCombo->currentIndex();
    
    QByteArray cmd;
    
    if (m_dcModeRadio->isChecked()) {
        cmd = m_protocol->createSetDynamicCurrentParamsCommand(value1, time1, value2, time2, mode);
    } else if (m_dvModeRadio->isChecked()) {
        cmd = m_protocol->createSetDynamicVoltageParamsCommand(value1, time1, value2, time2, mode);
    } else if (m_dwModeRadio->isChecked()) {
        cmd = m_protocol->createSetDynamicPowerParamsCommand(value1, time1, value2, time2, mode);
    } else if (m_drModeRadio->isChecked()) {
        cmd = m_protocol->createSetDynamicResistanceParamsCommand(value1, time1, value2, time2, mode);
    }

    if (!cmd.isEmpty()) {
        m_serial->write(cmd);
    }
}

void IT8512Plus_Widget::onTriggerSignal()
{
    if (!isConnected()) return;
    
    QByteArray cmd = m_protocol->createNewTriggerSignal();
    m_serial->write(cmd);
}

void IT8512Plus_Widget::checkTargetValues()
{
    if (!isConnected()) return;

    // 获取当前显示的值
    double voltage = m_voltageLabel->text().toDouble();
    double current = m_currentLabel->text().toDouble();
    double power = m_powerLabel->text().toDouble();
    
    // 检查电压目标值范围
    double minVoltage = m_minVoltageSpinBox->value();
    double maxVoltage = m_maxVoltageSpinBox->value();
    if (minVoltage > 0.001 || maxVoltage > 0.001) {  // 只有设置了有效范围才检查
        if (voltage >= minVoltage - 0.001 && voltage <= maxVoltage + 0.001) {
            if (!m_voltageTargetReached) {
                m_voltageTargetReached = true;
                m_alertSound->play();
            }
        } else {
            m_voltageTargetReached = false;
        }
    }
    
    // 检查电流目标值范围
    double minCurrent = m_minCurrentSpinBox->value();
    double maxCurrent = m_maxCurrentSpinBox->value();
    if (minCurrent > 0.001 || maxCurrent > 0.001) {  // 只有设置了有效范围才检查
        if (current >= minCurrent - 0.001 && current <= maxCurrent + 0.001) {
            if (!m_currentTargetReached) {
                m_currentTargetReached = true;
                m_alertSound->play();
            }
        } else {
            m_currentTargetReached = false;
        }
    }
    
    // 检查功率目标值范围
    double minPower = m_minPowerSpinBox->value();
    double maxPower = m_maxPowerSpinBox->value();
    if (minPower > 0.001 || maxPower > 0.001) {  // 只有设置了有效范围才检查
        if (power >= minPower - 0.001 && power <= maxPower + 0.001) {
            if (!m_powerTargetReached) {
                m_powerTargetReached = true;
                m_alertSound->play();
            }
        } else {
            m_powerTargetReached = false;
        }
    }
}

bool IT8512Plus_Widget::connectToPort(const QString &portName)
{
    if (m_serial->connectToPort(portName)) {
        m_isConnecting = true;
        // 发送远程控制模式命令
        QByteArray cmd = m_protocol->createControlModeCommand(0x01);
        m_serial->write(cmd);
        
        // 等待响应超时处理
        QTimer::singleShot(1000, this, [this]() {
            if (m_isConnecting) {
                m_isConnecting = false;
                m_serial->disconnectPort();
                emit serialError("连接超时，请检查设备");
            }
        });
        
        return true;
    }
    return false;
}

void IT8512Plus_Widget::handleSerialData(const QByteArray &data)
{
    // 累积接收的数据
    m_buffer.append(data);
    
    // 处理完整的数据包
    while (m_buffer.size() >= 26) {
        // 查找同步头
        int startIndex = m_buffer.indexOf(0xAA);
        if (startIndex == -1) {
            m_buffer.clear();
            return;
        }
        
        // 如果同步头不在开始位置，移除前面的数据
        if (startIndex > 0) {
            m_buffer.remove(0, startIndex);
            continue;
        }
        
        // 确保有完整的数据包
        if (m_buffer.size() < 26) {
            return;
        }
        
        // 提取一个完整数据包
        QByteArray packet = m_buffer.left(26);
        qDebug() << "!!!!!!!Packetdata is : " << packet.toHex();
        m_buffer.remove(0, 26);
        
        // 验证数据包
        if (!m_protocol->validateResponse(packet)) {
            continue;
        }
        
        // 根据功能码处理数据
        uint8_t functionCode = packet[2];
        switch (functionCode) {
            case 0x12: // 设置指令的响应
                handleSetCommandResponse(packet);
                break;
                
            case 0x23: // 最大电压响应
                handleMaxVoltageResponse(packet);
                break;
                
            case 0x25: // 最大电流响应
                handleMaxCurrentResponse(packet);
                break;
                
            case 0x27: // 最大功率响应
                handleMaxPowerResponse(packet);
                break;
                
            case 0x29: // 工作模式响应
                handleWorkModeResponse(packet);
                break;
                
            case 0x5F: // 实时状态响应
                handleStatusResponse(packet);
                break;
                
            case 0x6A: // 产品信息响应
                handleProductInfoResponse(packet);
                break;
                
            case 0x2B: // CC值
            case 0x2D: // CV值
            case 0x2F: // CW值
            case 0x31: // CR值
                handleConstantValueResponse(packet);
                break;
                
            case 0x33: // 动态电流参数
            case 0x35: // 动态电压参数
            case 0x37: // 动态功率参数
            case 0x39: // 动态电阻参数
                handleDynamicValueResponse(packet);
                break;
        }
    }
}

// 处理设置指令响应
void IT8512Plus_Widget::handleSetCommandResponse(const QByteArray &packet)
{
    if (m_isConnecting) {   // 正在进行连接
        m_isConnecting = false;
        m_isRemoteMode = true;
        m_statusTimer->start(2000);  // 启动状态更新定时器
        emit serialConnected(m_serial->portName());
        
        // 连接成功后获取设备信息
        QTimer::singleShot(100, this, [this]() {
            updateDeviceInfo();
        });
    } else {
        // 普通设置指令的响应处理
        quint8 state = static_cast<quint8>(packet[3]);
        QString showText;

        if(state == 0x90){
            showText = "校验和错误";
        }else if(state == 0xA0){
            showText = "设置参数错误或参数溢出";
        }else if(state == 0xB0){
            showText = "命令不能被执行";
        }else if(state == 0xC0){
            showText = "命令是无效的";
        }else if(state == 0xD0){
            showText = "命令是未知的";
        }else if(state == 0x80){
            showText = "设置成功";
        }
        ToastMessage *toast = new ToastMessage(showText, this);
        toast->showToast(1000);
    }
}

// 处理状态响应
void IT8512Plus_Widget::handleStatusResponse(const QByteArray &packet)
{
    QByteArray data;
    if (m_protocol->parseResponse(packet, data)) {
        auto params = m_protocol->analyseInputParams(data);
        
        // 更新UI显示
        m_voltageLabel->setText(QString::number(params.voltage, 'f', 3) + " V");
        m_currentLabel->setText(QString::number(params.current, 'f', 4) + " A");
        m_powerLabel->setText(QString::number(params.power, 'f', 3) + " W");
        m_tempLabel->setText(QString::number(params.radiatorTemperature) + " ℃");
        
        // 更新工作状态
        m_isRemoteMode = params.handleStateRegister.rem;
        m_isOutput = params.handleStateRegister.out;
        m_controlModeBtn->setText(m_isRemoteMode ? "PC控制" : "面板控制");
        m_outputStateBtn->setText(m_isOutput ? "输出 ON" : "输出 OFF");
        qDebug() << "m_isRemoteMode is : " << m_isRemoteMode;
        qDebug() << "isOutput is : " << m_isOutput;
        // 发送状态更新信号
        emit statusUpdated(params.voltage, params.current, params.power);
        
        // 检查目标值
        checkTargetValues();
    }
}

// 处理最大电压响应
void IT8512Plus_Widget::handleMaxVoltageResponse(const QByteArray &packet)
{
    float maxVoltage = m_protocol->analyseMaxVoltage(packet);
    if (maxVoltage > 0) {
        m_setMaxVoltageSpinBox->setValue(maxVoltage);
    }
}

// 处理最大电流响应
void IT8512Plus_Widget::handleMaxCurrentResponse(const QByteArray &packet)
{
    float maxCurrent = m_protocol->analyseMaxCurrent(packet);
    if (maxCurrent > 0) {
        m_setMCurrentSpinBox->setValue(maxCurrent);
    }
}

// 处理最大功率响应
void IT8512Plus_Widget::handleMaxPowerResponse(const QByteArray &packet)
{
    float maxPower = m_protocol->analyseMaxPower(packet);
    if (maxPower > 0) {
        m_setMPowerSpinBox->setValue(maxPower);
    }
}

// 处理工作模式响应
void IT8512Plus_Widget::handleWorkModeResponse(const QByteArray &packet)
{
    uint8_t mode = m_protocol->analyseLoadMode(packet);
    QString modeText;
    switch (mode) {
        case 0:
            modeText = "CC";
            m_ccModeRadio->setChecked(true);
            break;
        case 1:
            modeText = "CV";
            m_cvModeRadio->setChecked(true);
            break;
        case 2:
            modeText = "CW";
            m_cwModeRadio->setChecked(true);
            break;
        case 3:
            modeText = "CR";
            m_crModeRadio->setChecked(true);
            break;
    }
    m_workTypeLabel->setText(modeText);
}

// 处理产品信息响应
void IT8512Plus_Widget::handleProductInfoResponse(const QByteArray &packet)
{
    auto info = m_protocol->analyseProductMessage(packet);
    
    // 添加调试输出
    qDebug() << "Product info received:";
    qDebug() << "Model:" << info.productModel;
    qDebug() << "Version:" << info.softwareVersion;
    qDebug() << "Serial:" << info.productSerial;
    
    // 使用 QString::fromUtf8 确保正确的字符编码
    m_modelLabel->setText(QString::fromUtf8("产品型号：%1").arg(info.productModel));
    m_versionLabel->setText(QString::fromUtf8("软件版本：%1").arg(info.softwareVersion));
    m_serialLabel->setText(QString::fromUtf8("产品序列号：%1").arg(info.productSerial));
    
    // 验证标签内容
    qDebug() << "Label texts after update:";
    qDebug() << "Model label:" << m_modelLabel->text();
    qDebug() << "Version label:" << m_versionLabel->text();
    qDebug() << "Serial label:" << m_serialLabel->text();
}

// 处理定值响应
void IT8512Plus_Widget::handleConstantValueResponse(const QByteArray &packet)
{
    uint8_t functionCode = packet[2];
    float value = 0.0f;
    QString unit;

    switch (functionCode) {
        case 0x2B: // CC
            value = m_protocol->analyseConstantCurrent(packet);
            unit = "A";
            break;
        case 0x2D: // CV
            value = m_protocol->analyseConstantVoltage(packet);
            unit = "V";
            break;
        case 0x2F: // CW
            value = m_protocol->analyseConstantPower(packet);
            unit = "W";
            break;
        case 0x31: // CR
            value = m_protocol->analyseConstantResistance(packet);
            unit = "Ω";
            break;
    }

    if (value > 0) {
        m_valueSpinBox->setValue(value);
    }
}

// 处理动态值响应
void IT8512Plus_Widget::handleDynamicValueResponse(const QByteArray &packet)
{
    uint8_t functionCode = packet[2];
    EleLoad_ITPlus::DynamicParams params;


    switch (functionCode) {
        case 0x33: // 动态电流
            params = m_protocol->analyseDynamicCurrentParams(packet);
            break;
        case 0x35: // 动态电压
            params = m_protocol->analyseDynamicVoltageParams(packet);
            break;
        case 0x37: // 动态功率
            params = m_protocol->analyseDynamicPowerParams(packet);
            break;
        case 0x39: // 动态电阻
            params = m_protocol->analyseDynamicResistanceParams(packet);
            break;
    }

    // 更新UI显示
    m_value1SpinBox->setValue(params.valueA);
    m_time1SpinBox->setValue(params.timeA);
    m_value2SpinBox->setValue(params.valueB);
    m_time2SpinBox->setValue(params.timeB);
    
    // 设置触发模式
    m_triggerModeCombo->setCurrentIndex(params.mode);

    // 恢复信号连接
    m_value1SpinBox->blockSignals(false);
    m_time1SpinBox->blockSignals(false);
    m_value2SpinBox->blockSignals(false);
    m_time2SpinBox->blockSignals(false);
}

// 示例：发送设置最大电压命令
void IT8512Plus_Widget::onMaxVoltageChanged(double value)
{
    if (m_serial && m_serial->isConnected()) {
        QByteArray cmd = m_protocol->createSetMaxVoltageCommand(value);
        m_serial->write(cmd);
    }
    updateInputLimits();
}

void IT8512Plus_Widget::disconnectPort()
{
    if (m_serial) {
        m_statusTimer->stop();   // 断开连接时停止状态更新定时器
        m_serial->disconnectPort();
        emit serialDisconnected();
    }
}

bool IT8512Plus_Widget::isConnected() const
{
    return m_serial && m_serial->isConnected();
}

void IT8512Plus_Widget::handleSerialError(const QString &error)
{
    // 如果是断开连接错误，发送断开连接信号
    if (error.contains("disconnected", Qt::CaseInsensitive)) {
        emit serialDisconnected();
    }
}

QJsonObject IT8512Plus_Widget::saveSettings() const
{
    QJsonObject settings;
    
    // 保存基本设置
    settings["maxVoltage"] = m_setMaxVoltageSpinBox->value();
    settings["maxCurrent"] = m_setMCurrentSpinBox->value();
    settings["maxPower"] = m_setMPowerSpinBox->value();
    
    // 保存目标值范围
    settings["minVoltage"] = m_minVoltageSpinBox->value();
    settings["maxVoltage"] = m_maxVoltageSpinBox->value();
    settings["minCurrent"] = m_minCurrentSpinBox->value();
    settings["maxCurrent"] = m_maxCurrentSpinBox->value();
    settings["minPower"] = m_minPowerSpinBox->value();
    settings["maxPower"] = m_maxPowerSpinBox->value();
    
    return settings;
}

void IT8512Plus_Widget::loadSettings(const QJsonObject &settings)
{
    // 加载基本设置
    m_setMaxVoltageSpinBox->setValue(settings["maxVoltage"].toDouble());
    m_setMCurrentSpinBox->setValue(settings["maxCurrent"].toDouble());
    m_setMPowerSpinBox->setValue(settings["maxPower"].toDouble());
    
    // 加载目标值范围
    m_minVoltageSpinBox->setValue(settings["minVoltage"].toDouble());
    m_maxVoltageSpinBox->setValue(settings["maxVoltage"].toDouble());
    m_minCurrentSpinBox->setValue(settings["minCurrent"].toDouble());
    m_maxCurrentSpinBox->setValue(settings["maxCurrent"].toDouble());
    m_minPowerSpinBox->setValue(settings["minPower"].toDouble());
    m_maxPowerSpinBox->setValue(settings["maxPower"].toDouble());
}

// 添加工作模式选择变化的处理函数
void IT8512Plus_Widget::onWorkModeSelectChanged(int index)
{
    m_isBasicMode = (index == 0);
    
    // 更新UI状态
    m_basicModeGroup->setEnabled(m_isBasicMode);
    m_dynamicModeGroup->setEnabled(!m_isBasicMode);
    
    // 更新工作模式显示
    m_workModeLabel->setText(m_isBasicMode ? "基础模式" : "动态测试模式");
    
    // 如果已连接，读取当前模式的设置
    if (isConnected()) {
        if (m_isBasicMode) {
            // 读取基础模式的设置
            m_serial->write(m_protocol->createGetLoadModeCommand());  // 读取当前工作类型
            m_serial->write(m_protocol->createGetConstantCurrentCommand());
            m_serial->write(m_protocol->createGetConstantVoltageCommand());
            m_serial->write(m_protocol->createGetConstantPowerCommand());
            m_serial->write(m_protocol->createGetConstantResistanceCommand());
        } else {
            // 读取动态测试的设置
            m_serial->write(m_protocol->createGetDynamicCurrentParamsCommand());
            m_serial->write(m_protocol->createGetDynamicVoltageParamsCommand());
            m_serial->write(m_protocol->createGetDynamicPowerParamsCommand());
            m_serial->write(m_protocol->createGetDynamicResistanceParamsCommand());
        }
    }
    
    // 更新输入限制
    updateInputLimits();
}

// 添加更新输入限制的方法
void IT8512Plus_Widget::updateInputLimits()
{
    double maxVoltage = m_setMaxVoltageSpinBox->value();
    double maxCurrent = m_setMCurrentSpinBox->value();
    double maxPower = m_setMPowerSpinBox->value();
    
    if (m_isBasicMode) {
        // 更新基础模式的输入限制
        m_valueSpinBox->setMaximum(maxCurrent);  // 假设当前是CC模式
        
        // 根据当前选择的模式设置限制
        if (m_ccModeRadio->isChecked()) {
            m_valueSpinBox->setMaximum(maxCurrent);
            m_valueSpinBox->setSuffix(" A");
        } else if (m_cvModeRadio->isChecked()) {
            m_valueSpinBox->setMaximum(maxVoltage);
            m_valueSpinBox->setSuffix(" V");
        } else if (m_cwModeRadio->isChecked()) {
            m_valueSpinBox->setMaximum(maxPower);
            m_valueSpinBox->setSuffix(" W");
        } else if (m_crModeRadio->isChecked()) {
            m_valueSpinBox->setMaximum(10000);  // 最大电阻
            m_valueSpinBox->setSuffix(" Ω");
        }
    } else {
        // 更新动态测试的输入限制
        if (m_dcModeRadio->isChecked()) {
            m_value1SpinBox->setMaximum(maxCurrent);
            m_value2SpinBox->setMaximum(maxCurrent);
            m_value1SpinBox->setSuffix(" A");
            m_value2SpinBox->setSuffix(" A");
        } else if (m_dvModeRadio->isChecked()) {
            m_value1SpinBox->setMaximum(maxVoltage);
            m_value2SpinBox->setMaximum(maxVoltage);
            m_value1SpinBox->setSuffix(" V");
            m_value2SpinBox->setSuffix(" V");
        } else if (m_dwModeRadio->isChecked()) {
            m_value1SpinBox->setMaximum(maxPower);
            m_value2SpinBox->setMaximum(maxPower);
            m_value1SpinBox->setSuffix(" W");
            m_value2SpinBox->setSuffix(" W");
        } else if (m_drModeRadio->isChecked()) {
            double maxResistance = maxVoltage/maxCurrent;
            m_value1SpinBox->setMaximum(maxResistance);
            m_value2SpinBox->setMaximum(maxResistance);
            m_value1SpinBox->setSuffix(" Ω");
            m_value2SpinBox->setSuffix(" Ω");
        }
    }
}

IT8512Plus_Widget::~IT8512Plus_Widget()
{
    if (m_statusTimer) {
        m_statusTimer->stop();
        delete m_statusTimer;
    }
    if (m_alertSound) {
        delete m_alertSound;
    }
} 
