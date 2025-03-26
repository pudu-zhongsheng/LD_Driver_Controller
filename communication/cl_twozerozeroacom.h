#ifndef CL_TWOZEROZEROACOM_H
#define CL_TWOZEROZEROACOM_H

#include <QWidget>

class CL_TwoZeroZeroACOM : public QWidget
{
    Q_OBJECT
public:
    explicit CL_TwoZeroZeroACOM(QWidget *parent = nullptr);

    
    // 结构体，存放解析接收数据的结果（长报文，读取测量状态和数据）
    struct MeasurementData{
        QString registerCode;   // Long功能码01、02、03、08、15 Special功能码45、47
        char fixedValue;    // 固定值 1or5 正常运行    （47 0or1 正常运行）
        char err;           // 错误信息 " "、"4"：正常运行；"1"-"3"：重启cl-200a；"5"：超过误差的测量值（请降低亮度或拉远光源距离）（47中5是正常）;、"6"、"7"可能是正常，也可能是异常
        char rng;           // 范围状态 "0"：发送命令和响应的周期可能不正确，请设置正确的等待时间；"1"-"4"：正常；"6"：测量结果超出范围，请执行EXT模式；（47中固定为20h）
        char ba;            // 电池电量 "0"正常，"1"电量不足；（47中固定为20h）
        float data1;      // 测量值
        float data2;      // 测量值
        float data3;      // 测量值
    };
    // 结构体，存放短报文指令响应结果
    struct ShortMeasurementData{
        QString registerCode;   // 功能码40、48、54
        // 错误信息 " "：正常运行；"1"-"3"：重启cl-200a；"4"：请先发送55指令设置为保持状态；"5"-"7"：表示之前测量数据异常，但是不影响本操作；
        // 48中"4"是设置数值超出范围，54中所有参数都是固定
        char err;
    };

    static QString calculateBCC(const QByteArray &data);    // 计算校验位

    ShortMeasurementData handleReceivedSettingResult(const QByteArray &data);   // Short通讯模式，获取设置指令的响应结果
    MeasurementData handleReceivedMeasurementData(const QByteArray &data);  // Long通讯模式，处理接收到的测量数据
    MeasurementData handleReceivedMeasurementDataSpecial(const QByteArray &data);  // special通讯模式，处理接收到的测量数据
    float parseLongDataBlock(const QByteArray &block);   // 解析长报文的数据块
    QByteArray createSpecialFormatMessage(float value1, float value2, float value3);    // 封装特殊报文
    float parseSpecialDataBlock(const QByteArray &block);   // 解析特殊报文的数据块

    QByteArray createMessage(const QByteArray &data);   // 封装数据报文

    // 01读取测量数据(x,y,z);02读取测量数据(EV,x,y);03读取测量数据(EV,u',v')
    // 08读取测量数据(EV,Tcp,Δuv);15读取测量数据(EV,DW,P);
    // 45读取测量数据(x2,y,z) 不确定用短报文还是特殊报文？
    QByteArray readMeasure(const QString &registCode, quint8 header, char CFable, char calibrationMode);
    QByteArray setEXT40(quint8 header);        // 设置EXT模式（有应答）
    QByteArray takeEXT40();                    // 进行EXT测量（无应答）
    QByteArray readCalibrationFactor47(quint8 header, char MCF); // 读取用户校准系数
    QByteArray setCalibrationFactor48(quint8 header, char MCF, const QString &data1, const QString &data2,
                                      const QString &data3);  // 写入用户校准系数，data是8个字符(0-F)的字符串
    QByteArray setPCConnect54();    // 设置PC连接模式
    QByteArray setHoldState55();    // 设置保持状态（无应答）
};

#endif // CL_TWOZEROZEROACOM_H