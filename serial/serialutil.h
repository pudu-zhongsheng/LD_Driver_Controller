#ifndef ELECTRONICLOADSERIAL_H
#define ELECTRONICLOADSERIAL_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QQueue>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

class SerialUtil : public QWidget
{
    Q_OBJECT
public:
    explicit SerialUtil(QWidget *parent = nullptr);
    ~SerialUtil();

    static QList<QSerialPortInfo> getAvailablePorts(); // 搜索可用串口
    bool connectToPort(const QString &portName,
                       qint32 baudRate = 256000,
                       QSerialPort::DataBits dataBits = QSerialPort::Data8,
                       QSerialPort::Parity parity = QSerialPort::NoParity,
                       QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                       QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl);   // 连接到指定串口
    void disconnectPort();  // 断开当前连接串口
    bool isConnected() const;   // 检查是否连接
    QString currentPortName() const;    // 获取当前连接的串口信息
    qint32 currentBaudRate() const; // 获取当前连接的串口波特率
    QString getPortName();  // 返回串口名称
    void enqueueData(const QByteArray &data);   // 添加数据到队列
    void endSending();      // 停止定时发送

public slots:
    void readData();    // 读取数据
    void processQueue();    // 定时器触发，处理队列中的数据
    void onSerialPortError(QSerialPort::SerialPortError error); // 串口错误发生

signals:
    void dataReceived(const QByteArray &data);  // 数据接收
    void portDisconnected(const QString &portName); // 串口断开信号

private:
    QSerialPort *m_serial;
    QQueue<QByteArray> dataQueue;    // 数据缓存队列
    QTimer sendTimer;                // 定时器
    int sendData(const QByteArray &data);   // 发送数据
};

#endif // ELECTRONICLOADSERIAL_H
