#include "cl200awidget.h"
#include <QVBoxLayout>
#include <QDebug>

CL200AWidget::CL200AWidget(QWidget *parent)
    : MeterBase(parent)
{
    // 创建串口对象
    m_serial = new SerialUtil(this);
    
    // 创建协议对象
    m_protocol = new CL_TwoZeroZeroACOM(this);
    
    // 创建状态更新定时器
    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(100);  // 100ms更新一次
    
    initUI();
    initConnections();
}

void CL200AWidget::initConnections()
{
    // 连接串口信号
    connect(m_serial, &SerialUtil::dataReceived,
            this, &CL200AWidget::handleSerialData);
    connect(m_serial, &SerialUtil::portDisconnected,
            this, [this]() {
                emit serialDisconnected();
                stopMeasurement();
            });
            
    // 连接定时器
    connect(m_statusTimer, &QTimer::timeout, this, [this]() {
        if (m_serial && m_serial->isConnected()) {
            // 发送查询命令
            quint8 header = 0x00;
            char CFable = '2';   // 禁用
            char calibrationMode = '0'; // 0规范，1多重
            QByteArray cmd = m_protocol->readMeasure("02",header,CFable,calibrationMode);
            m_serial->enqueueData(cmd);
        }
    });
}

void CL200AWidget::handleSerialData(const QByteArray &data)
{
    QByteArray parsedData;
//    if (m_protocol->parseResponse(data, parsedData)) {
        CL_TwoZeroZeroACOM::MeasurementData measurement;
//        if (m_protocol->parseMeasurementData(parsedData, measurement)) {
//            // 发送测量数据更新信号
//            emit measurementUpdated(
//                measurement.data1,  // 照度
//                measurement.data2,  // 色温
//                measurement.data3,  // 色度
//                0.0f,              // 预留G值
//                0.0f               // 预留B值
//            );
//        }
//    }
}

void CL200AWidget::connectToPort(const QString &portName)
{
    if (m_serial->connectToPort(portName, 9600)) {
        // 连接成功后设置PC模式
        setPCMode();
        startMeasurement();
        emit serialConnected(portName);
    }
}

CL200AWidget::~CL200AWidget()
{
    if (m_protocol) {
        delete m_protocol;
        m_protocol = nullptr;
    }
    
    if (m_statusTimer) {
        m_statusTimer->stop();
        delete m_statusTimer;
        m_statusTimer = nullptr;
    }
}

void CL200AWidget::disconnectPort()
{
    if (m_serial) {
        stopMeasurement();
        m_serial->disconnectPort();
        emit serialDisconnected();
    }
}

void CL200AWidget::setHold(bool hold)
{
    if (m_serial && m_serial->isConnected()) {
        QByteArray cmd = m_protocol->setHoldState55();
        m_serial->enqueueData(cmd);
    }
}

void CL200AWidget::setBacklight(bool on)
{
//    if (m_serial && m_serial->isConnected()) {
//        QByteArray cmd = m_protocol->makeBacklightCommand(on);
//        m_serial->enqueueData(cmd);
//    }
}

void CL200AWidget::setRange(int range)
{
//    if (m_serial && m_serial->isConnected()) {
//        QByteArray cmd = m_protocol->makeRangeCommand(range);
//        m_serial->enqueueData(cmd);
//    }
}

void CL200AWidget::setPCMode()
{
//    if (m_serial && m_serial->isConnected()) {
//        QByteArray cmd = m_protocol->makePCConnectCommand();
//        m_serial->enqueueData(cmd);
//    }
}

void CL200AWidget::setEXTMode(bool on)
{
//    if (m_serial && m_serial->isConnected()) {
//        QByteArray cmd = m_protocol->makeEXTCommand(on);
//        m_serial->enqueueData(cmd);
//    }
}

void CL200AWidget::startMeasurement()
{
    m_statusTimer->start();
}

void CL200AWidget::stopMeasurement()
{
    m_statusTimer->stop();
}

void CL200AWidget::handleSerialError(QSerialPort::SerialPortError error)
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
            break;
        default:
            errorMsg = "未知错误";
            break;
    }
    emit serialError(errorMsg);
}

bool CL200AWidget::isConnected() const
{
    return m_serial && m_serial->isConnected();
}

void CL200AWidget::initUI()
{
    // 创建主布局
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

// ... 其他方法实现 
