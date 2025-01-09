#include "driver8ch.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDebug>

Driver8CH::Driver8CH(QWidget *parent) : DriverBase(parent)
{
    // 创建串口对象
    m_serial = new SerialUtil(this);
    
    // 创建协议对象
    m_protocol = new DriverProtocol(this);
    
    initUI();
    initConnections();
}

void Driver8CH::initUI()
{
    auto *mainHLayout = new QHBoxLayout(this);
    mainHLayout->setSpacing(20);
    auto *mainLayout1 = new QVBoxLayout();
    mainLayout1->setSpacing(20);
    auto *mainLayout2 = new QVBoxLayout();
    mainLayout2->setSpacing(20);
    auto *mainLayout3 = new QVBoxLayout();
    mainLayout3->setSpacing(20);
    // 1. 基本信息区域
    auto *infoGroup = new QGroupBox("基本信息", this);
    auto *infoLayout = new QVBoxLayout(infoGroup);
    m_infoWidget = new DriverInfoWidget(this);
    infoLayout->addWidget(m_infoWidget);
    mainLayout1->addWidget(infoGroup);

    // 2. 调控区域
    auto *controlGroup = new QGroupBox("调控区域", this);
    auto *controlLayout = new QVBoxLayout(controlGroup);
    m_controlWidget = new ControlWidget(this);
    m_controlWidget->setRegisterRange(8);  // 8通道
    controlLayout->addWidget(m_controlWidget);
    mainLayout1->addWidget(controlGroup);

    // 3. 扫描测试区域
    auto *scanGroup = new QGroupBox("扫描测试", this);
    auto *scanLayout = new QVBoxLayout(scanGroup);
    m_scanWidget = new ScanWidget(this);
    scanLayout->addWidget(m_scanWidget);
    mainLayout2->addWidget(scanGroup);

    // 4. 滑条控制区域
    auto *sliderGroup = new QGroupBox("滑条控制", this);
    auto *sliderLayout = new QVBoxLayout(sliderGroup);
    m_sliderWidget = new SliderWidget(8, this);  // 8通道
    sliderLayout->addWidget(m_sliderWidget);
    mainLayout3->addWidget(sliderGroup);

    // 5. 参数表区域
    auto *paramGroup = new QGroupBox("参数表", this);
    auto *paramLayout = new QVBoxLayout(paramGroup);
    m_paramTable = new ParamTableWidget(8, this);  // 8通道
    paramLayout->addWidget(m_paramTable);
    mainLayout2->addWidget(paramGroup);

    mainHLayout->addLayout(mainLayout1);
    mainHLayout->addLayout(mainLayout2);
    mainHLayout->addLayout(mainLayout3);

    // 加载样式表
    QFile file(":/styles/driver.qss");
    if (file.open(QFile::ReadOnly)) {
        setStyleSheet(file.readAll());
        file.close();
    }
}

void Driver8CH::initConnections()
{
    // 连接串口信号
    connect(m_serial, &SerialUtil::dataReceived,
            this, &Driver8CH::handleSerialData);
    connect(m_serial, &SerialUtil::portDisconnected,
            this, [this]() {
                emit serialDisconnected();
            });

    // 连接控制区域信号
    connect(m_controlWidget, &ControlWidget::controlChanged,
            this, &Driver8CH::onControlChanged);

    // 连接扫描测试信号
    connect(m_scanWidget, &ScanWidget::scanValueChanged,
            this, &Driver8CH::onScanValueChanged);

    // 连接滑条控制信号
    connect(m_sliderWidget, &SliderWidget::channelValueChanged,
            this, &Driver8CH::onSliderChannelChanged);
    connect(m_sliderWidget, &SliderWidget::allChannelsValueChanged,
            this, &Driver8CH::onSliderAllChanged);
}

void Driver8CH::onControlChanged()
{
    // 获取当前控制参数
    bool highLevel = m_controlWidget->isHighLevel();
    int startReg = m_controlWidget->getStartRegister();
    int regCount = m_controlWidget->getRegisterCount();
    int value1 = m_controlWidget->getChannelValue1();
    int value2 = m_controlWidget->getChannelValue2();
    int value3 = m_controlWidget->getChannelValue3();

    // 构造并发送命令
    QByteArray cmd = m_protocol->makeControlCommand(
        highLevel, startReg, regCount, value1, value2, value3);
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }

    // 更新参数表
    for (int i = startReg - 1; i < startReg - 1 + regCount; ++i) {
        m_paramTable->updateChannelValue(i, value1, value2, value3);
    }
}

void Driver8CH::onScanValueChanged(int value)
{
    // 根据扫描目标更新对应的通道值
    QString target = m_scanWidget->currentTarget();
    if (target == "通道值一") {
        m_controlWidget->setChannelValue1(value);
    } else if (target == "通道值二") {
        m_controlWidget->setChannelValue2(value);
    } else if (target == "通道值三") {
        m_controlWidget->setChannelValue3(value);
    }
}

