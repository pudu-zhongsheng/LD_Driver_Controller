#include "eleload_itplus.h"
#include <QDebug>

EleLoad_ITPlus::EleLoad_ITPlus(uint8_t loadAddress, QObject *parent)
    : Protocol(parent)
    , m_loadAddress(loadAddress)
{
}

QByteArray EleLoad_ITPlus::makeReadCommand(int address, int count)
{
    Q_UNUSED(address)
    Q_UNUSED(count)
    return createGetMessage(0x5F); // 默认读取输入状态
}

QByteArray EleLoad_ITPlus::makeWriteCommand(int address, const QByteArray &data)
{
    Q_UNUSED(address)
    Q_UNUSED(data)
    return QByteArray(); // 具体写入命令通过其他专用函数实现
}

bool EleLoad_ITPlus::parseResponse(const QByteArray &response, QByteArray &data)
{
    if (!validateResponse(response)) {
        emit error("响应数据校验失败");
        return false;
    }
    data = response;
    return true;
}

// 计算校验和
uint8_t EleLoad_ITPlus::calculateChecksum(const QByteArray &data)
{
    uint8_t checksum = 0;
    for(int i = 0; i < data.size()-1; ++i) {
        checksum += static_cast<uint8_t>(data[i]);
    }
    return checksum;
}

// 校验接收报文是否准确
bool EleLoad_ITPlus::validateResponse(const QByteArray &data)
{
    if (data.size() != CommandLength || 
        (data[0] & 0xFF) != SyncHeader || 
        calculateChecksum(data) != static_cast<uint8_t>(data[data.size() - 1])) {
        return false;
    }
    return true;
}

