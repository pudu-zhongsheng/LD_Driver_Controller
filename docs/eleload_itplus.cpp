#include "eleload_itplus.h"

EleLoad_ITPlus::EleLoad_ITPlus(uint8_t loadAddress, QWidget *parent)
    : QWidget(parent)
    , m_loadAddress(loadAddress)
{

}

EleLoad_ITPlus::~EleLoad_ITPlus(){}

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
//    qDebug() << "validateResponse接收到的报文为：" << data.toHex();

    bool a1 = data.size() != CommandLength;
    bool a2 = (data[0]&0xFF) != SyncHeader; // 把data[0]&上0xFF，避免因为首位1导致AA的值变成负数（170变成-86）
    bool a3 = calculateChecksum(data) != static_cast<uint8_t>(data[data.size() - 1]);
//    qDebug() << "a1:" << a1 << "a2:" << a2 << "a3:" << a3;
//    if (data.size() != CommandLength || data[0] != SyncHeader ||
//        calculateChecksum(data) != static_cast<uint8_t>(data[data.size() - 1])) {
    if(a1||a2||a3){
        return false;
    }
    return true;
}

// 发送读取数据请求
QByteArray EleLoad_ITPlus::createGetMessage(uint8_t action)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = action;  // Command byte for setting max voltage

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 设置负载控制模式
QByteArray EleLoad_ITPlus::createControlModeCommand(uint8_t mode)
{
    QByteArray command(CommandLength,0x00); // 创建固定的26字节数组，并填充0
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x20;
    command[3] = mode;  // 0是面板控制，1是远程控制
    command[CommandLength-1] = calculateChecksum(command);
    return command;
}

