#include "driverwidget.h"
#include <QHeaderView>
#include <QSerialPortInfo>

DriverWidget::DriverWidget(int channelCount, QWidget *parent)
    : QWidget(parent)
    , m_channelCount(channelCount)
    , m_serial(new SerialUtil(this))
    , m_driverGeneral(new DriverGeneral(this))
    , m_dataSendTimer(new QTimer(this))
    , m_dataChanged(false)
    , m_sendAddress(0x00)          // 默认发送地址
    , m_receiveAddress(0xFF)       // 默认接收地址
    , m_scanTimer(new QTimer(this))
    , m_connectionTimeoutTimer(new QTimer(this))
    , m_connectionPending(false)
{
    m_dataSendTimer->setSingleShot(true);
    m_dataSendTimer->setInterval(50); // 50ms防抖
    
    m_connectionTimeoutTimer->setSingleShot(true);
    m_connectionTimeoutTimer->setInterval(3000); // 3秒超时
    
    connect(m_connectionTimeoutTimer, &QTimer::timeout, this, &DriverWidget::handleConnectionTimeout);
    
    initUI();
    setupConnections();
}

void DriverWidget::initUI()
{
    // 创建主布局，使用水平布局分成三列
    auto *mainHLayout = new QHBoxLayout(this);
    mainHLayout->setSpacing(20);
    
    // 创建三个垂直布局容器
    auto *leftLayout = new QVBoxLayout();
    auto *middleLayout = new QVBoxLayout();
    auto *rightLayout = new QVBoxLayout();
    leftLayout->setSpacing(20);
    middleLayout->setSpacing(20);
    rightLayout->setSpacing(20);

    // 1. 基本信息区域 (左列)
    auto *infoGroup = new QGroupBox("基本信息", this);
    auto *infoLayout = new QGridLayout(infoGroup);
    
    m_addressEdit = new QLineEdit(this);
    m_addressConfirmBtn = new QPushButton("确认", this);
    m_addressConfirmBtn->setFixedWidth(60);
    
    auto *addressLayout = new QHBoxLayout();
    addressLayout->addWidget(m_addressEdit);
    addressLayout->addWidget(m_addressConfirmBtn);
    
    m_ledStatusLabel = new QLabel("关闭", this);
    m_ledTempLabel = new QLabel("0℃", this);
    m_pcbTempLabel = new QLabel("0℃", this);
    m_ratedVoltageLabel = new QLabel("0V", this);
    m_ratedCurrentLabel = new QLabel("0A", this);
    m_controlModeLabel = new QLabel("未知", this);
    m_alarmStatusLabel = new QLabel("正常", this);
    
    infoLayout->addWidget(new QLabel("从机地址:"), 0, 0);
    infoLayout->addLayout(addressLayout, 0, 1);
    infoLayout->addWidget(new QLabel("LED状态:"), 1, 0);
    infoLayout->addWidget(m_ledStatusLabel, 1, 1);
    infoLayout->addWidget(new QLabel("LED温度:"), 2, 0);
    infoLayout->addWidget(m_ledTempLabel, 2, 1);
    infoLayout->addWidget(new QLabel("PCB温度:"), 3, 0);
    infoLayout->addWidget(m_pcbTempLabel, 3, 1);
    infoLayout->addWidget(new QLabel("额定电压:"), 4, 0);
    infoLayout->addWidget(m_ratedVoltageLabel, 4, 1);
    infoLayout->addWidget(new QLabel("额定电流:"), 5, 0);
    infoLayout->addWidget(m_ratedCurrentLabel, 5, 1);
    infoLayout->addWidget(new QLabel("控制模式:"), 6, 0);
    infoLayout->addWidget(m_controlModeLabel, 6, 1);
    infoLayout->addWidget(new QLabel("告警状态:"), 7, 0);
    infoLayout->addWidget(m_alarmStatusLabel, 7, 1);
    
    leftLayout->addWidget(infoGroup);

    // 2. 调控区域
    auto *controlGroup = new QGroupBox("调控区", this);
    auto *controlLayout = new QGridLayout(controlGroup);
    
    // 模式切换
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItems({"模式1", "模式2", "模式3"});
    
    // LED开关
    m_ledStatus = false;
    m_ledSwitch = new QPushButton("打开LED", this);
    m_ledSwitch->setStyleSheet("QPushButton { background-color: #d9534f; color: white; }");
    
    // 寄存器管理
    m_startRegBox = new QSpinBox(this);
    m_startRegBox->setRange(1, m_channelCount);
    m_regCountBox = new QSpinBox(this);
    m_regCountBox->setRange(1, m_channelCount);
    
    // 连接起始寄存器值变化信号
    connect(m_startRegBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
        // 更新寄存器数量的范围
        int maxCount = m_channelCount - value + 1;
        m_regCountBox->setRange(1, maxCount);
        // 重置寄存器数量为1
        m_regCountBox->setValue(1);
    });
    
    // 灯时长
    m_ledTimeEdit = new QSpinBox(this);
    m_ledTimeEdit->setRange(0, 65535);
    m_ledTimeConfirmBtn = new QPushButton("确认", this);
    m_ledTimeConfirmBtn->setFixedWidth(60);
    auto *ledTimeLayout = new QHBoxLayout();
    ledTimeLayout->addWidget(m_ledTimeEdit);
    ledTimeLayout->addWidget(m_ledTimeConfirmBtn);
    
    // 最大电压
    m_maxVoltageEdit = new QDoubleSpinBox(this);
    m_maxVoltageEdit->setRange(0, 500);
    m_maxVoltageEdit->setDecimals(2);  // 设置小数位数
    m_maxVoltageConfirmBtn = new QPushButton("确认", this);
    m_maxVoltageConfirmBtn->setFixedWidth(60);
    auto *maxVoltageLayout = new QHBoxLayout();
    maxVoltageLayout->addWidget(m_maxVoltageEdit);
    maxVoltageLayout->addWidget(m_maxVoltageConfirmBtn);
    
    // 最大电流
    m_maxCurrentEdit = new QDoubleSpinBox(this);
    m_maxCurrentEdit->setRange(0, 200);
    m_maxCurrentEdit->setDecimals(2);  // 设置小数位数
    m_maxCurrentConfirmBtn = new QPushButton("确认", this);
    m_maxCurrentConfirmBtn->setFixedWidth(60);
    auto *maxCurrentLayout = new QHBoxLayout();
    maxCurrentLayout->addWidget(m_maxCurrentEdit);
    maxCurrentLayout->addWidget(m_maxCurrentConfirmBtn);
    
    // 添加到调控区布局
    controlLayout->addWidget(new QLabel("模式选择:"), 0, 0);
    controlLayout->addWidget(m_modeCombo, 0, 1);
    controlLayout->addWidget(new QLabel("LED开关:"), 1, 0);
    controlLayout->addWidget(m_ledSwitch, 1, 1);
    controlLayout->addWidget(new QLabel("起始寄存器:"), 2, 0);
    controlLayout->addWidget(m_startRegBox, 2, 1);
    controlLayout->addWidget(new QLabel("寄存器数量:"), 3, 0);
    controlLayout->addWidget(m_regCountBox, 3, 1);
    controlLayout->addWidget(new QLabel("灯时长:"), 4, 0);
    controlLayout->addLayout(ledTimeLayout, 4, 1);
    controlLayout->addWidget(new QLabel("最大电压:"), 5, 0);
    controlLayout->addLayout(maxVoltageLayout, 5, 1);
    controlLayout->addWidget(new QLabel("最大电流:"), 6, 0);
    controlLayout->addLayout(maxCurrentLayout, 6, 1);
    
    leftLayout->addWidget(controlGroup);

    // 3. 扫描测试区域 (中列)
    auto *scanGroup = new QGroupBox("扫描测试", this);
    auto *scanLayout = new QGridLayout(scanGroup);
    
    m_scanTargetCombo = new QComboBox(this);
    m_scanTargetCombo->addItem("通道值");
    
    // 添加测试起始值
    m_startValueBox = new QSpinBox(this);
    m_startValueBox->setRange(0, 255);
    
    m_stepValueBox = new QSpinBox(this);
    m_stepValueBox->setRange(0, 255);
    m_intervalBox = new QSpinBox(this);
    m_intervalBox->setRange(0, 65535);
    m_endValueBox = new QSpinBox(this);
    m_endValueBox->setRange(0, 255);
    
    m_increaseBtn = new QPushButton("递增", this);
    m_decreaseBtn = new QPushButton("递减", this);
    m_stopScanBtn = new QPushButton("停止", this);
    
    scanLayout->addWidget(new QLabel("扫描目标:"), 0, 0);
    scanLayout->addWidget(m_scanTargetCombo, 0, 1);
    scanLayout->addWidget(new QLabel("起始值:"), 1, 0);
    scanLayout->addWidget(m_startValueBox, 1, 1);
    scanLayout->addWidget(new QLabel("步长值:"), 2, 0);
    scanLayout->addWidget(m_stepValueBox, 2, 1);
    scanLayout->addWidget(new QLabel("时间间隔(ms):"), 3, 0);
    scanLayout->addWidget(m_intervalBox, 3, 1);
    scanLayout->addWidget(new QLabel("终止值:"), 4, 0);
    scanLayout->addWidget(m_endValueBox, 4, 1);
    
    auto *scanBtnLayout = new QHBoxLayout();
    scanBtnLayout->addWidget(m_increaseBtn);
    scanBtnLayout->addWidget(m_decreaseBtn);
    scanBtnLayout->addWidget(m_stopScanBtn);
    scanLayout->addLayout(scanBtnLayout, 5, 0, 1, 2);
    
    middleLayout->addWidget(scanGroup);

    // 4. 参数表区域 (中列)
    auto *paramGroup = new QGroupBox("参数表", this);
    auto *paramLayout = new QVBoxLayout(paramGroup);
    
    m_paramTable = new QTableWidget(m_channelCount, 2, this);
    m_paramTable->setHorizontalHeaderLabels({"通道", "数值"});
    m_paramTable->horizontalHeader()->setStretchLastSection(true);
    m_paramTable->verticalHeader()->setVisible(false);
    
    // 初始化参数表数据
    for(int i = 0; i < m_channelCount; i++) {
        m_paramTable->setItem(i, 0, new QTableWidgetItem(QString("通道%1").arg(i+1)));
        m_paramTable->setItem(i, 1, new QTableWidgetItem("0"));
    }
    
    paramLayout->addWidget(m_paramTable);
    middleLayout->addWidget(paramGroup);

    // 5. 滑条控制区域
    auto *sliderGroup = new QGroupBox("滑条控制", this);
    auto *sliderLayout = new QVBoxLayout(sliderGroup);
    
    // 控制目标
    auto *targetLayout = new QHBoxLayout();
    m_sliderTargetCombo = new QComboBox(this);
    m_sliderTargetCombo->addItem("通道值");
    targetLayout->addWidget(new QLabel("控制目标:"));
    targetLayout->addWidget(m_sliderTargetCombo);
    sliderLayout->addLayout(targetLayout);
    
    // 总控滑条
    auto *masterLayout = new QHBoxLayout();
    m_masterSlider = new QSlider(Qt::Horizontal, this);
    m_masterValueEdit = new QSpinBox(this);
    m_masterValueEdit->setRange(0, 255);
    m_masterSlider->setRange(0, 255);
    masterLayout->addWidget(new QLabel("总 控:"));
    masterLayout->addWidget(m_masterSlider);
    masterLayout->addWidget(m_masterValueEdit);
    sliderLayout->addLayout(masterLayout);
    
    // 单控滑条
    for(int i = 0; i < m_channelCount; i++) {
        auto *channelLayout = new QHBoxLayout();
        QSlider* slider = new QSlider(Qt::Horizontal, this);
        QSpinBox* spin = new QSpinBox(this);
        
        slider->setRange(0, 255);
        spin->setRange(0, 255);
        
        m_channelSliders.append(slider);
        m_channelValueSpins.append(spin);
        
        channelLayout->addWidget(new QLabel(QString("通道%1:").arg(i+1)));
        channelLayout->addWidget(slider);
        channelLayout->addWidget(spin);
        sliderLayout->addLayout(channelLayout);
    }
    
    rightLayout->addWidget(sliderGroup);

    // 将三列添加到主布局
    mainHLayout->addLayout(leftLayout);
    mainHLayout->addLayout(middleLayout);
    mainHLayout->addLayout(rightLayout);
    
    // 设置列的比例
    mainHLayout->setStretch(0, 2);  // 左列
    mainHLayout->setStretch(1, 2);  // 中列
    mainHLayout->setStretch(2, 3);  // 右列(滑条区域需要更多空间)

    // 设置扫描按钮样式
    m_stopScanBtn->setEnabled(false);
    m_stopScanBtn->setStyleSheet("QPushButton:disabled { background-color: #cccccc; }");
    m_increaseBtn->setStyleSheet("QPushButton:enabled { background-color: #5cb85c; color: white; }"
                                "QPushButton:disabled { background-color: #cccccc; }");
    m_decreaseBtn->setStyleSheet("QPushButton:enabled { background-color: #5cb85c; color: white; }"
                                "QPushButton:disabled { background-color: #cccccc; }");
}

