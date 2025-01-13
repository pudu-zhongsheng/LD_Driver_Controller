#ifndef ELELOAD_ITPLUS_H
#define ELELOAD_ITPLUS_H

#include <QWidget>
#include <QDebug>

class EleLoad_ITPlus : public QWidget
{
    Q_OBJECT
public:
    explicit EleLoad_ITPlus(uint8_t loadAddress = 0xFF, QWidget *parent = nullptr);
    ~EleLoad_ITPlus();

    uint8_t calculateChecksum(const QByteArray &data);  // 计算校验和

    bool validateResponse(const QByteArray &data);  // 校验接收报文是否准确

    // 结构体，用来接收动态数值
    struct DynamicParams {
        float valueA;
        float timeA;
        float valueB;
        float timeB;
        uint8_t mode;
    };

    // 结构体，操作状态寄存器
    struct HandleStateRegister{
        bool cal;       // 校准模式
        bool wtg;       // 负载在等待触发状态
        bool rem;       // 远端控制模式
        bool out;       // 负载输出状态
        bool local;     // 本地操作状态
        bool sense;     // 远端测量模式
        bool lot;       // LOT状态
        bool noUse;     // 保留位

        void printState() {
            qDebug() << "CAL:" << cal
                     << "WTG:" << wtg
                     << "REM:" << rem
                     << "OUT:" << out
                     << "LOCAL:" << local
                     << "SENSE:" << sense
                     << "LOT:" << lot
                     << "NO USE:" << noUse;
        }
    };

    // 结构体，用来接收负载的输入值及相关状态
    struct InputParams {
        float voltage;
        float current;
        float power;
        HandleStateRegister handleStateRegister;    // 操作状态寄存器
        uint16_t selStateRegister;       // 查询状态寄存器
        uint8_t radiatorTemperature;    // 散热器温度
        uint8_t workMode;               // 工作模式
        uint8_t listStep;               // 当前LIST的步数
        uint16_t listCycleIndex;        // 当前LIST的循环次数
        std::chrono::system_clock::time_point time; // 表示当前时间
    };

    HandleStateRegister parseHandleState(quint8 handleStateRegister){
        HandleStateRegister stateRegister;

        // 解析每一位
        stateRegister.noUse = handleStateRegister & 0x80;
        stateRegister.lot = handleStateRegister & 0x40;      // 位6
        stateRegister.sense = handleStateRegister & 0x20;    // 位5
        stateRegister.local = handleStateRegister & 0x10;    // 位4
        stateRegister.out = handleStateRegister & 0x08;      // 位3
        stateRegister.rem = handleStateRegister & 0x04;      // 位2
        stateRegister.wtg = handleStateRegister & 0x02;      // 位1
        stateRegister.cal = handleStateRegister & 0x01;      // 位0

        return stateRegister;
    }

    // 结构体，用来接收产品的序列号、型号和软件版本号
    struct ProductMessage {
        QString productModel;
        QString softwareVersion;
        QString productSerial;
    };

    // 设置负载控制模式（20H），0为面板操作，1为远程操作
    QByteArray createControlModeCommand(uint8_t mode);

    // 控制负载输入状态（21H），0为输出OFF，1为输入ON
    QByteArray createLoadStateCommand(uint8_t state);

    // 发送读取数据请求
    QByteArray createGetMessage(uint8_t action);

    // 负载最大输入电压值（22H/23H），1表示1mV
    QByteArray createSetMaxVoltageCommand(float voltage);
    QByteArray createGetMaxVoltageCommand();
    float analyseMaxVoltage(const QByteArray &data);

    // 负载最大输入电流值（24H/25H），1表示0.1mA
    QByteArray createSetMaxCurrentCommand(float current);
    QByteArray createGetMaxCurrentCommand();
    float analyseMaxCurrent(const QByteArray &data);

    // 负载的最大输入功率值（26H/27H），1表示1mW
    QByteArray createSetMaxPowerCommand(float power);
    QByteArray createGetMaxPowerCommand();
    float analyseMaxPower(const QByteArray &data);