// 控制负载输入状态
QByteArray EleLoad_ITPlus::createLoadStateCommand(uint8_t state)
{
    QByteArray command(CommandLength,0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x21;
    command[3] = state; // 0为输出OFF，1为输入ON
    command[CommandLength-1] = calculateChecksum(command);
    return command;
}

// 设置负载最大输入电压值，1表示1mV
QByteArray EleLoad_ITPlus::createSetMaxVoltageCommand(float voltage) {
    uint32_t sendVoltage = static_cast<uint32_t>(1000*voltage);
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x22;  // Command byte for setting max voltage

    // Voltage value: low byte first
    command[3] = static_cast<uint8_t>(sendVoltage & 0xFF);
    command[4] = static_cast<uint8_t>((sendVoltage >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendVoltage >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendVoltage >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 获取负载最大输入电压值
QByteArray EleLoad_ITPlus::createGetMaxVoltageCommand()
{
    return createGetMessage(0x23);
}

// 解析报文获取最大输入电压值
float EleLoad_ITPlus::analyseMaxVoltage(const QByteArray &data)
{
    // 数据异常
    if(!validateResponse(data)){
        return -1.0f;
    }

    uint32_t voltage = 0;
    voltage |= static_cast<uint8_t>(data[3]);
    voltage |= static_cast<uint8_t>(data[4]) << 8;
    voltage |= static_cast<uint8_t>(data[5]) << 16;
    voltage |= static_cast<uint8_t>(data[6]) << 24;

    return static_cast<float>(voltage) / 1000.0f;
}

// 设置负载最大输入电流值，1表示0.1mA
QByteArray EleLoad_ITPlus::createSetMaxCurrentCommand(float current)
{
    uint32_t sendCurrent = static_cast<uint32_t>(current * 10000);  // 1表示0.1mA
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x24;  // Command byte for setting max current

    // Current value: low byte first
    command[3] = static_cast<uint8_t>(sendCurrent & 0xFF);
    command[4] = static_cast<uint8_t>((sendCurrent >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendCurrent >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendCurrent >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 获取负载最大输入电流值
QByteArray EleLoad_ITPlus::createGetMaxCurrentCommand()
{
    return createGetMessage(0x25);
}

// 解析报文获取最大输入电流值
float EleLoad_ITPlus::analyseMaxCurrent(const QByteArray &data)
{
    // 数据异常
    if(!validateResponse(data)){
        return -1.0f;
    }

    // Extract the current value (4 bytes, low byte first)
    uint32_t current = 0;
    current |= static_cast<uint8_t>(data[3]);
    current |= static_cast<uint8_t>(data[4]) << 8;
    current |= static_cast<uint8_t>(data[5]) << 16;
    current |= static_cast<uint8_t>(data[6]) << 24;

    // Convert to float: 1 unit = 0.1 mA
    return static_cast<float>(current) / 10000.0f;
}

// 设置负载的最大输入功率值，1表示1mW
QByteArray EleLoad_ITPlus::createSetMaxPowerCommand(float power) {
    uint32_t sendPower = static_cast<uint32_t>(power * 1000); // 1mW units
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x26;

    command[3] = static_cast<uint8_t>(sendPower & 0xFF);
    command[4] = static_cast<uint8_t>((sendPower >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendPower >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendPower >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载的最大输入功率值
QByteArray EleLoad_ITPlus::createGetMaxPowerCommand() {
    return createGetMessage(0x27);
}

// 解析报文获取最大输入功率值
float EleLoad_ITPlus::analyseMaxPower(const QByteArray &data) {
    if (!validateResponse(data)) {
        return -1.0f;
    }
    uint32_t power = static_cast<uint8_t>(data[3]) |
                     (static_cast<uint8_t>(data[4]) << 8) |
                     (static_cast<uint8_t>(data[5]) << 16) |
                     (static_cast<uint8_t>(data[6]) << 24);
    return power / 1000.0f;
}

// 设置负载模式,mode = 0 为CC, 1 为输出CV, 2 为CW, 3 为CR
QByteArray EleLoad_ITPlus::createSetLoadModeCommand(uint8_t mode) {
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x28;
    command[3] = mode;
    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载模式
QByteArray EleLoad_ITPlus::createGetLoadModeCommand() {
    return createGetMessage(0x29);
}

// 解析报文获取负载模式
uint8_t EleLoad_ITPlus::analyseLoadMode(const QByteArray &data) {
    if (!validateResponse(data)) {
        return 0xFF; // Invalid mode
    }
    return static_cast<uint8_t>(data[3]);
}

// 设置负载的定电流值
QByteArray EleLoad_ITPlus::createSetConstantCurrentCommand(float current) {
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

// 读取负载的定电流值
QByteArray EleLoad_ITPlus::createGetConstantCurrentCommand() {
    return createGetMessage(0x2B);
}

// 解析报文获取定电流值
float EleLoad_ITPlus::analyseConstantCurrent(const QByteArray &data) {
    if (!validateResponse(data)) {
        return -1.0f;
    }
    uint32_t current = static_cast<uint8_t>(data[3]) |
                       (static_cast<uint8_t>(data[4]) << 8) |
                       (static_cast<uint8_t>(data[5]) << 16) |
                       (static_cast<uint8_t>(data[6]) << 24);
    return current / 10000.0f;
}

// 设置负载的定电压值
QByteArray EleLoad_ITPlus::createSetConstantVoltageCommand(float voltage) {
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

// 读取负载的定电压值
QByteArray EleLoad_ITPlus::createGetConstantVoltageCommand() {
    return createGetMessage(0x2D);
}

// 解析报文获取定电压值
float EleLoad_ITPlus::analyseConstantVoltage(const QByteArray &data) {
    if (!validateResponse(data)) {
        return -1.0f;
    }
    uint32_t voltage = static_cast<uint8_t>(data[3]) |
                       (static_cast<uint8_t>(data[4]) << 8) |
                       (static_cast<uint8_t>(data[5]) << 16) |
                       (static_cast<uint8_t>(data[6]) << 24);
    return voltage / 1000.0f;
}

// 设置负载的定功率值
QByteArray EleLoad_ITPlus::createSetConstantPowerCommand(float power) {
    uint32_t sendPower = static_cast<uint32_t>(power * 1000); // 1mW units
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x2E;

    command[3] = static_cast<uint8_t>(sendPower & 0xFF);
    command[4] = static_cast<uint8_t>((sendPower >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendPower >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendPower >> 24) & 0xFF);

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载的定功率值
QByteArray EleLoad_ITPlus::createGetConstantPowerCommand() {
    return createGetMessage(0x2F);
}

// 解析报文获取定功率值
float EleLoad_ITPlus::analyseConstantPower(const QByteArray &data) {
    if (!validateResponse(data)) {
        return -1.0f;
    }
    uint32_t power = static_cast<uint8_t>(data[3]) |
                       (static_cast<uint8_t>(data[4]) << 8) |
                       (static_cast<uint8_t>(data[5]) << 16) |
                       (static_cast<uint8_t>(data[6]) << 24);
    return power / 1000.0f;
}

// 设置负载的定电阻值
QByteArray EleLoad_ITPlus::createSetConstantResistanceCommand(float resistance) {
    uint32_t sendResistance = static_cast<uint32_t>(resistance * 1000); // 1mR units
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

// 读取负载的定电阻值
QByteArray EleLoad_ITPlus::createGetConstantResistanceCommand() {
    return createGetMessage(0x31);
}

// 解析报文获取定电阻值
float EleLoad_ITPlus::analyseConstantResistance(const QByteArray &data) {
    if (!validateResponse(data)) {
        return -1.0f;
    }
    uint32_t resistance = static_cast<uint8_t>(data[3]) |
                       (static_cast<uint8_t>(data[4]) << 8) |
                       (static_cast<uint8_t>(data[5]) << 16) |
                       (static_cast<uint8_t>(data[6]) << 24);
    return resistance / 1000.0f;
}

// 设置负载的动态电流参数值
QByteArray EleLoad_ITPlus::createSetDynamicCurrentParamsCommand(float currentA, float timeA,
                                                float currentB, float timeB,
                                                uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x32;  // Command byte for setting dynamic current parameters

    uint32_t sendCurrentA = static_cast<uint32_t>(10000 * currentA);
    command[3] = static_cast<uint8_t>(sendCurrentA & 0xFF);
    command[4] = static_cast<uint8_t>((sendCurrentA >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendCurrentA >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendCurrentA >> 24) & 0xFF);

    uint16_t sendTimeA = static_cast<uint16_t>(10000 * timeA);
    command[7] = static_cast<uint8_t>(sendTimeA & 0xFF);
    command[8] = static_cast<uint8_t>((sendTimeA >> 8) & 0xFF);

    uint32_t sendCurrentB = static_cast<uint32_t>(10000 * currentB);
    command[9] = static_cast<uint8_t>(sendCurrentB & 0xFF);
    command[10] = static_cast<uint8_t>((sendCurrentB >> 8) & 0xFF);
    command[11] = static_cast<uint8_t>((sendCurrentB >> 16) & 0xFF);
    command[12] = static_cast<uint8_t>((sendCurrentB >> 24) & 0xFF);

    uint16_t sendTimeB = static_cast<uint16_t>(10000 * timeB);
    command[13] = static_cast<uint8_t>(sendTimeB & 0xFF);
    command[14] = static_cast<uint8_t>((sendTimeB >> 8) & 0xFF);

    command[15] = mode;

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载的动态电流参数值
QByteArray EleLoad_ITPlus::createGetDynamicCurrentParamsCommand()
{
    return createGetMessage(0x33);
}

// 解析报文获取的负载的动态电流值
EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicCurrentParams(const QByteArray &data)
{
    DynamicParams params;

    if (!validateResponse(data)) {
        return params;
    }
    /*
     * static_cast<uint8_t>(data[3]) |
                       (static_cast<uint8_t>(data[4]) << 8) |
                       (static_cast<uint8_t>(data[5]) << 16) |
                       (static_cast<uint8_t>(data[6]) << 24);
     */

    uint32_t recvCurrentA = static_cast<uint8_t>(data[3]) |
                            (static_cast<uint8_t>(data[4]) << 8) |
                            (static_cast<uint8_t>(data[5]) << 16) |
                            (static_cast<uint8_t>(data[6]) << 24);
    params.valueA = recvCurrentA / 10000.0f;

    uint16_t recvTimeA = static_cast<uint8_t>(data[8]) << 8 | static_cast<uint8_t>(data[7]);
    params.timeA = recvTimeA / 10000.0f;

    uint32_t recvCurrentB = (static_cast<uint8_t>(data[12]) << 24) |
                            (static_cast<uint8_t>(data[11]) << 16) |
                            (static_cast<uint8_t>(data[10]) << 8) |
                            static_cast<uint8_t>(data[9]);
    params.valueB = recvCurrentB / 10000.0f;

    uint16_t recvTimeB = static_cast<uint8_t>(data[14]) << 8 | static_cast<uint8_t>(data[13]);
    params.timeB = recvTimeB / 10000.0f;

    params.mode = data[15];

    return params;
}

// 设置负载的动态电压参数值
QByteArray EleLoad_ITPlus::createSetDynamicVoltageParamsCommand(float voltageA, float timeA,
                                                                   float voltageB, float timeB,
                                                                   uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x34;  // Command byte for setting dynamic current parameters

    uint32_t sendVoltageA = static_cast<uint32_t>(1000 * voltageA);
    command[3] = static_cast<uint8_t>(sendVoltageA & 0xFF);
    command[4] = static_cast<uint8_t>((sendVoltageA >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendVoltageA >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendVoltageA >> 24) & 0xFF);

    uint16_t sendTimeA = static_cast<uint16_t>(10000 * timeA);
    command[7] = static_cast<uint8_t>(sendTimeA & 0xFF);
    command[8] = static_cast<uint8_t>((sendTimeA >> 8) & 0xFF);

    uint32_t sendVoltageB = static_cast<uint32_t>(1000 * voltageB);
    command[9] = static_cast<uint8_t>(sendVoltageB & 0xFF);
    command[10] = static_cast<uint8_t>((sendVoltageB >> 8) & 0xFF);
    command[11] = static_cast<uint8_t>((sendVoltageB >> 16) & 0xFF);
    command[12] = static_cast<uint8_t>((sendVoltageB >> 24) & 0xFF);

    uint16_t sendTimeB = static_cast<uint16_t>(10000 * timeB);
    command[13] = static_cast<uint8_t>(sendTimeB & 0xFF);
    command[14] = static_cast<uint8_t>((sendTimeB >> 8) & 0xFF);

    command[15] = mode;

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载的动态电流参数值
QByteArray EleLoad_ITPlus::createGetDynamicVoltageParamsCommand()
{
    return createGetMessage(0x35);
}

// 解析报文获取的负载的动态电流值
EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicVoltageParams(const QByteArray &data)
{
    DynamicParams params;

    if (!validateResponse(data)) {
        return params;
    }

    uint32_t recvVoltageA = (static_cast<uint8_t>(data[6]) << 24) |
                            (static_cast<uint8_t>(data[5]) << 16) |
                            (static_cast<uint8_t>(data[4]) << 8) |
                            static_cast<uint8_t>(data[3]);
    params.valueA = recvVoltageA / 1000.0f;

    uint16_t recvTimeA = static_cast<uint8_t>(data[8]) << 8 | static_cast<uint8_t>(data[7]);
    params.timeA = recvTimeA / 10000.0f;

    uint32_t recvVoltageB = (static_cast<uint8_t>(data[12]) << 24) |
                            (static_cast<uint8_t>(data[11]) << 16) |
                            (static_cast<uint8_t>(data[10]) << 8) |
                            static_cast<uint8_t>(data[9]);
    params.valueB = recvVoltageB / 1000.0f;

    uint16_t recvTimeB = static_cast<uint8_t>(data[14]) << 8 | static_cast<uint8_t>(data[13]);
    params.timeB = recvTimeB / 10000.0f;

    params.mode = data[15];

    return params;
}

// 设置负载的动态功率参数值
QByteArray EleLoad_ITPlus::createSetDynamicPowerParamsCommand(float powerA, float timeA,
                                                float powerB, float timeB,
                                                uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x36;  // Command byte for setting dynamic current parameters

    uint32_t sendPowerA = static_cast<uint32_t>(1000 * powerA);
    command[3] = static_cast<uint8_t>(sendPowerA & 0xFF);
    command[4] = static_cast<uint8_t>((sendPowerA >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendPowerA >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendPowerA >> 24) & 0xFF);

    uint16_t sendTimeA = static_cast<uint16_t>(10000 * timeA);
    command[7] = static_cast<uint8_t>(sendTimeA & 0xFF);
    command[8] = static_cast<uint8_t>((sendTimeA >> 8) & 0xFF);

    uint32_t sendPowerB = static_cast<uint32_t>(1000 * powerB);
    command[9] = static_cast<uint8_t>(sendPowerB & 0xFF);
    command[10] = static_cast<uint8_t>((sendPowerB >> 8) & 0xFF);
    command[11] = static_cast<uint8_t>((sendPowerB >> 16) & 0xFF);
    command[12] = static_cast<uint8_t>((sendPowerB >> 24) & 0xFF);

    uint16_t sendTimeB = static_cast<uint16_t>(10000 * timeB);
    command[13] = static_cast<uint8_t>(sendTimeB & 0xFF);
    command[14] = static_cast<uint8_t>((sendTimeB >> 8) & 0xFF);

    command[15] = mode;

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载的动态功率参数值
QByteArray EleLoad_ITPlus::createGetDynamicPowerParamsCommand()
{
    return createGetMessage(0x37);
}

// 解析报文获取的负载的动态功率值
EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicPowerParams(const QByteArray &data)
{
    DynamicParams params;

    if (!validateResponse(data)) {
        return params;
    }

    uint32_t recvPowerA = (static_cast<uint8_t>(data[6]) << 24) |
                          (static_cast<uint8_t>(data[5]) << 16) |
                          (static_cast<uint8_t>(data[4]) << 8) |
                          static_cast<uint8_t>(data[3]);
    params.valueA = recvPowerA / 1000.0f;

    uint16_t recvTimeA = static_cast<uint8_t>(data[8]) << 8 | static_cast<uint8_t>(data[7]);
    params.timeA = recvTimeA / 10000.0f;

    uint32_t recvPowerB = (static_cast<uint8_t>(data[12]) << 24) |
                          (static_cast<uint8_t>(data[11]) << 16) |
                          (static_cast<uint8_t>(data[10]) << 8) |
                          static_cast<uint8_t>(data[9]);
    params.valueB = recvPowerB / 1000.0f;

    uint16_t recvTimeB = static_cast<uint8_t>(data[14]) << 8 | static_cast<uint8_t>(data[13]);
    params.timeB = recvTimeB / 10000.0f;

    params.mode = data[15];

    return params;
}

// 设置负载的动态电阻参数值
QByteArray EleLoad_ITPlus::createSetDynamicResistanceParamsCommand(float resistanceA, float timeA,
                                                float resistanceB, float timeB,
                                                uint8_t mode)
{
    QByteArray command(CommandLength, 0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x38;  // Command byte for setting dynamic current parameters

    uint32_t sendResistanceA = static_cast<uint32_t>(1000 * resistanceA);
    command[3] = static_cast<uint8_t>(sendResistanceA & 0xFF);
    command[4] = static_cast<uint8_t>((sendResistanceA >> 8) & 0xFF);
    command[5] = static_cast<uint8_t>((sendResistanceA >> 16) & 0xFF);
    command[6] = static_cast<uint8_t>((sendResistanceA >> 24) & 0xFF);

    uint16_t sendTimeA = static_cast<uint16_t>(10000 * timeA);
    command[7] = static_cast<uint8_t>(sendTimeA & 0xFF);
    command[8] = static_cast<uint8_t>((sendTimeA >> 8) & 0xFF);

    uint32_t sendResistanceB = static_cast<uint32_t>(1000 * resistanceB);
    command[9] = static_cast<uint8_t>(sendResistanceB & 0xFF);
    command[10] = static_cast<uint8_t>((sendResistanceB >> 8) & 0xFF);
    command[11] = static_cast<uint8_t>((sendResistanceB >> 16) & 0xFF);
    command[12] = static_cast<uint8_t>((sendResistanceB >> 24) & 0xFF);

    uint16_t sendTimeB = static_cast<uint16_t>(10000 * timeB);
    command[13] = static_cast<uint8_t>(sendTimeB & 0xFF);
    command[14] = static_cast<uint8_t>((sendTimeB >> 8) & 0xFF);

    command[15] = mode;

    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 读取负载的动态电阻参数值
QByteArray EleLoad_ITPlus::createGetDynamicResistanceParamsCommand()
{
    return createGetMessage(0x39);
}

// 解析报文获取的负载的动态电阻值
EleLoad_ITPlus::DynamicParams EleLoad_ITPlus::analyseDynamicResistanceParams(const QByteArray &data)
{
    DynamicParams params;

    if (!validateResponse(data)) {
        return params;
    }

    uint32_t recvResistanceA = (static_cast<uint8_t>(data[6]) << 24) |
                               (static_cast<uint8_t>(data[5]) << 16) |
                               (static_cast<uint8_t>(data[4]) << 8) |
                               static_cast<uint8_t>(data[3]);
    params.valueA = recvResistanceA / 1000.0f;

    uint16_t recvTimeA = static_cast<uint8_t>(data[8]) << 8 | static_cast<uint8_t>(data[7]);
    params.timeA = recvTimeA / 10000.0f;

    uint32_t recvResistanceB = (static_cast<uint8_t>(data[12]) << 24) |
                               (static_cast<uint8_t>(data[11]) << 16) |
                               (static_cast<uint8_t>(data[10]) << 8) |
                               static_cast<uint8_t>(data[9]);
    params.valueB = recvResistanceB / 1000.0f;

    uint16_t recvTimeB = static_cast<uint8_t>(data[14]) << 8 | static_cast<uint8_t>(data[13]);
    params.timeB = recvTimeB / 10000.0f;

    params.mode = data[15];

    return params;
}

// 发送一个BUS触发信号（5AH）
QByteArray EleLoad_ITPlus::createBusTriggerSignal()
{
    return createGetMessage(0x5A);
}
// 设置或读取负载的工作模式(5DH/5EH)
QByteArray EleLoad_ITPlus::createSetWorkModel(uint8_t model)
{
    QByteArray command(CommandLength,0x00);
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
quint8 EleLoad_ITPlus::analyseWorkModel(const QByteArray &data)
{
    return static_cast<uint8_t>(data[3]);
}

// 读取负载的输入电压,输入电流,输入功率及相关状态(5FH)
QByteArray EleLoad_ITPlus::createGetInputCommand()
{
    return createGetMessage(0x5F);
}

// 解析报文获取负载的输入值与相关状态
EleLoad_ITPlus::InputParams EleLoad_ITPlus::analyseInputParams(const QByteArray &data)
{
    InputParams params;

    // 验证报文是否有效
    if (!validateResponse(data)) {
        return params;
    }

    // 按小端字节序解析实际输入电压值
    uint32_t recvVoltage = (static_cast<uint8_t>(data[3]) |
                            static_cast<uint8_t>(data[4]) << 8 |
                            static_cast<uint8_t>(data[5]) << 16 |
                            static_cast<uint8_t>(data[6]) << 24);

    params.voltage = recvVoltage / 1000.0f;

    // 按小端字节序解析实际输入电流值
    uint32_t recvCurrent = (static_cast<uint8_t>(data[7]) |
                            static_cast<uint8_t>(data[8]) << 8 |
                            static_cast<uint8_t>(data[9]) << 16 |
                            static_cast<uint8_t>(data[10]) << 24);
    params.current = recvCurrent / 10000.0f;

    // 按小端字节序解析实际输入功率值
    uint32_t recvPower = (static_cast<uint8_t>(data[11]) |
                          static_cast<uint8_t>(data[12]) << 8 |
                          static_cast<uint8_t>(data[13]) << 16 |
                          static_cast<uint8_t>(data[14]) << 24);
    params.power = recvPower / 1000.0f;

    // 操作状态寄存器
    params.handleStateRegister = parseHandleState(static_cast<uint8_t>(data[15]));

    // 查询状态寄存器（2 字节，小端字节序）
    params.selStateRegister = static_cast<uint8_t>(data[16]) |
                               static_cast<uint8_t>(data[17]) << 8;

    // 散热器温度
    params.radiatorTemperature = static_cast<uint8_t>(data[20]);

    // 工作模式
    params.workMode = static_cast<uint8_t>(data[21]);

    // 当前 LIST 的步数
    params.listStep = static_cast<uint8_t>(data[22]);

    // 当前 LIST 的循环次数（2 字节，小端字节序）
    params.listCycleIndex = static_cast<uint8_t>(data[23]) |
                            static_cast<uint8_t>(data[24]) << 8;

    // 获取当前时间并设置给 params.time
    params.time = std::chrono::system_clock::now();

    return params;
}

// 读取、解析负载的产品序列号、产品型号及软件版本号（6AH）
QByteArray EleLoad_ITPlus::createGetProductMessage()
{
    return createGetMessage(0x6A);
}
EleLoad_ITPlus::ProductMessage EleLoad_ITPlus::analyseProductMessage(const QByteArray &data)
{
    ProductMessage pResult;
    pResult.productModel = QString(data.mid(3,5));  // ascii转换为QStirng可以直接隐式转换

    // 获取当前两个字节
    uchar lowByte = static_cast<uchar>(data[8]);
    uchar highByte = static_cast<uchar>(data[9]);
    // 提取低字节和高字节的各自的4位BCD数字
    uchar lowDigit = lowByte & 0x0F;   // 低字节的4位
    uchar highDigit = highByte >> 4;   // 高字节的4位
    // 将低高位BCD码转换为字符并拼接
    QString softVer;
    softVer.append(QChar('0' + highDigit));  // 高位
    softVer.append(QChar('0' + lowDigit));   // 低位
    pResult.softwareVersion = softVer;

    pResult.productSerial = QString(data.mid(10,10));

    return pResult;
}

// 新版模拟键盘按下（98H）
QByteArray EleLoad_ITPlus::createAnalogKeyboardPress(quint8 keyboard)
{
    QByteArray command(CommandLength,0x00);
    command[0] = SyncHeader;
    command[1] = m_loadAddress;
    command[2] = 0x98;
    command[3] = keyboard;
    command[CommandLength - 1] = calculateChecksum(command);
    return command;
}

// 新版发送一个触发信号（9DH）
QByteArray EleLoad_ITPlus::createNewTriggerSignal()
{
    return createGetMessage(0x9D);
}