void DriverWidget::setupConnections()
{
    // 确保创建一个正确的 m_driverGeneral 实例
    if (!m_driverGeneral) {
        m_driverGeneral = new DriverGeneral(this);
    }
    
    // 连接扫描定时器
    connect(m_scanTimer, &QTimer::timeout, this, &DriverWidget::onScanTimeout);
    
    // 连接滑条信号
    connect(m_masterSlider, &QSlider::valueChanged,
            m_masterValueEdit, &QSpinBox::setValue);
    connect(m_masterValueEdit, QOverload<int>::of(&QSpinBox::valueChanged),
            m_masterSlider, &QSlider::setValue);
    connect(m_masterSlider, &QSlider::valueChanged,
            this, &DriverWidget::onMasterValueChanged);
    
    for(int i = 0; i < m_channelSliders.size(); i++) {
        connect(m_channelSliders[i], &QSlider::valueChanged,
                m_channelValueSpins[i], &QSpinBox::setValue);
        connect(m_channelValueSpins[i], QOverload<int>::of(&QSpinBox::valueChanged),
                m_channelSliders[i], &QSlider::setValue);
        connect(m_channelSliders[i], &QSlider::valueChanged,
                this, [this, i](int value) { onChannelValueChanged(i, value); });
    }

    connect(m_stopScanBtn, &QPushButton::clicked, this, &DriverWidget::stopScan);

    // LED开关按钮 - 更新使用协议命令
    connect(m_ledSwitch, &QPushButton::clicked, this, [this]() {
        m_ledStatus = !m_ledStatus;
        if (m_ledStatus) {
            m_ledSwitch->setText("关闭LED");
            m_ledSwitch->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; }");
        } else {
            m_ledSwitch->setText("打开LED");
            m_ledSwitch->setStyleSheet("QPushButton { background-color: #d9534f; color: white; }");
        }
        // 发送LED状态命令
        sendWriteLEDStatusCommand(m_ledStatus);
    });

    // 扫描按钮连接
    connect(m_increaseBtn, &QPushButton::clicked, this, [this]() {
        startScan(true);
    });

    connect(m_decreaseBtn, &QPushButton::clicked, this, [this]() {
        startScan(false);
    });

    // 串口连接
    connect(m_serial, &SerialUtil::dataReceived, this, &DriverWidget::handleSerialData);
    connect(m_serial, &SerialUtil::portDisconnected, this, [this]() {
        emit serialDisconnected();
    });
    
    // 数据发送定时器
    connect(m_dataSendTimer, &QTimer::timeout, this, &DriverWidget::onDataSendTimerTimeout);
    
    // 从机地址确认按钮 - 更新为更改设备地址
    connect(m_addressConfirmBtn, &QPushButton::clicked, this, [this]() {
        bool ok;
        quint8 newAddress = static_cast<quint8>(m_addressEdit->text().toUInt(&ok, 16));
        if (!ok) {
            QMessageBox::warning(this, "地址格式错误", "请输入有效的十六进制地址");
            return;
        }
        
        // 如果当前已连接，发送更改地址命令
        if (isConnected() && m_driverGeneral) {
            QByteArray cmd = m_driverGeneral->writeDriverAddress(
                m_sendAddress, m_receiveAddress, newAddress);
            m_serial->enqueueData(cmd);
            
            // 更新当前接收地址
            m_receiveAddress = newAddress;
        }
        
        emit settingsChanged();
    });
    
    // 确认按钮 - 更新使用协议命令
    connect(m_ledTimeConfirmBtn, &QPushButton::clicked, this, [this]() {
        if (isConnected() && m_driverGeneral) {
            QByteArray cmd = m_driverGeneral->writeLEDWorkTime(
                m_sendAddress, m_receiveAddress, m_ledTimeEdit->value());
            m_serial->enqueueData(cmd);
        }
    });
    
    connect(m_maxVoltageConfirmBtn, &QPushButton::clicked, this, [this]() {
        sendWriteLimitVoltageCurrentCommand();
    });
    
    connect(m_maxCurrentConfirmBtn, &QPushButton::clicked, this, [this]() {
        sendWriteLimitVoltageCurrentCommand();
    });
}

