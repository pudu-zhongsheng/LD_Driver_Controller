#include "cl200awidget.h"
#include "../../../util/logger.h"
#include "util/errorhandler.h"
#include <QMessageBox>
#include <QDebug>
#include <math.h>

CL200AWidget::CL200AWidget(QWidget *parent)
    : MeterBase(parent)
    , m_serialPort(new QSerialPort(this))
    , m_protocol(new CL_TwoZeroZeroACOM(this))
    , m_measurementTimer(new QTimer(this))
    , m_commandTimeoutTimer(new QTimer(this))
    , m_commState(CommState::Idle)
    , m_isMeasuring(false)
    , m_isInitialized(false)
    , m_currentMeasurementType(0)
    , m_illuminance(0.0f)
    , m_colorTemp(0.0f)
    , m_r(0.0f)
    , m_g(0.0f)
    , m_b(0.0f)
{
    // Connecting serial signal
    connect(m_serialPort, &QSerialPort::readyRead, this, &CL200AWidget::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &CL200AWidget::handleError);
    
    // Set measuring timer to control measuring frequency
    m_measurementTimer->setInterval(1000); // The measurement is performed every 500ms by default
    connect(m_measurementTimer, &QTimer::timeout, this, &CL200AWidget::onMeasurementTimerTimeout);
    
    // Set the timeout timer for sending commands
    m_commandTimeoutTimer->setSingleShot(true);
    m_commandTimeoutTimer->setInterval(2000); // 2 second timeout
    connect(m_commandTimeoutTimer, &QTimer::timeout, this, [this]() {
        LOG_ERROR("The illuminometer command timed out");
        m_commState = CommState::Error;
        emit serialError("The command timed out, please try again");

        connectToPort(m_portname);  // reconnect serial port
    });
}

CL200AWidget::~CL200AWidget()
{
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool CL200AWidget::connectToPort(const QString &portName)
{
    m_portname = portName;
    // Make sure the previous connection is closed
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    
    // Set the serial port name and parameters
    m_serialPort->setPortName(portName);
    setupSerialParameters();
    
    // Try to open the serial port
    if(!m_serialPort->open(QIODevice::ReadWrite)) {
        LOG_ERROR("Unable to open the illuminometer serial port: " + portName);
        emit serialError("Unable to open the serial port: " + portName);
        return false;
    }

    LOG_INFO("The illuminometer serial port is connected: " + portName);
    m_isInitialized = false;
    m_commState = CommState::Idle;
    
    // Performs the initialization communication sequence
    setupInitialCommunication();
    
    return true;
}

void CL200AWidget::disconnectPort()
{
    // Stop measurement
    stopMeasurement();
    
    // Close serial port
    if(m_serialPort->isOpen()) {
        m_serialPort->close();
        LOG_INFO("The illuminometer serial port is disconnected");
        emit serialDisconnected();
    }
    
    m_isInitialized = false;
    m_commState = CommState::Idle;
}

bool CL200AWidget::isConnected() const
{
    return m_serialPort->isOpen() && m_isInitialized;
}

void CL200AWidget::startMeasurement()
{
    if(!isConnected()) {
        LOG_ERROR("Cannot start measurement: the illuminometer serial port is not connected or initialized");
        emit serialError("Illuminometer is not connected, please connect first");
        return;
    }
    
    m_isMeasuring = true;
    m_measurementTimer->start();
    LOG_INFO("The illuminometer starts measuring");
}

void CL200AWidget::stopMeasurement()
{
    m_isMeasuring = false;
    m_measurementTimer->stop();
    LOG_INFO("The illuminometer stops measuring");
}

void CL200AWidget::setupSerialParameters()
{
    // Setting serial port parameters
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data7);
    m_serialPort->setParity(QSerialPort::EvenParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
}

void CL200AWidget::setupInitialCommunication()
{
    // Initialize the communication status
    m_commState = CommState::WaitingForPCModeResponse;
    
    // 1. Set the PC mode to illuminometer
    QByteArray pcModeCommand = m_protocol->setPCConnect54();
    if(!sendCommand(pcModeCommand)) {
        LOG_ERROR("Failed to set the PC mode");
        return;
    }
    
    // Start the timeout timer
    m_commandTimeoutTimer->start();
}

void CL200AWidget::handleReadyRead()
{
    // Received all available data
    m_receivedData.append(m_serialPort->readAll());
    LOG_INFO("received illuminometer data: " + m_receivedData.toHex());
    
    // Check to see if the full message is received(ends with CR+LF)
    if(m_receivedData.contains("\r\n")) {
        // Stop the timeout timer
        m_commandTimeoutTimer->stop();
        
        // Parse the received data
        parseReceivedData(m_receivedData);
        
        // Clear the receive buffer, ready to receive the next message.
        m_receivedData.clear();
    }
}

void CL200AWidget::handleError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::NoError) {
        return;
    }
    
    LOG_ERROR("The error about illuminometer serial port: " + QString::number(error));
    
    // Processing error
    switch(error) {
        case QSerialPort::DeviceNotFoundError:
        case QSerialPort::PermissionError:
        case QSerialPort::OpenError:
            emit serialError("Unable to open the serial port");
            break;
        case QSerialPort::ReadError:
            emit serialError("Read error");
            break;
        case QSerialPort::WriteError:
            emit serialError("Write error");
            break;
        case QSerialPort::ResourceError:
            // Device removed
            disconnectPort();
            emit serialError("Device connecting is disconnected");
            break;
        default:
            emit serialError("Serial port error: " + QString::number(error));
            break;
    }
    
    m_commState = CommState::Error;
}

