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
    m_statusTimer->start();

    // 初始化提示音
    m_alertSound = new QSound(":/sounds/alert.wav", this);

    // 初始更新设备信息
    updateDeviceInfo();
}

void IT8512Plus_Widget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *topLayout = new QHBoxLayout();
    auto *bottomLayout = new QHBoxLayout();

    // 1. 创建设备信息区
    auto *infoGroup = new QGroupBox("设备信息", this);
    auto *infoLayout = new QVBoxLayout(infoGroup);
    m_modelLabel = new QLabel(infoGroup);
    m_versionLabel = new QLabel(infoGroup);
    m_serialLabel = new QLabel(infoGroup);
    infoLayout->addWidget(m_modelLabel);
    infoLayout->addWidget(m_versionLabel);
    infoLayout->addWidget(m_serialLabel);

    // 2. 创建基本设置区
    auto *basicGroup = new QGroupBox("基本设置", this);
    auto *basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel("最大电压(V):"), 0, 0);
    m_setMaxVoltageSpinBox = new QDoubleSpinBox(basicGroup);
    m_setMaxVoltageSpinBox->setRange(0, 150);
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

    // 动态参数设置连接
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
}

void IT8512Plus_Widget::updateDeviceInfo()
{
    if (!isConnected()) return;

    // 获取产品信息
    QByteArray cmd = m_protocol->createGetProductMessage();
    m_serial->write(cmd);
    
    if (m_serial->waitForResponse(100)) {
        QByteArray response = m_serial->readAll();
        QByteArray data;
        if (m_protocol->parseResponse(response, data)) {
            auto info = m_protocol->analyseProductMessage(data);
            m_modelLabel->setText(QString("型号: %1").arg(info.productModel));
            m_versionLabel->setText(QString("版本: %1").arg(info.softwareVersion));
            m_serialLabel->setText(QString("序列号: %1").arg(info.productSerial));
        }
    }
}

void IT8512Plus_Widget::updateStatus()
{
    if (!isConnected()) return;
    
    QByteArray cmd = m_protocol->createGetInputCommand();
    m_serial->write(cmd);
    
    if (m_serial->waitForResponse(100)) {
        QByteArray response = m_serial->readAll();
        QByteArray data;
        if (m_protocol->parseResponse(response, data)) {
            auto params = m_protocol->analyseInputParams(data);
            
            // 更新显示
            m_voltageLabel->setText(QString::number(params.voltage, 'f', 3) + " V");
            m_currentLabel->setText(QString::number(params.current, 'f', 3) + " A");
            m_powerLabel->setText(QString::number(params.power, 'f', 3) + " W");
            m_tempLabel->setText(QString::number(params.radiatorTemperature) + " ℃");

            // 更新控制状态
            bool isRemote = params.handleStateRegister.rem;
            m_controlModeBtn->setText(isRemote ? "切换到面板控制" : "切换到PC控制");
            
            bool isOutput = params.handleStateRegister.out;
            m_outputStateBtn->setText(isOutput ? "输出 ON" : "输出 OFF");

            // 检查目标值
            checkTargetValues();
            
            // 发送状态更新信号
            emit statusUpdated(params.voltage, params.current, params.power);
        }
    }
}

void IT8512Plus_Widget::onControlModeChanged()
{
    if (!isConnected()) return;
    
    QByteArray response;
    QByteArray cmd = m_protocol->createGetInputCommand();
    m_serial->write(cmd);
    
    if (m_serial->waitForResponse(100)) {
        response = m_serial->readAll();
        if (m_protocol->parseResponse(response, response)) {
            auto params = m_protocol->analyseInputParams(response);
            bool isRemote = params.handleStateRegister.rem;
            
            // 切换控制模式
            cmd = m_protocol->createControlModeCommand(isRemote ? 0 : 1);
            m_serial->write(cmd);
        }
    }
}

void IT8512Plus_Widget::onOutputStateChanged()
{
    if (!isConnected()) return;

    QByteArray cmd = m_protocol->createGetInputCommand();
    m_serial->write(cmd);
    
    if (m_serial->waitForResponse(100)) {
        QByteArray response = m_serial->readAll();
        QByteArray data;
        if (m_protocol->parseResponse(response, data)) {
            auto params = m_protocol->analyseInputParams(data);
            bool isOutput = params.handleStateRegister.out;
            
            // 切换输出状态
            cmd = m_protocol->createLoadStateCommand(isOutput ? 0 : 1);
            m_serial->write(cmd);
        }
    }
}