// 析构函数实现
DriverWidget::~DriverWidget()
{
    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
    
    if (m_dataSendTimer->isActive()) {
        m_dataSendTimer->stop();
    }
    
    disconnectPort();
}

// 修改总控滑条值改变函数
void DriverWidget::onMasterValueChanged(int value)
{
    // 更新所有单控滑条
    for (int i = 0; i < m_channelSliders.size(); ++i) {
        m_channelSliders[i]->blockSignals(true); // 阻止发送信号
        m_channelSliders[i]->setValue(value);
        m_channelSliders[i]->blockSignals(false);
        
        m_channelValueSpins[i]->blockSignals(true); // 阻止发送信号
        m_channelValueSpins[i]->setValue(value);
        m_channelValueSpins[i]->blockSignals(false);
    }
    
    // 更新参数表
    updateParamTable();
    
    // 不使用scheduleSendData，直接发送所有通道数据
    if (isConnected() && m_driverGeneral) {
        sendAllChannelsData(value);
    }
    
    // 发送通道值变化信号
    QVector<int> values;
    for (int i = 0; i < m_channelCount; ++i) {
        values.append(value);
    }
    emit channelValuesChanged(values);
}

// 添加发送所有通道数据的方法
void DriverWidget::sendAllChannelsData(int value)
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    // 使用起始寄存器1，寄存器数量为通道数
    quint8 startReg = 1;
    quint8 regCount = static_cast<quint8>(m_channelCount);
    
    // 构建所有通道的数据
    QByteArray valueData;
    for (int i = 0; i < m_channelCount; ++i) {
        quint16 channelValue = static_cast<quint16>(value);
        // 添加高字节和低字节（大端序）
        valueData.append(static_cast<char>((channelValue >> 8) & 0xFF));
        valueData.append(static_cast<char>(channelValue & 0xFF));
    }
    
    QByteArray cmd = m_driverGeneral->writeLEDStrength(
        m_sendAddress, m_receiveAddress, startReg, regCount, valueData);
    m_serial->enqueueData(cmd);
}