    // 负载FIXED模式（28H/29H），第四字节0为CC，1为CV，2为CW，3为CR
    QByteArray createSetLoadModeCommand(uint8_t mode);
    QByteArray createGetLoadModeCommand();
    uint8_t analyseLoadMode(const QByteArray &data);

    // 负载的定电流值（2AH/2BH）
    QByteArray createSetConstantCurrentCommand(float current);
    QByteArray createGetConstantCurrentCommand();
    float analyseConstantCurrent(const QByteArray &data);

    // 负载的定电压值（2CH/2DH）
    QByteArray createSetConstantVoltageCommand(float voltage);
    QByteArray createGetConstantVoltageCommand();
    float analyseConstantVoltage(const QByteArray &data);

    // 负载的定功率值（2EH/2FH）
    QByteArray createSetConstantPowerCommand(float power);
    QByteArray createGetConstantPowerCommand();
    float analyseConstantPower(const QByteArray &data);

    // 负载的定电阻值（30H/31H）
    QByteArray createSetConstantResistanceCommand(float resistance);
    QByteArray createGetConstantResistanceCommand();
    float analyseConstantResistance(const QByteArray &data);

    // 负载的动态电流参数值（32H/33H）
    QByteArray createSetDynamicCurrentParamsCommand(float currentA, float timeA,
                                                    float currentB, float timeB,
                                                    uint8_t mode);
    QByteArray createGetDynamicCurrentParamsCommand();
    DynamicParams analyseDynamicCurrentParams(const QByteArray &data);

    // 负载的动态电压参数值（34H/35H）
    QByteArray createSetDynamicVoltageParamsCommand(float voltageA, float timeA,
                                                    float voltageB, float timeB,
                                                    uint8_t mode);
    QByteArray createGetDynamicVoltageParamsCommand();
    DynamicParams analyseDynamicVoltageParams(const QByteArray &data);

    // 负载的动态功率参数值（36H/37H）
    QByteArray createSetDynamicPowerParamsCommand(float powerA, float timeA,
                                                  float powerB, float timeB,
                                                  uint8_t mode);
    QByteArray createGetDynamicPowerParamsCommand();
    DynamicParams analyseDynamicPowerParams(const QByteArray &data);

    // 负载的动态电阻参数值（38H/39H）
    QByteArray createSetDynamicResistanceParamsCommand(float resistanceA, float timeA,
                                                       float resistanceB, float timeB,
                                                       uint8_t mode);
    QByteArray createGetDynamicResistanceParamsCommand();
    DynamicParams analyseDynamicResistanceParams(const QByteArray &data);

    // 发送一个BUS触发信号（5AH）
    QByteArray createBusTriggerSignal();

    // 设置或读取负载的工作模式(5DH/5EH)
    QByteArray createSetWorkModel(uint8_t model);
    QByteArray createGetWorkModel();
    quint8 analyseWorkModel(const QByteArray &data);

    // 读取负载的输入电压,输入电流,输入功率及相关状态(5FH)
    QByteArray createGetInputCommand();
    InputParams analyseInputParams(const QByteArray &data);

    // 读取负载的产品序列号、产品型号及软件版本号（6AH）
    QByteArray createGetProductMessage();
    ProductMessage analyseProductMessage(const QByteArray &data);

    // 新版模拟键盘按下（98H）
    QByteArray createAnalogKeyboardPress(quint8 keyboard);

    // 新版发送一个触发信号（9DH）
    QByteArray createNewTriggerSignal();


private:
    uint8_t m_loadAddress;
    const uint8_t SyncHeader = 0xAA;
    static const int CommandLength = 26;
};

#endif // ELELOAD_ITPLUS_H

/*
报文是十六进制格式，固定26个字节长度，第一字节是同步头AA；第二字节是负载地址，FF代表广播
第三字节是命令字，有多种，如20、21、22，分别指代不同命令；
第四字节到第二十五字节就是根据不同指令有不同的表示方式；
第二十六字节就是前二十五个字节的和，用来做校验码。
 */