// 基本控制命令
QByteArray EleLoad_ITPlus::createControlModeCommand(uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x20;
    command[3] = mode;  // 0是面板控制，1是远程控制
    command[CommandLength-1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createLoadStateCommand(uint8_t state)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x21;
    command[3] = state; // 0为输出OFF，1为输入ON
    command[CommandLength-1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetMessage(uint8_t action)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = action;
    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 最大值设置命令
QByteArray EleLoad_ITPlus::createSetMaxVoltageCommand(float voltage)
{
    return createSetValueCommand(0x22, voltage, 1000.0f);
}

QByteArray EleLoad_ITPlus::createGetMaxVoltageCommand()
{
    return createGetMessage(0x23);
}

float EleLoad_ITPlus::analyseMaxVoltage(const QByteArray &data)
{
    return analyseValue(data, 1000.0f);
}

// 解析操作状态寄存器
EleLoad_ITPlus::HandleStateRegister EleLoad_ITPlus::parseHandleState(uint8_t handleStateRegister)
{
    HandleStateRegister state;
    state.noUse = handleStateRegister & 0x80;
    state.lot = handleStateRegister & 0x40;
    state.sense = handleStateRegister & 0x20;
    state.local = handleStateRegister & 0x10;
    state.out = handleStateRegister & 0x08;
    state.rem = handleStateRegister & 0x04;
    state.wtg = handleStateRegister & 0x02;
    state.cal = handleStateRegister & 0x01;
    return state;
}

// 输入参数解析
EleLoad_ITPlus::InputParams EleLoad_ITPlus::analyseInputParams(const QByteArray &data)
{
    InputParams params;
    if (!validateResponse(data)) {
        return params;
    }

    // 解析电压值（4字节，小端序）
    uint32_t voltage = static_cast<uint8_t>(data[3]) |
                      (static_cast<uint8_t>(data[4]) << 8) |
                      (static_cast<uint8_t>(data[5]) << 16) |
                      (static_cast<uint8_t>(data[6]) << 24);
    params.voltage = voltage / 1000.0f;

    // 解析电流值
    uint32_t current = static_cast<uint8_t>(data[7]) |
                      (static_cast<uint8_t>(data[8]) << 8) |
                      (static_cast<uint8_t>(data[9]) << 16) |
                      (static_cast<uint8_t>(data[10]) << 24);
    params.current = current / 10000.0f;

    // 解析功率值
    uint32_t power = static_cast<uint8_t>(data[11]) |
                    (static_cast<uint8_t>(data[12]) << 8) |
                    (static_cast<uint8_t>(data[13]) << 16) |
                    (static_cast<uint8_t>(data[14]) << 24);
    params.power = power / 1000.0f;

    // 解析状态寄存器
    params.handleStateRegister = parseHandleState(static_cast<uint8_t>(data[15]));
    
    // 解析查询状态寄存器
    params.selStateRegister = static_cast<uint8_t>(data[16]) |
                             (static_cast<uint8_t>(data[17]) << 8);

    // 解析其他参数
    params.radiatorTemperature = static_cast<uint8_t>(data[20]);
    params.workMode = static_cast<uint8_t>(data[21]);
    params.listStep = static_cast<uint8_t>(data[22]);
    params.listCycleIndex = static_cast<uint8_t>(data[23]) |
                           (static_cast<uint8_t>(data[24]) << 8);

    // 设置当前时间
    params.time = std::chrono::system_clock::now();

    return params;
}

// 产品信息解析
EleLoad_ITPlus::ProductMessage EleLoad_ITPlus::analyseProductMessage(const QByteArray &data)
{
    ProductMessage msg;
    if (!validateResponse(data)) {
        return msg;
    }

    // 解析产品型号（5字节ASCII）
    msg.productModel = QString(data.mid(3, 5));

    // 解析软件版本（2字节BCD码）
    uint8_t lowByte = static_cast<uint8_t>(data[8]);
    uint8_t highByte = static_cast<uint8_t>(data[9]);
    uint8_t lowDigit = lowByte & 0x0F;
    uint8_t highDigit = highByte >> 4;
    msg.softwareVersion = QString("%1.%2").arg(highDigit).arg(lowDigit);

    // 解析序列号（10字节ASCII）
    msg.productSerial = QString(data.mid(10, 10));

    return msg;
}

// 最大电流设置命令
QByteArray EleLoad_ITPlus::createSetMaxCurrentCommand(float current)
{
    return createSetValueCommand(0x24, current, 10000.0f);
}

QByteArray EleLoad_ITPlus::createGetMaxCurrentCommand()
{
    return createGetMessage(0x25);
}

float EleLoad_ITPlus::analyseMaxCurrent(const QByteArray &data)
{
    return analyseValue(data, 10000.0f);
}

// 最大功率设置命令
QByteArray EleLoad_ITPlus::createSetMaxPowerCommand(float power)
{
    return createSetValueCommand(0x26, power, 1000.0f);
}

QByteArray EleLoad_ITPlus::createGetMaxPowerCommand()
{
    return createGetMessage(0x27);
}

float EleLoad_ITPlus::analyseMaxPower(const QByteArray &data)
{
    return analyseValue(data, 1000.0f);
}

// 动态参数命令
QByteArray EleLoad_ITPlus::createSetDynamicCurrentParamsCommand(
    float currentA, float timeA, float currentB, float timeB, uint8_t mode)
{
    return createDynamicParamsCommand(0x32, currentA, timeA, currentB, timeB, mode, 10000.0f);
}

QByteArray EleLoad_ITPlus::createGetDynamicCurrentParamsCommand()
{
    return createGetMessage(0x33);
}

EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicCurrentParams(const QByteArray &data)
{
    return analyseDynamicParams(data, 10000.0f);
}

// 触发相关命令
QByteArray EleLoad_ITPlus::createBusTriggerSignal()
{
    return createGetMessage(0x5A);
}

QByteArray EleLoad_ITPlus::createNewTriggerSignal()
{
    return createGetMessage(0x9D);
}

// 工作模式相关命令
QByteArray EleLoad_ITPlus::createSetWorkModel(uint8_t model)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x5D;
    command[3] = model;
    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetWorkModel()
{
    return createGetMessage(0x5E);
}

uint8_t EleLoad_ITPlus::analyseWorkModel(const QByteArray &data)
{
    if (!validateResponse(data)) {
        return 0xFF;
    }
    return static_cast<uint8_t>(data[3]);
}

// 键盘模拟命令
QByteArray EleLoad_ITPlus::createAnalogKeyboardPress(uint8_t keyboard)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x98;
    command[3] = keyboard;
    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetInputCommand()
{
    return createCommand(0x5F);
}

QByteArray EleLoad_ITPlus::createGetProductMessage()
{
    return createCommand(0x6A);
}

QByteArray EleLoad_ITPlus::createSetConstantPowerCommand(float power)
{
    return createSetValueCommand(0x24, power, 1000.0f);
}

// 定值设置命令
QByteArray EleLoad_ITPlus::createSetConstantCurrentCommand(float current)
{
    uint32_t sendCurrent = static_cast<uint32_t>(current * 10000); // 0.1mA units
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x2A;

    command[3] = static_cast<uint8_t>(sendCurrent & 0xFF);
    command[4] = static_cast<uint8_t>((sendCurrent >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendCurrent >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendCurrent >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetConstantCurrentCommand()
{
    return createGetMessage(0x2B);
}

float EleLoad_ITPlus::analyseConstantCurrent(const QByteArray &data)
{
    return analyseValue(data, 10000.0f);
}

QByteArray EleLoad_ITPlus::createSetConstantVoltageCommand(float voltage)
{
    uint32_t sendVoltage = static_cast<uint32_t>(voltage * 1000); // 1mV units
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x2C;

    command[3] = static_cast<uint8_t>(sendVoltage & 0xFF);
    command[4] = static_cast<uint8_t>((sendVoltage >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendVoltage >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendVoltage >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetConstantVoltageCommand()
{
    return createGetMessage(0x2D);
}

float EleLoad_ITPlus::analyseConstantVoltage(const QByteArray &data)
{
    return analyseValue(data, 1000.0f);
}

QByteArray EleLoad_ITPlus::createSetConstantResistanceCommand(float resistance)
{
    uint32_t sendResistance = static_cast<uint32_t>(resistance * 1000); // 1mΩ units
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x30;

    command[3] = static_cast<uint8_t>(sendResistance & 0xFF);
    command[4] = static_cast<uint8_t>((sendResistance >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendResistance >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendResistance >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetConstantResistanceCommand()
{
    return createGetMessage(0x31);
}

float EleLoad_ITPlus::analyseConstantResistance(const QByteArray &data)
{
    return analyseValue(data, 1000.0f);
}

// 动态电压参数命令
QByteArray EleLoad_ITPlus::createSetDynamicVoltageParamsCommand(
    float voltageA, float timeA, float voltageB, float timeB, uint8_t mode)
{
    return createDynamicParamsCommand(0x34, voltageA, timeA, voltageB, timeB, mode, 1000.0f);
}

QByteArray EleLoad_ITPlus::createGetDynamicVoltageParamsCommand()
{
    return createGetMessage(0x35);
}

EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicVoltageParams(const QByteArray &data)
{
    return analyseDynamicParams(data, 1000.0f);
}

// 动态功率参数命令
QByteArray EleLoad_ITPlus::createSetDynamicPowerParamsCommand(
    float powerA, float timeA, float powerB, float timeB, uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x36;

    // 设置功率A值（4字节）
    uint32_t sendPowerA = static_cast<uint32_t>(powerA * 1000);
    command[3] = static_cast<uint8_t>(sendPowerA & 0xFF);
    command[4] = static_cast<uint8_t>((sendPowerA >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendPowerA >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendPowerA >> 24) & 0xFF);

    // 设置时间A值（2字节）
    uint16_t sendTimeA = static_cast<uint16_t>(timeA * 10000);
    command[7] = static_cast<uint8_t>(sendTimeA & 0xFF);
    command[8] = static_cast<uint8_t>((sendTimeA >> 8) & 0xFF);

    // 设置功率B值（4字节）
    uint32_t sendPowerB = static_cast<uint32_t>(powerB * 1000);
    command[9] = static_cast<uint8_t>(sendPowerB & 0xFF);
    command[10] = static_cast<uint8_t>((sendPowerB >> 8) & 0xFF);
    command[11] = static_cast<uint8_t>((sendPowerB >> 16) & 0xFF);
    command[12] = static_cast<uint8_t>((sendPowerB >> 24) & 0xFF);

    // 设置时间B值（2字节）
    uint16_t sendTimeB = static_cast<uint16_t>(timeB * 10000);
    command[13] = static_cast<uint8_t>(sendTimeB & 0xFF);
    command[14] = static_cast<uint8_t>((sendTimeB >> 8) & 0xFF);

    command[15] = mode;

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetDynamicPowerParamsCommand()
{
    return createGetMessage(0x37);
}

EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicPowerParams(const QByteArray &data)
{
    return analyseDynamicParams(data, 1000.0f);
}

// 动态电阻参数命令
QByteArray EleLoad_ITPlus::createSetDynamicResistanceParamsCommand(
    float resistanceA, float timeA, float resistanceB, float timeB, uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x38;

    // 设置电阻A值（4字节）
    uint32_t sendResistanceA = static_cast<uint32_t>(resistanceA * 1000);
    command[3] = static_cast<uint8_t>(sendResistanceA & 0xFF);
    command[4] = static_cast<uint8_t>((sendResistanceA >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendResistanceA >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendResistanceA >> 24) & 0xFF);

    // 设置时间A值（2字节）
    uint16_t sendTimeA = static_cast<uint16_t>(timeA * 10000);
    command[7] = static_cast<uint8_t>(sendTimeA & 0xFF);
    command[8] = static_cast<uint8_t>((sendTimeA >> 8) & 0xFF);

    // 设置电阻B值（4字节）
    uint32_t sendResistanceB = static_cast<uint32_t>(resistanceB * 1000);
    command[9] = static_cast<uint8_t>(sendResistanceB & 0xFF);
    command[10] = static_cast<uint8_t>((sendResistanceB >> 8) & 0xFF);
    command[11] = static_cast<uint8_t>((sendResistanceB >> 16) & 0xFF);
    command[12] = static_cast<uint8_t>((sendResistanceB >> 24) & 0xFF);

    // 设置时间B值（2字节）
    uint16_t sendTimeB = static_cast<uint16_t>(timeB * 10000);
    command[13] = static_cast<uint8_t>(sendTimeB & 0xFF);
    command[14] = static_cast<uint8_t>((sendTimeB >> 8) & 0xFF);

    command[15] = mode;

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

QByteArray EleLoad_ITPlus::createGetDynamicResistanceParamsCommand()
{
    return createGetMessage(0x39);
}

EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicResistanceParams(const QByteArray &data)
{
    return analyseDynamicParams(data, 1000.0f);
}

// 创建通用命令
QByteArray EleLoad_ITPlus::createCommand(uint8_t cmd, const QByteArray &data)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = cmd;
    
    if (!data.isEmpty()) {
        command.replace(3, data.size(), data);
    }
    
    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 值转换模板函数
template<typename T>
uint32_t EleLoad_ITPlus::valueToRaw(T value, float scale)
{
    return static_cast<uint32_t>(value * scale);
}

template<typename T>
T EleLoad_ITPlus::rawToValue(const QByteArray &data, int offset, float scale)
{
    uint32_t raw = static_cast<uint8_t>(data[offset]) |
                   (static_cast<uint8_t>(data[offset + 1]) << 8) |
                   (static_cast<uint8_t>(data[offset + 2]) << 16) |
                   (static_cast<uint8_t>(data[offset + 3]) << 24);
    return static_cast<T>(raw) / scale;
}

// 通用的设置值命令
QByteArray EleLoad_ITPlus::createSetValueCommand(uint8_t cmd, float value, float scale)
{
    uint32_t rawValue = valueToRaw(value, scale);
    QByteArray data(4, 0x00);
    data[0] = static_cast<char>(rawValue & 0xFF);
    data[1] = static_cast<char>((rawValue >> 8) & 0xFF);
    data[2] = static_cast<char>((rawValue >> 16) & 0xFF);
    data[3] = static_cast<char>((rawValue >> 24) & 0xFF);
    return createCommand(cmd, data);
}

// 通用的动态参数命令
QByteArray EleLoad_ITPlus::createDynamicParamsCommand(uint8_t cmd, float valueA, float timeA,
                                                     float valueB, float timeB, uint8_t mode,
                                                     float scale)
{
    QByteArray data(13, 0x00);
    
    // 设置值A
    uint32_t rawValueA = valueToRaw(valueA, scale);
    data[0] = static_cast<char>(rawValueA & 0xFF);
    data[1] = static_cast<char>((rawValueA >> 8) & 0xFF);
    data[2] = static_cast<char>((rawValueA >> 16) & 0xFF);
    data[3] = static_cast<char>((rawValueA >> 24) & 0xFF);
    
    // 设置时间A
    uint16_t rawTimeA = valueToRaw(timeA, 10000.0f);
    data[4] = static_cast<char>(rawTimeA & 0xFF);
    data[5] = static_cast<char>((rawTimeA >> 8) & 0xFF);
    
    // 设置值B
    uint32_t rawValueB = valueToRaw(valueB, scale);
    data[6] = static_cast<char>(rawValueB & 0xFF);
    data[7] = static_cast<char>((rawValueB >> 8) & 0xFF);
    data[8] = static_cast<char>((rawValueB >> 16) & 0xFF);
    data[9] = static_cast<char>((rawValueB >> 24) & 0xFF);
    
    // 设置时间B
    uint16_t rawTimeB = valueToRaw(timeB, 10000.0f);
    data[10] = static_cast<char>(rawTimeB & 0xFF);
    data[11] = static_cast<char>((rawTimeB >> 8) & 0xFF);
    
    // 设置模式
    data[12] = mode;
    
    return createCommand(cmd, data);
}

// 通用的动态参数解析函数
EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicParams(const QByteArray &data, float scale)
{
    DynamicParams params{};
    if (!validateResponse(data)) {
        return params;
    }

    // 解析值A
    params.valueA = rawToValue<float>(data, 3, scale);
    
    // 解析时间A
    params.timeA = rawToValue<float>(data, 7, 10000.0f);
    
    // 解析值B
    params.valueB = rawToValue<float>(data, 9, scale);
    
    // 解析时间B
    params.timeB = rawToValue<float>(data, 13, 10000.0f);
    
    // 解析模式
    params.mode = data[15];

    return params;
}

// 通用的值解析函数
float EleLoad_ITPlus::analyseValue(const QByteArray &data, float scale)
{
    if (!validateResponse(data)) {
        return -1.0f;
    }
    return rawToValue<float>(data, 3, scale);
}