// 单控滑条值改变
void DriverWidget::onChannelValueChanged(int index, int value)
{
    if (index < 0 || index >= m_channelCount) {
        return;
    }
    
    // 更新数值框
    m_channelValueSpins[index]->blockSignals(true);
    m_channelValueSpins[index]->setValue(value);
    m_channelValueSpins[index]->blockSignals(false);
    
    // 更新参数表
    updateParamTable();
    
    // 安排发送数据 - 防止频繁发送
    scheduleSendData();
    
    // 发送通道值变化信号
    QVector<int> values;
    for (int i = 0; i < m_channelCount; ++i) {
        values.append(m_channelValueSpins[i]->value());
    }
    emit channelValuesChanged(values);
}

// 扫描定时器处理
void DriverWidget::onScanTimeout()
{
    int currentValue = m_startValueBox->value();  // 使用起始值
    int stepValue = m_stepValueBox->value();
    int endValue = m_endValueBox->value();
    int startReg = m_startRegBox->value() - 1;  // 转换为0基索引
    int regCount = m_regCountBox->value();
    
    // 根据扫描方向更新值
    if (m_scanTimer->property("isIncrease").toBool()) {
        currentValue += stepValue;
        if (currentValue > endValue) {
            stopScan();
            return;
        }
    } else {
        currentValue -= stepValue;
        if (currentValue < endValue) {
            stopScan();
            return;
        }
    }
    
    // 更新指定范围的通道值
    for (int i = startReg; i < startReg + regCount && i < m_channelCount; ++i) {
        if (i >= 0 && i < m_channelSliders.size()) {
            m_channelSliders[i]->setValue(currentValue);
            m_channelValueSpins[i]->setValue(currentValue);
        }
    }
    
    // 更新起始值
    m_startValueBox->setValue(currentValue);
    
    // 更新参数表
    updateParamTable();
    
    // 发送更新后的通道值
    scheduleSendData();
}