void IT8512Plus_Widget::onWorkModeChanged()
{
    if (!isConnected()) return;

    // 判断是基础模式还是动态模式
    bool isDynamic = m_dcModeRadio->isChecked() || m_dvModeRadio->isChecked() ||
                    m_dwModeRadio->isChecked() || m_drModeRadio->isChecked();
    
    m_workModeLabel->setText(isDynamic ? "动态测试模式" : "定值模式");
    
    // 设置工作类型
    QString type;
    uint8_t mode = 0;
    
    if (m_ccModeRadio->isChecked() || m_dcModeRadio->isChecked()) {
        type = "CC";
        mode = isDynamic ? 0x1A : 0x01;
    } else if (m_cvModeRadio->isChecked() || m_dvModeRadio->isChecked()) {
        type = "CV";
        mode = isDynamic ? 0x1B : 0x00;
    } else if (m_cwModeRadio->isChecked() || m_dwModeRadio->isChecked()) {
        type = "CW";
        mode = isDynamic ? 0x1C : 0x02;
    } else if (m_crModeRadio->isChecked() || m_drModeRadio->isChecked()) {
        type = "CR";
        mode = isDynamic ? 0x1D : 0x03;
    }
    
    m_workTypeLabel->setText(type);
    
    // 发送工作模式命令
    QByteArray cmd = m_protocol->createSetWorkModel(mode);
    m_serial->write(cmd);
}

void IT8512Plus_Widget::onValueSettingChanged()
{
    if (!isConnected()) return;

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
    if (!isConnected()) return;

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

    QByteArray cmd = m_protocol->createGetInputCommand();
    m_serial->write(cmd);
    
    if (m_serial->waitForResponse(100)) {
        QByteArray response = m_serial->readAll();
        QByteArray data;
        if (m_protocol->parseResponse(response, data)) {
            auto params = m_protocol->analyseInputParams(data);
            
            // 检查电压目标值范围
            double minVoltage = m_minVoltageSpinBox->value();
            double maxVoltage = m_maxVoltageSpinBox->value();
            if (minVoltage <= params.voltage && params.voltage <= maxVoltage) {
                if (!m_voltageTargetReached) {
                    m_voltageTargetReached = true;
                    m_alertSound->play();
                }
            } else {
                m_voltageTargetReached = false;
            }
            
            // 检查电流目标值范围
            double minCurrent = m_minCurrentSpinBox->value();
            double maxCurrent = m_maxCurrentSpinBox->value();
            if (minCurrent <= params.current && params.current <= maxCurrent) {
                if (!m_currentTargetReached) {
                    m_currentTargetReached = true;
                    m_alertSound->play();
                }
            } else {
                m_currentTargetReached = false;
            }
            
            // 检查功率目标值范围
            double minPower = m_minPowerSpinBox->value();
            double maxPower = m_maxPowerSpinBox->value();
            if (minPower <= params.power && params.power <= maxPower) {
                if (!m_powerTargetReached) {
                    m_powerTargetReached = true;
                    m_alertSound->play();
                }
            } else {
                m_powerTargetReached = false;
            }
        }
    }
}

bool IT8512Plus_Widget::connectToPort(const QString &portName)
{
    if (m_serial->connectToPort(portName)) {
        m_statusTimer->start();  // 连接成功后启动状态更新定时器
        emit serialConnected(portName);
        return true;
    }
    return false;
}

void IT8512Plus_Widget::handleSerialData(const QByteArray &data)
{
    // 累积接收的数据
    static QByteArray buffer;
    buffer.append(data);
    
    // 处理完整的数据包
    while (buffer.size() >= 26) {  // 假设一个完整数据包是26字节
        QByteArray packet = buffer.left(26);
        buffer.remove(0, 26);
        
        QByteArray parsedData;
        if (m_protocol->parseResponse(packet, parsedData)) {
            auto params = m_protocol->analyseInputParams(parsedData);
            
            // 更新UI显示
            m_voltageLabel->setText(QString::number(params.voltage, 'f', 3) + " V");
            m_currentLabel->setText(QString::number(params.current, 'f', 3) + " A");
            m_powerLabel->setText(QString::number(params.power, 'f', 3) + " W");
            m_tempLabel->setText(QString::number(params.radiatorTemperature) + " ℃");
            
            // 更新工作状态
            bool isRemote = params.handleStateRegister.rem;
            bool isOutput = params.handleStateRegister.out;
            m_controlModeBtn->setText(isRemote ? "切换到面板控制" : "切换到PC控制");
            m_outputStateBtn->setText(isOutput ? "输出 ON" : "输出 OFF");
            
            // 发送状态更新信号
            emit statusUpdated(params.voltage, params.current, params.power);
            
            // 检查目标值
            checkTargetValues();
        }
    }
}

// 示例：发送设置最大电压命令
void IT8512Plus_Widget::onMaxVoltageChanged(double value)
{
    if (m_serial && m_serial->isConnected()) {
        QByteArray cmd = m_protocol->createSetMaxVoltageCommand(value);
        m_serial->sendData(cmd);
    }
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
    emit this->error(error);
    
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