void CL200AWidget::parseReceivedData(const QByteArray &data)
{
    // Processing data based on communication status
    switch(m_commState) {
        case CommState::WaitingForPCModeResponse: {
            // Parse the command response to set PC mode
            auto result = m_protocol->handleReceivedSettingResult(data);
            if(result.registerCode == "54" && result.err == ' ') {
                LOG_INFO("Illuminometer successed to set PC mode");
                
                // Wait 500 milliseconds to send the hold status command
                QTimer::singleShot(500, this, [this]() {
                    QByteArray holdCommand = m_protocol->setHoldState55();
                    sendCommand(holdCommand);
                    
                    // Wait 500 milliseconds to send the set EXT mode command
                    QTimer::singleShot(500, this, [this]() {
                        m_commState = CommState::WaitingForEXTModeResponse;
                        QByteArray extCommand = m_protocol->setEXT40(0x00);
                        sendCommand(extCommand);
                        m_commandTimeoutTimer->start();
                    });
                });
            } else {
                LOG_ERROR("Fail to set illuminometer to PC mode");
                emit serialError("Fail to initialize illuminometer");
                m_commState = CommState::Error;
            }
            break;
        }
        
        case CommState::WaitingForEXTModeResponse: {
            // Parse the response command to set EXT mode
            auto result = m_protocol->handleReceivedSettingResult(data);
            if(result.registerCode == "40" && result.err == ' ') {
                LOG_INFO("Setting the EXT mode of the illuminometer succeeded");
                
                // The inialization is complete, and the measurement can begin after 175 milliseconds
                QTimer::singleShot(175, this, [this]() {
                    m_isInitialized = true;
                    m_commState = CommState::Idle;
                    emit serialConnected(m_serialPort->portName());
                });
            } else {
                LOG_ERROR("Fail to set illuminometer mode to EXT");
                emit serialError("Fail to initialize the illuminometer");
                m_commState = CommState::Error;
            }
            break;
        }
        
        case CommState::WaitingForMeasurementData: {
            // Here we need to parse the data according to the current measurement type
            // For different measurement commands, the parsing methods are different
            CL_TwoZeroZeroACOM::MeasurementData measureData;
            measureData = m_protocol->handleReceivedMeasurementData(data);
            // The analytical method is determined according to the current measurement type
            switch(m_currentMeasurementType) {
                case 0: // command 01 - X,Y,Z
                    if(measureData.registerCode == "01") {
                        // The X,Y,Z values are stored in data1,data2,data3 respectively
                        m_r = measureData.data1;
                        m_g = measureData.data2;
                        m_b = measureData.data3;

                        // Send a signal to update the measurement data
                        emit measurementUpdated(m_illuminance, m_colorTemp, m_r, m_g, m_b);
                    }
                    break;

                // Can add data parsing operations for other measurement commands
                case 1: // command 02 - EV,X,Y
                    if(measureData.registerCode == "02") {
                        // Measure the illuminance data
                        m_illuminance = measureData.data1;
                        emit measurementUpdated(m_illuminance, m_colorTemp, m_r, m_g, m_b);
                    }
                case 2: // command 03 - EV,U',V'
                case 3: // command 08 - EV,TCP,DUV
                case 4: // command 15 - EV,DW,P
                case 5: // command 45 - X2,Y,Z
                    // The implementation is omitted here and can be added as needed
                    break;
            }
            
            m_commState = CommState::Idle;
            break;
        }
        
        case CommState::Idle:
        case CommState::Error:
            // Ignore data received in idle state or error state
            break;
    }
}