// 更新参数表
void DriverWidget::updateParamTable()
{
    for (int i = 0; i < m_channelCount; ++i) {
        // 设置通道号
        QTableWidgetItem* channelItem = m_paramTable->item(i, 0);
        if (!channelItem) {
            channelItem = new QTableWidgetItem(QString("通道%1").arg(i + 1));
            m_paramTable->setItem(i, 0, channelItem);
        }
        
        // 设置通道值
        QTableWidgetItem* valueItem = m_paramTable->item(i, 1);
        if (!valueItem) {
            valueItem = new QTableWidgetItem();
            m_paramTable->setItem(i, 1, valueItem);
        }
        valueItem->setText(QString::number(m_channelValueSpins[i]->value()));
    }
}

// 开始扫描
void DriverWidget::startScan(bool isIncrease)
{
    // 验证起始值和终止值
    int startValue = m_startValueBox->value();
    int endValue = m_endValueBox->value();
    
    if (isIncrease && startValue >= endValue) {
        // 如果是递增，起始值必须小于终止值
        return;
    }
    if (!isIncrease && startValue <= endValue) {
        // 如果是递减，起始值必须大于终止值
        return;
    }
    
    // 设置扫描方向
    m_scanTimer->setProperty("isIncrease", isIncrease);
    
    // 设置扫描间隔
    m_scanTimer->setInterval(m_intervalBox->value());
    
    // 禁用相关控件
    m_increaseBtn->setEnabled(false);
    m_decreaseBtn->setEnabled(false);
    m_stepValueBox->setEnabled(false);
    m_intervalBox->setEnabled(false);
    m_endValueBox->setEnabled(false);
    m_startValueBox->setEnabled(false);
    
    // 启用停止按钮
    m_stopScanBtn->setEnabled(true);
    
    // 启动定时器
    m_scanTimer->start();
}

// 停止扫描
void DriverWidget::stopScan()
{
    if (m_scanTimer) {
        m_scanTimer->stop();  // 确保定时器停止
    }
    
    // 启用相关控件
    m_increaseBtn->setEnabled(true);
    m_decreaseBtn->setEnabled(true);
    m_stepValueBox->setEnabled(true);
    m_intervalBox->setEnabled(true);
    m_endValueBox->setEnabled(true);
    m_startValueBox->setEnabled(true);
    
    // 禁用停止按钮
    m_stopScanBtn->setEnabled(false);
}

// 串口连接管理
bool DriverWidget::isConnected() const
{
    return m_serial && m_serial->isConnected();
}

void DriverWidget::connectToPort(const QString &portName)
{
    // 如果正在处理连接，先取消
    if (m_connectionPending) {
        m_connectionTimeoutTimer->stop();
        m_connectionPending = false;
    }
    
    // 连接物理串口
    if (m_serial->connectToPort(portName, 115200)) {
        // 保存当前待连接端口名
        m_pendingPortName = portName;
        m_connectionPending = true;
        
        // 设置默认地址
        m_receiveAddress = 0xFF;
        
        // 发送初始化命令
        sendInitCommand();
        
        // 启动超时定时器
        m_connectionTimeoutTimer->start();
    } else {
        emit serialError("无法连接到串口 " + portName);
    }
}

void DriverWidget::disconnectPort()
{
    if (m_serial && m_serial->isConnected()) {
        m_serial->disconnectPort();
        emit serialDisconnected();  // 确保发送断开信号
    }
}

// 数据发送安排 - 防止频繁发送
void DriverWidget::scheduleSendData()
{
    m_dataChanged = true;
    if (!m_dataSendTimer->isActive()) {
        m_dataSendTimer->start();
    }
}

// 数据发送定时器超时 - 实际发送数据
void DriverWidget::onDataSendTimerTimeout()
{
    if (m_dataChanged) {
        sendData();
        m_dataChanged = false;
    }
}

// 实际发送数据
void DriverWidget::sendData()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    // 使用通信协议发送LED强度命令
    sendWriteLEDStrengthCommand();
}