void Driver8CH::onSliderChannelChanged(int channel, int value)
{
    // 构造并发送单通道命令
    QByteArray cmd = m_protocol->makeChannelCommand(
        channel + 1, value, m_controlWidget->isHighLevel());
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }

    // 更新参数表
    m_paramTable->updateChannelValue(channel, value, value, value);
}

void Driver8CH::onSliderAllChanged(int value)
{
    // 构造并发送全通道命令
    QByteArray cmd = m_protocol->makeAllChannelsCommand(
        value, m_controlWidget->isHighLevel());
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }

    // 更新参数表
    for (int i = 0; i < 8; ++i) {
        m_paramTable->updateChannelValue(i, value, value, value);
    }
}

void Driver8CH::handleSerialData(const QByteArray &data)
{
    QByteArray parsedData;
    if (m_protocol->parseResponse(data, parsedData)) {
        // 处理解析后的数据
        // TODO: 根据具体协议实现
    }
}

bool Driver8CH::isConnected() const
{
    return m_serial && m_serial->isConnected();
}

void Driver8CH::connectToPort(const QString &portName)
{
    if (m_serial->connectToPort(portName, 115200)) {
        emit serialConnected(portName);
    }
}

void Driver8CH::disconnectPort()
{
    if (m_serial) {
        m_serial->disconnectPort();
    }
}

QJsonObject Driver8CH::getSettings() const
{
    QJsonObject settings;
    
    // 保存基本信息
    settings["customer"] = m_infoWidget->getCustomer();
    settings["driverCode"] = m_infoWidget->getDriverCode();
    settings["testCode"] = m_infoWidget->getTestCode();
    
    // 保存控制区域设置
    settings["highLevel"] = m_controlWidget->isHighLevel();
    settings["startRegister"] = m_controlWidget->getStartRegister();
    settings["registerCount"] = m_controlWidget->getRegisterCount();
    settings["channelValue1"] = m_controlWidget->getChannelValue1();
    settings["channelValue2"] = m_controlWidget->getChannelValue2();
    settings["channelValue3"] = m_controlWidget->getChannelValue3();
    
    return settings;
}

void Driver8CH::applySettings(const QJsonObject &settings)
{
    // 恢复基本信息
    if (settings.contains("customer")) {
        m_infoWidget->setCustomer(settings["customer"].toString());
    }
    if (settings.contains("driverCode")) {
        m_infoWidget->setDriverCode(settings["driverCode"].toString());
    }
    if (settings.contains("testCode")) {
        m_infoWidget->setTestCode(settings["testCode"].toString());
    }
    
    // 恢复控制区域设置
    if (settings.contains("highLevel")) {
        m_controlWidget->setHighLevel(settings["highLevel"].toBool());
    }
    if (settings.contains("startRegister")) {
        m_controlWidget->setStartRegister(settings["startRegister"].toInt());
    }
    if (settings.contains("registerCount")) {
        m_controlWidget->setRegisterCount(settings["registerCount"].toInt());
    }
    if (settings.contains("channelValue1")) {
        m_controlWidget->setChannelValue1(settings["channelValue1"].toInt());
    }
    if (settings.contains("channelValue2")) {
        m_controlWidget->setChannelValue2(settings["channelValue2"].toInt());
    }
    if (settings.contains("channelValue3")) {
        m_controlWidget->setChannelValue3(settings["channelValue3"].toInt());
    }
}

void Driver8CH::sendChannelCommand(int channel, int value)
{
    QByteArray cmd = m_protocol->makeChannelCommand(
        channel, value, m_controlWidget->isHighLevel());
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }
}

void Driver8CH::sendAllChannelsCommand(int value)
{
    QByteArray cmd = m_protocol->makeAllChannelsCommand(
        value, m_controlWidget->isHighLevel());
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }
}

void Driver8CH::sendControlCommand(bool highLevel, int startReg, int regCount, 
                                 int value1, int value2, int value3)
{
    QByteArray cmd = m_protocol->makeControlCommand(
        highLevel, startReg, regCount, value1, value2, value3);
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }
}

void Driver8CH::updateParamTable()
{
    // 更新所有通道的参数显示
    for (int i = 0; i < 8; ++i) {
        int value1 = 0, value2 = 0, value3 = 0;
        // TODO: 从协议或缓存中获取实际值
        m_paramTable->updateChannelValue(i, value1, value2, value3);
    }
}

Driver8CH::~Driver8CH()
{
    if (m_protocol) {
        delete m_protocol;
        m_protocol = nullptr;
    }
}

void Driver8CH::handleSerialError(QSerialPort::SerialPortError error)
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

// ... 其他方法实现保持不变 
