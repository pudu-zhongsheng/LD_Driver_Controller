#include "serialutil.h"

SerialUtil::SerialUtil(QWidget *parent)
    : QWidget(parent)
    , m_serial(new QSerialPort(this))
{
    connect(m_serial,&QSerialPort::readyRead,this,&SerialUtil::readData);
    connect(m_serial,&QSerialPort::errorOccurred,this,&SerialUtil::onSerialPortError);
}

SerialUtil::~SerialUtil()
{
    if(m_serial->isOpen()){
        m_serial->close();
    }
    delete m_serial;
}

// 搜索可用串口
QList<QSerialPortInfo> SerialUtil::getAvailablePorts()
{
    QList<QSerialPortInfo> availablePort = QSerialPortInfo::availablePorts();
    return availablePort;
}

// 连接到指定串口
bool SerialUtil::connectToPort(const QString &portName,
                               qint32 baudRate,
                               QSerialPort::DataBits dataBits,
                               QSerialPort::Parity parity,
                               QSerialPort::StopBits stopBits,
                               QSerialPort::FlowControl flowControl)
{
    qDebug() << "Connecting to port:" << portName
             << ", BaudRate:" << baudRate
             << ", DataBits:" << dataBits
             << ", Parity:" << parity
             << ", StopBits:" << stopBits
             << ", FlowControl:" << flowControl;

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(dataBits);
    m_serial->setParity(parity);
    m_serial->setStopBits(stopBits);
    m_serial->setFlowControl(flowControl);

    if(m_serial->open(QIODevice::ReadWrite)){
        qDebug() << "Connected to" << portName;
        return true;
    } else {
        qDebug() << "Failed to connect to" << portName;
        return false;
    }
}

/*  使用示例
SerialUtil loadSerial;

// 使用默认参数连接串口
if (loadSerial.connectToPort("COM1")) {
    qDebug() << "Successfully connected!";
} else {
    qDebug() << "Connection failed!";
}

// 使用自定义参数连接串口
if (loadSerial.connectToPort("COM1", 115200, QSerialPort::Data7, QSerialPort::OddParity, QSerialPort::TwoStop, QSerialPort::HardwareControl)) {
    qDebug() << "Successfully connected with custom settings!";
} else {
    qDebug() << "Connection failed!";
}
*/

// 断开当前连接串口
void SerialUtil::disconnectPort()
{
    if(m_serial->isOpen()){
        m_serial->close();
        qDebug() << "Disconnected from port";
    }else {
        qDebug() << "No port is currently open";
    }
}

// 发送数据
int SerialUtil::sendData(const QByteArray &data)
{
    if(m_serial->isOpen()){
       qDebug() << "Send data is : " << data.toHex();
        return m_serial->write(data);
    }else {
        qDebug() << "Serial port is not open";
        return 0;
    }
}

// 返回串口名称
QString SerialUtil::getPortName()
{
    return m_serial->portName();
}

// 读取数据
void SerialUtil::readData()
{
    QByteArray data = m_serial->readAll();
    emit dataReceived(data);
}

// 检查是否连接
bool SerialUtil::isConnected() const
{
    return m_serial->isOpen();
}

// 获取当前连接串口信息
QString SerialUtil::currentPortName() const
{
    if(m_serial->isOpen()){
        return m_serial->portName();
    }else{
        return QString();
    }
}

// 获取当前连接串口波特率
qint32 SerialUtil::currentBaudRate() const
{
    if(m_serial->isOpen()){
        return m_serial->baudRate();
    }else{
        return -1;
    }
}

// 处理串口错误
void SerialUtil::onSerialPortError(QSerialPort::SerialPortError error){
    if(error == QSerialPort::ResourceError){
        // 串口断开或不可用
        qDebug() << "Serial port error: ResourceError (Disconnected)";
        emit portDisconnected(m_serial->portName());
        m_serial->close();    // 关闭串口
    }
}

bool SerialUtil::write(const QByteArray &data)
{
    if (!m_serial || !m_serial->isOpen()) return false;
    QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
    qDebug() << "Write data is : " << data.toHex() << ", Time: " << time;
    return m_serial->write(data) != -1;
}

QByteArray SerialUtil::readAll()
{
    if (!m_serial || !m_serial->isOpen()) return QByteArray();
    return m_serial->readAll();
}

bool SerialUtil::waitForResponse(int msecs)
{
    if (!m_serial || !m_serial->isOpen()) return false;
    return m_serial->waitForReadyRead(msecs);
}