// 创建通道命令
QByteArray DriverWidget::makeChannelCommand()
{
    // 简单的命令格式: 地址(1字节) + 命令(1字节) + 起始寄存器(1字节) + 
    // 寄存器数量(1字节) + 数据(n字节) + 校验(1字节)
    QByteArray cmd;
    int address = m_addressEdit->text().toInt();
    int startReg = m_startRegBox->value();
    int regCount = m_regCountBox->value();
    
    cmd.append(static_cast<char>(address)); // 地址
    cmd.append(0x10); // 命令码 (0x10为示例，实际根据协议定义)
    cmd.append(static_cast<char>(startReg)); // 起始寄存器
    cmd.append(static_cast<char>(regCount)); // 寄存器数量
    
    // 添加通道值数据
    for (int i = startReg - 1; i < startReg - 1 + regCount && i < m_channelCount; ++i) {
        if (i >= 0 && i < m_channelSliders.size()) {
            cmd.append(static_cast<char>(m_channelValueSpins[i]->value()));
        }
    }
    
    // 简单校验 - 累加和
    char checksum = 0;
    for (int i = 0; i < cmd.size(); ++i) {
        checksum += static_cast<unsigned char>(cmd.at(i));
    }
    cmd.append(checksum);
    
    return cmd;
}

// 创建所有通道命令
QByteArray DriverWidget::makeAllChannelsCommand(int value)
{
    // 简单的命令格式: 地址(1字节) + 命令(1字节) + 数据(1字节) + 校验(1字节)
    QByteArray cmd;
    int address = m_addressEdit->text().toInt();
    
    cmd.append(static_cast<char>(address)); // 地址
    cmd.append(0x11); // 命令码 (0x11为示例，实际根据协议定义)
    cmd.append(static_cast<char>(value)); // 所有通道值
    
    // 简单校验 - 累加和
    char checksum = 0;
    for (int i = 0; i < cmd.size(); ++i) {
        checksum += static_cast<unsigned char>(cmd.at(i));
    }
    cmd.append(checksum);
    
    return cmd;
}

// 处理接收到的串口数据
void DriverWidget::handleSerialData(const QByteArray &data)
{
    if (!m_driverGeneral || data.size() < 9) {
        // 数据太短，无法解析
        return;
    }
    
    // 确认起始字符
    quint16 stxValue = ((static_cast<quint16>(data[0]) << 8) | static_cast<quint8>(data[1]));
    if (stxValue != 0x4C44) { // 'LD'
        qDebug() << "起始字符错误";
        return;
    }
    
    quint8 len = static_cast<quint8>(data[2]);      // 命令长度
    quint8 action = static_cast<quint8>(data[3]);   // 命令类型（读/写）
    quint8 sender = static_cast<quint8>(data[4]);   // 发送者地址
    quint8 receiver = static_cast<quint8>(data[5]); // 接收者地址
    quint8 function = static_cast<quint8>(data[6]);  // 功能码
    
    // 提取数据部分
    QByteArray payload = data.mid(7, len - 3); // 去掉len, action, sender, receiver, function后剩余的数据
    
    // 如果正在等待连接响应，且收到初始化响应
    if (m_connectionPending && function == 0x08) {
        m_connectionPending = false;
        m_connectionTimeoutTimer->stop();
        
        // 更新接收地址
        m_receiveAddress = sender;
        m_addressEdit->setText(QString::number(m_receiveAddress, 16).toUpper());
        
        // 通知连接成功
        emit serialConnected(m_pendingPortName);
        
        // 获取设备状态
        sendReadLEDStatusCommand();
        sendReadTemperatureCommand();
        sendReadVoltageCurrentCommand();
    }
    
    // 根据功能码进行处理
    switch (function) {
        case 0x08: { // 初始化响应
            DriverGeneral::DriverMessage message = m_driverGeneral->parseInit(payload);
            // 更新UI显示
            m_ratedVoltageLabel->setText(QString("%1V").arg(message.maxV / 100.0));
            m_ratedCurrentLabel->setText(QString("%1A").arg(message.maxA / 100.0));
            break;
        }
        case 0x1B: { // 地址响应
            char newAddress = m_driverGeneral->parseWrite1Byte(payload);
            m_addressEdit->setText(QString::number(newAddress, 16).toUpper());
            m_receiveAddress = static_cast<quint8>(newAddress);
            break;
        }
        case 0x1C: { // 温度响应
            DriverGeneral::Temperatures temps = m_driverGeneral->parseTemperature(payload);
            // 更新温度显示
            m_ledTempLabel->setText(QString("%1℃").arg(temps.LEDTemperature / 100.0));
            m_pcbTempLabel->setText(QString("%1℃").arg(temps.PCBTemperature / 100.0));
            break;
        }
        case 0x24: { // LED开关状态响应
            if (action == 0x81) { // 读响应
                quint16 ledStatus = m_driverGeneral->parseWrite2Byte(payload);
                // 更新LED状态显示
                m_ledStatus = (ledStatus == 0x0001);
                m_ledStatusLabel->setText(m_ledStatus ? "开启" : "关闭");
                m_ledSwitch->setText(m_ledStatus ? "关闭LED" : "打开LED");
                m_ledSwitch->setStyleSheet(m_ledStatus ? 
                    "QPushButton { background-color: #5cb85c; color: white; }" :
                    "QPushButton { background-color: #d9534f; color: white; }");
            }
            break;
        }
        case 0x26: { // LED强度响应
            if (action == 0x81) { // 读响应
                DriverGeneral::ChannelValue channelValues = m_driverGeneral->parseChannelValue(payload);
                // 更新通道值显示
                for (int i = 0; i < channelValues.chValue.size(); ++i) {
                    int channelIndex = channelValues.startRegister - 1 + i;
                    if (channelIndex >= 0 && channelIndex < m_channelSliders.size()) {
                        m_channelSliders[channelIndex]->setValue(channelValues.chValue[i]);
                        m_channelValueSpins[channelIndex]->setValue(channelValues.chValue[i]);
                    }
                }
                updateParamTable();
            }
            break;
        }
        case 0x50: { // LED模式响应
            quint16 ledMode = m_driverGeneral->parseWrite2Byte(payload);
            // 更新模式显示
            m_modeCombo->setCurrentIndex(ledMode);
            break;
        }
        case 0x52: { // LED工作时间响应
            quint32 ledTime = m_driverGeneral->parseWrite4Byte(payload);
            m_ledTimeEdit->setValue(static_cast<int>(ledTime));
            break;
        }
        case 0x56: { // 电压电流响应
            DriverGeneral::CurrentPower power = m_driverGeneral->parsePower(payload);
            // 这里可以添加UI组件来显示当前电压电流
            break;
        }
        case 0x5E: { // 最大电压电流响应
            // 只针对写操作，不需要解析
            break;
        }
        case 0x60: { // 清除报警响应
            // 只针对写操作，不需要解析
            break;
        }
    }
}

