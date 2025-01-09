#include "serialutil.h"

SerialUtil::SerialUtil(QWidget *parent)
    : QWidget(parent)
    , serialPort(new QSerialPort(this))
{
    connect(serialPort,&QSerialPort::readyRead,this,&SerialUtil::readData);
    connect(serialPort,&QSerialPort::errorOccurred,this,&SerialUtil::onSerialPortError);
}

SerialUtil::~SerialUtil()
{
    if(serialPort->isOpen()){
        serialPort->close();
    }
    delete serialPort;
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

    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setParity(parity);
    serialPort->setStopBits(stopBits);
    serialPort->setFlowControl(flowControl);

    if(serialPort->open(QIODevice::ReadWrite)){
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
    if(serialPort->isOpen()){
        serialPort->close();
        qDebug() << "Disconnected from port";
    }else {
        qDebug() << "No port is currently open";
    }
}

// 发送数据
int SerialUtil::sendData(const QByteArray &data)
{
    if(serialPort->isOpen()){
//        qDebug() << "Send data is : " << data.toHex();
        return serialPort->write(data);
    }else {
        qDebug() << "Serial port is not open";
        return 0;
    }
}

// 返回串口名称
QString SerialUtil::getPortName()
{
    return serialPort->portName();
}

// 读取数据
void SerialUtil::readData()
{
    QByteArray data = serialPort->readAll();
    emit dataReceived(data);
}

// 检查是否连接
bool SerialUtil::isConnected() const
{
    return serialPort->isOpen();
}

// 获取当前连接串口信息
QString SerialUtil::currentPortName() const
{
    if(serialPort->isOpen()){
        return serialPort->portName();
    }else{
        return QString();
    }
}

// 获取当前连接串口波特率
qint32 SerialUtil::currentBaudRate() const
{
    if(serialPort->isOpen()){
        return serialPort->baudRate();
    }else{
        return -1;
    }
}

// 处理串口错误
void SerialUtil::onSerialPortError(QSerialPort::SerialPortError error){
    if(error == QSerialPort::ResourceError){
        // 串口断开或不可用
        qDebug() << "Serial port error: ResourceError (Disconnected)";
        emit portDisconnected(serialPort->portName());
        serialPort->close();    // 关闭串口
    }
}

bool SerialUtil::write(const QByteArray &data)
{
    if (!serialPort || !serialPort->isOpen()) return false;
    return serialPort->write(data) != -1;
}

QByteArray SerialUtil::readAll()
{
    if (!serialPort || !serialPort->isOpen()) return QByteArray();
    return serialPort->readAll();
}

bool SerialUtil::waitForResponse(int msecs)
{
    if (!serialPort || !serialPort->isOpen()) return false;
    return serialPort->waitForReadyRead(msecs);
}
