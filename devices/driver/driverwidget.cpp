#include "driverwidget.h"

DriverWidget::DriverWidget(int channelCount, QWidget *parent)
    : QWidget(parent)
    , m_channelCount(channelCount)
    , m_scanTimer(new QTimer(this))
{
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

    // LED开关按钮
    connect(m_ledSwitch, &QPushButton::clicked, this, [this]() {
        m_ledStatus = !m_ledStatus;
        if (m_ledStatus) {
            m_ledSwitch->setText("关闭LED");
            m_ledSwitch->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; }");
        } else {
            m_ledSwitch->setText("打开LED");
            m_ledSwitch->setStyleSheet("QPushButton { background-color: #d9534f; color: white; }");
        }
    });

    // 扫描按钮连接
    connect(m_increaseBtn, &QPushButton::clicked, this, [this]() {
        startScan(true);
    });

    connect(m_decreaseBtn, &QPushButton::clicked, this, [this]() {
        startScan(false);
    });
}

// 析构函数实现
DriverWidget::~DriverWidget()
{
    if (m_scanTimer->isActive()) {
        m_scanTimer->stop();
    }
}

// 总控滑条值改变
void DriverWidget::onMasterValueChanged(int value)
{
    // 更新总控数值显示
    m_masterValueEdit->setValue(value);
    
    // 同步更新所有单控滑条
    for (int i = 0; i < m_channelSliders.size(); ++i) {
        m_channelSliders[i]->setValue(value);
        m_channelValueSpins[i]->setValue(value);
    }
    
    // 更新参数表
    updateParamTable();
    
    // 发送通道值变化信号
    QVector<int> values;
    for (int i = 0; i < m_channelCount; ++i) {
        values.append(value);
    }
    emit channelValuesChanged(values);
}

// 单控滑条值改变
void DriverWidget::onChannelValueChanged(int index, int value)
{
    // 更新对应的数值显示
    m_channelValueSpins[index]->setValue(value);
    
    // 更新参数表
    updateParamTable();
    
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

// ... 实现其他槽函数 