// 修复DriverWidget.cpp中的handleSerialError方法 - 重命名为onSerialPortError
void DriverWidget::onSerialPortError(QSerialPort::SerialPortError error)
{
    QString errorMsg;
    switch (error) {
        case QSerialPort::DeviceNotFoundError:
            errorMsg = "设备未找到";
            break;
        case QSerialPort::PermissionError:
            errorMsg = "无权限访问设备";
            break;
        case QSerialPort::OpenError:
            errorMsg = "无法打开设备";
            break;
        case QSerialPort::NotOpenError:
            errorMsg = "设备未打开";
            break;
        case QSerialPort::WriteError:
            errorMsg = "写入错误";
            break;
        case QSerialPort::ReadError:
            errorMsg = "读取错误";
            break;
        case QSerialPort::ResourceError:
            errorMsg = "设备已断开";
            emit serialDisconnected();
            break;
        default:
            errorMsg = "未知错误";
            break;
    }
    emit serialError(errorMsg);
}

// 获取设置
QJsonObject DriverWidget::getSettings() const
{
    QJsonObject settings;
    
    // 保存基本设置
    settings["address"] = m_addressEdit->text();
    settings["ledStatus"] = m_ledStatus;
    
    // 保存寄存器设置
    settings["startRegister"] = m_startRegBox->value();
    settings["registerCount"] = m_regCountBox->value();
    
    // 保存其他设置
    settings["ledTime"] = m_ledTimeEdit->value();
    settings["maxVoltage"] = m_maxVoltageEdit->value();
    settings["maxCurrent"] = m_maxCurrentEdit->value();
    
    // 保存通道值
    QJsonArray channelValues;
    for (int i = 0; i < m_channelCount; ++i) {
        channelValues.append(m_channelValueSpins[i]->value());
    }
    settings["channelValues"] = channelValues;
    
    return settings;
}

// 应用设置
void DriverWidget::applySettings(const QJsonObject &settings)
{
    // 恢复基本设置
    if (settings.contains("address")) {
        m_addressEdit->setText(settings["address"].toString());
    }
    
    if (settings.contains("ledStatus")) {
        m_ledStatus = settings["ledStatus"].toBool();
        if (m_ledStatus) {
            m_ledSwitch->setText("关闭LED");
            m_ledSwitch->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; }");
        } else {
            m_ledSwitch->setText("打开LED");
            m_ledSwitch->setStyleSheet("QPushButton { background-color: #d9534f; color: white; }");
        }
    }
    
    // 恢复寄存器设置
    if (settings.contains("startRegister")) {
        m_startRegBox->setValue(settings["startRegister"].toInt());
    }
    
    if (settings.contains("registerCount")) {
        m_regCountBox->setValue(settings["registerCount"].toInt());
    }
    
    // 恢复其他设置
    if (settings.contains("ledTime")) {
        m_ledTimeEdit->setValue(settings["ledTime"].toInt());
    }
    
    if (settings.contains("maxVoltage")) {
        m_maxVoltageEdit->setValue(settings["maxVoltage"].toDouble());
    }
    
    if (settings.contains("maxCurrent")) {
        m_maxCurrentEdit->setValue(settings["maxCurrent"].toDouble());
    }
    
    // 恢复通道值
    if (settings.contains("channelValues")) {
        QJsonArray channelValues = settings["channelValues"].toArray();
        for (int i = 0; i < qMin(channelValues.size(), m_channelCount); ++i) {
            int value = channelValues[i].toInt();
            m_channelSliders[i]->setValue(value);
            m_channelValueSpins[i]->setValue(value);
        }
    }
    
    // 更新参数表
    updateParamTable();
}

