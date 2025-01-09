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
            QByteArray cmd = m_protocol->makeQueryCommand();
            m_serial->sendData(cmd);
        }
    });
}

void CL200AWidget::handleSerialData(const QByteArray &data)
{
    QByteArray parsedData;
    if (m_protocol->parseResponse(data, parsedData)) {
        CL_TwoZeroZeroACOM::MeasurementData measurement;
        if (m_protocol->parseMeasurementData(parsedData, measurement)) {
            // 发送测量数据更新信号
            emit measurementUpdated(
                measurement.data1,  // 照度
                measurement.data2,  // 色温
                measurement.data3,  // 色度
                0.0f,              // 预留G值
                0.0f               // 预留B值
            );
        }
    }
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

// ... 其他方法实现 
