#ifndef ELELOAD_ITPLUS_H
#define ELELOAD_ITPLUS_H

#include "protocol.h"
#include <QObject>
#include <chrono>

class EleLoad_ITPlus : public Protocol
{
    Q_OBJECT
public:
    explicit EleLoad_ITPlus(uint8_t loadAddress = 0xFF, QObject *parent = nullptr);

    // 工作模式
    enum class Mode {
        CC,     // 恒流模式
        CV,     // 恒压模式
        CR,     // 恒阻模式
        CW      // 恒功率模式
    };

    // 量程
    enum class Range {
        HIGH,   // 高量程
        LOW     // 低量程
    };

    // 基本命令接口
    QByteArray makeReadCommand(int address, int count = 1) override;
    QByteArray makeWriteCommand(int address, const QByteArray &data) override;
    bool parseResponse(const QByteArray &response, QByteArray &data) override;

    // 动态参数结构体
    struct DynamicParams {
        float valueA;
        float timeA;
        float valueB;
        float timeB;
        uint8_t mode;
    };

    // 操作状态寄存器结构体
    struct HandleStateRegister {
        bool cal;       // 校准模式
        bool wtg;       // 负载在等待触发状态
        bool rem;       // 远端控制模式
        bool out;       // 负载输出状态
        bool local;     // 本地操作状态
        bool sense;     // 远端测量模式
        bool lot;       // LOT状态
        bool noUse;     // 保留位
    };

    // 输入参数结构体
    struct InputParams {
        float voltage;
        float current;
        float power;
        HandleStateRegister handleStateRegister;
        uint16_t selStateRegister;
        uint8_t radiatorTemperature;
        uint8_t workMode;
        uint8_t listStep;
        uint16_t listCycleIndex;
        std::chrono::system_clock::time_point time;
    };

    // 产品信息结构体
    struct ProductMessage {
        QString productModel;
        QString softwareVersion;
        QString productSerial;
    };

    struct LoadStatus {
        float voltage;
        float current;
        float power;
        float temperature;
        bool isRemoteControl;
        bool isOutputOn;
    };

    // 基本控制命令
    QByteArray createControlModeCommand(uint8_t mode);
    QByteArray createLoadStateCommand(uint8_t state);
    QByteArray createGetMessage(uint8_t action);

    // 最大值设置命令
    QByteArray createSetMaxVoltageCommand(float voltage);
    QByteArray createGetMaxVoltageCommand();
    float analyseMaxVoltage(const QByteArray &data);

    QByteArray createSetMaxCurrentCommand(float current);
    QByteArray createGetMaxCurrentCommand();
    float analyseMaxCurrent(const QByteArray &data);

    QByteArray createSetMaxPowerCommand(float power);
    QByteArray createGetMaxPowerCommand();
    float analyseMaxPower(const QByteArray &data);

    // 工作模式命令
    QByteArray createSetLoadModeCommand(uint8_t mode);
    QByteArray createGetLoadModeCommand();
    uint8_t analyseLoadMode(const QByteArray &data);

    // 定值设置命令
    QByteArray createSetConstantCurrentCommand(float current);
    QByteArray createGetConstantCurrentCommand();
    float analyseConstantCurrent(const QByteArray &data);

    QByteArray createSetConstantVoltageCommand(float voltage);
    QByteArray createGetConstantVoltageCommand();
    float analyseConstantVoltage(const QByteArray &data);

    QByteArray createSetConstantPowerCommand(float power);
    QByteArray createGetConstantPowerCommand();
    float analyseConstantPower(const QByteArray &data);

    QByteArray createSetConstantResistanceCommand(float resistance);
    QByteArray createGetConstantResistanceCommand();
    float analyseConstantResistance(const QByteArray &data);

    // 动态参数命令
    QByteArray createSetDynamicCurrentParamsCommand(float currentA, float timeA,
                                                   float currentB, float timeB,
                                                   uint8_t mode);
    QByteArray createGetDynamicCurrentParamsCommand();
    DynamicParams analyseDynamicCurrentParams(const QByteArray &data);
    QByteArray createSetDynamicVoltageParamsCommand(float voltageA, float timeA,
                                                   float voltageB, float timeB,
                                                   uint8_t mode);
    QByteArray createGetDynamicVoltageParamsCommand();
    DynamicParams analyseDynamicVoltageParams(const QByteArray &data);
    QByteArray createSetDynamicPowerParamsCommand(float powerA, float timeA,
                                                   float powerB, float timeB,
                                                   uint8_t mode);
    QByteArray createGetDynamicPowerParamsCommand();
    DynamicParams analyseDynamicPowerParams(const QByteArray &data);
    QByteArray createSetDynamicResistanceParamsCommand(float resistanceA, float timeA,
                                                   float resistanceB, float timeB,
                                                   uint8_t mode);
    QByteArray createGetDynamicResistanceParamsCommand();
    DynamicParams analyseDynamicResistanceParams(const QByteArray &data);
    

    // 其他功能命令
    QByteArray createBusTriggerSignal();
    QByteArray createSetWorkModel(uint8_t model);
    QByteArray createGetWorkModel();
    uint8_t analyseWorkModel(const QByteArray &data);
    QByteArray createGetInputCommand();
    InputParams analyseInputParams(const QByteArray &data);
    QByteArray createGetProductMessage();
    ProductMessage analyseProductMessage(const QByteArray &data);
    QByteArray createAnalogKeyboardPress(uint8_t keyboard);
    QByteArray createNewTriggerSignal();

    // 辅助函数
    HandleStateRegister parseHandleState(uint8_t handleStateRegister);
    uint8_t calculateChecksum(const QByteArray &data);
    bool validateResponse(const QByteArray &data);

private:
    uint8_t m_loadAddress;
    static const uint8_t SyncHeader = 0xAA;
    static const int CommandLength = 26;

    // 辅助函数
    QByteArray createCommand(uint8_t cmd, const QByteArray &data = QByteArray());
    
    // 通用的数值转换函数
    template<typename T>
    static uint32_t valueToRaw(T value, float scale);
    
    template<typename T>
    static T rawToValue(const QByteArray &data, int offset, float scale);
    
    // 通用的动态参数命令创建函数
    QByteArray createDynamicParamsCommand(uint8_t cmd, float valueA, float timeA,
                                        float valueB, float timeB, uint8_t mode,
                                        float scale);
    
    // 通用的动态参数解析函数
    DynamicParams analyseDynamicParams(const QByteArray &data, float scale);
    
    // 通用的设置/获取值命令
    QByteArray createSetValueCommand(uint8_t cmd, float value, float scale);
    float analyseValue(const QByteArray &data, float scale);
};

#endif // ELELOAD_ITPLUS_H