// 初始化驱动器连接
void DriverWidget::initDriverConnection()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    // 从界面上获取设备地址
    bool ok;
    m_receiveAddress = m_addressEdit->text().toUInt(&ok, 16);
    if (!ok) {
        // 地址格式错误，使用默认地址
        m_receiveAddress = 0xFF;
        m_addressEdit->setText(QString::number(m_receiveAddress, 16).toUpper());
    }
    
    // 发送初始化命令
    sendInitCommand();
    
    // 获取LED状态
    sendReadLEDStatusCommand();
    
    // 获取温度
    sendReadTemperatureCommand();
    
    // 获取电压电流
    sendReadVoltageCurrentCommand();
}

// 发送初始化命令
void DriverWidget::sendInitCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    QByteArray cmd = m_driverGeneral->connectInit(m_sendAddress, m_receiveAddress);
    m_serial->enqueueData(cmd);
}

// 发送读取温度命令
void DriverWidget::sendReadTemperatureCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    QByteArray cmd = m_driverGeneral->readTemperature(m_sendAddress, m_receiveAddress);
    m_serial->enqueueData(cmd);
}

// 发送读取LED状态命令
void DriverWidget::sendReadLEDStatusCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    QByteArray cmd = m_driverGeneral->readLEDOnOff(m_sendAddress, m_receiveAddress);
    m_serial->enqueueData(cmd);
}

// 发送设置LED状态命令
void DriverWidget::sendWriteLEDStatusCommand(bool on)
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    quint16 status = on ? 0x0001 : 0x0000;
    QByteArray cmd = m_driverGeneral->writeLEDOnOff(m_sendAddress, m_receiveAddress, status);
    m_serial->enqueueData(cmd);
}

// 发送读取LED强度命令
void DriverWidget::sendReadLEDStrengthCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    quint8 startReg = static_cast<quint8>(m_startRegBox->value());
    quint8 regCount = static_cast<quint8>(m_regCountBox->value());
    
    QByteArray cmd = m_driverGeneral->readLEDStrength(
        m_sendAddress, m_receiveAddress, startReg, regCount);
    m_serial->enqueueData(cmd);
}

// 发送设置LED强度命令
void DriverWidget::sendWriteLEDStrengthCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    quint8 startReg = static_cast<quint8>(m_startRegBox->value());
    quint8 regCount = static_cast<quint8>(m_regCountBox->value());
    
    // 构建数据
    QByteArray valueData;
    for (int i = startReg - 1; i < startReg - 1 + regCount && i < m_channelCount; ++i) {
        if (i >= 0 && i < m_channelValueSpins.size()) {
            quint16 value = static_cast<quint16>(m_channelValueSpins[i]->value());
            // 添加高字节和低字节（大端序）
            valueData.append(static_cast<char>((value >> 8) & 0xFF));
            valueData.append(static_cast<char>(value & 0xFF));
        }
    }
    
    QByteArray cmd = m_driverGeneral->writeLEDStrength(
        m_sendAddress, m_receiveAddress, startReg, regCount, valueData);
    m_serial->enqueueData(cmd);
}

// 发送读取电压电流命令
void DriverWidget::sendReadVoltageCurrentCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    QByteArray cmd = m_driverGeneral->readVoltageCurrent(m_sendAddress, m_receiveAddress);
    m_serial->enqueueData(cmd);
}

// 发送设置限制电压电流命令
void DriverWidget::sendWriteLimitVoltageCurrentCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    // 将面板上的值转换为需要的格式（例如：乘以100转为整数）
    quint32 maxVoltage = static_cast<quint32>(m_maxVoltageEdit->value() * 100);
    quint32 maxCurrent = static_cast<quint32>(m_maxCurrentEdit->value() * 100);
    
    QByteArray cmd = m_driverGeneral->writeLimitVoltageCurrent(
        m_sendAddress, m_receiveAddress, maxVoltage, maxCurrent);
    m_serial->enqueueData(cmd);
}

// 发送清除告警命令
void DriverWidget::sendWriteClearAlarmCommand()
{
    if (!isConnected() || !m_driverGeneral) {
        return;
    }
    
    QByteArray cmd = m_driverGeneral->writeClearAlarm(m_sendAddress, m_receiveAddress);
    m_serial->enqueueData(cmd);
}

// 修复handleConnectionTimeout方法实现
void DriverWidget::handleConnectionTimeout()
{
    if (m_connectionPending) {
        m_connectionPending = false;
        
        // 断开串口连接
        if (m_serial->isConnected()) {
            m_serial->disconnectPort();
        }
        
        // 发送连接失败信号
        emit serialError("连接超时，未收到设备响应");
    }
}

// ... 实现其他槽函数 