bool CL200AWidget::sendCommand(const QByteArray &command)
{
    if(!m_serialPort->isOpen()) {
        LOG_ERROR("Cannot send command: serial port is not open");
        return false;
    }
    
    // Write command
    qint64 written = m_serialPort->write(command);
    if(written != command.size()) {
        LOG_ERROR("The command was sent incompletely");
        return false;
    }
    
    // Make sure the data is sent
    if(!m_serialPort->waitForBytesWritten(1000)) {
        LOG_ERROR("Waiting for the command to be sent times out");
        return false;
    }
    
    return true;
}

void CL200AWidget::onMeasurementTimerTimeout()
{
    // 只有在空闲状态下才发送新的测量命令
    if (m_commState != CommState::Idle || !m_isInitialized) {
        return;
    }
    
    // 先发送 EXT 测试命令
    QByteArray extTestCommand = m_protocol->takeEXT40();
    if (!sendCommand(extTestCommand)) {
        LOG_ERROR("发送 EXT 测试命令失败");
        return;
    }
    
    // 等待 750ms 后读取测量数据
    QTimer::singleShot(750, this, [this]() {
        // 只有在空闲状态下才继续
        if (m_commState != CommState::Idle) {
            return;
        }
        
        m_commState = CommState::WaitingForMeasurementData;
        
        // 根据当前测量类型选择命令
        QString cmdCode;
        switch (m_currentMeasurementType) {
            case 0: cmdCode = "01"; break; // X,Y,Z
            case 1: cmdCode = "02"; break; // EV,X,Y
            case 2: cmdCode = "03"; break; // EV,U',V'
            case 3: cmdCode = "08"; break; // EV,TCP,DUV
            case 4: cmdCode = "15"; break; // EV,DW,P
            case 5: cmdCode = "45"; break; // X2,Y,Z
            default: cmdCode = "01"; break;
        }
        
        QByteArray readCommand = m_protocol->readMeasure(cmdCode, 0x00, '2', '0');
        if (!sendCommand(readCommand)) {
            LOG_ERROR("发送读取测量数据命令失败");
            m_commState = CommState::Idle;
            return;
        }
        
        // 启动超时定时器
        m_commandTimeoutTimer->start();
    });
}

void CL200AWidget::setMeasurementType(int type)
{
    // 确保类型在有效范围内
    if (type >= 0 && type <= 5) {
        m_currentMeasurementType = type;
        LOG_INFO("照度计测量类型设置为: " + QString::number(type));
    } else {
        LOG_ERROR("无效的照度计测量类型: " + QString::number(type));
    }
}

// ... other methods realize
