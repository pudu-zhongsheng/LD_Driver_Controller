#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QVariant>
#include <QString>

/**
1. 配置文件管理：
INI格式配置文件
自动创建配置目录和文件
默认配置生成
配置读写接口
2. 配置分组：
串口设置
驱动设置
电子负载设置
照度计设置
UI设置
3. 便捷功能：
配置键常量定义
分组访问
默认值支持
配置同步
4. 错误处理：
配置文件检查
错误日志记录
状态检查
 */

// 添加一些便捷的配置访问方法
namespace ConfigKeys {
    // 串口配置键
    const QString SERIAL_BAUDRATE = "SerialPort/BaudRate";
    const QString SERIAL_DATABITS = "SerialPort/DataBits";
    const QString SERIAL_PARITY = "SerialPort/Parity";
    const QString SERIAL_STOPBITS = "SerialPort/StopBits";
    const QString SERIAL_FLOWCONTROL = "SerialPort/FlowControl";

    // 驱动配置键
    const QString DRIVER_LEVEL = "Driver/DefaultLevel";
    const QString DRIVER_START_REG = "Driver/StartRegister";
    const QString DRIVER_REG_COUNT = "Driver/RegisterCount";

    // 电子负载配置键
    const QString ELOAD_MODE = "ELoad/DefaultMode";
    const QString ELOAD_RANGE = "ELoad/DefaultRange";
    const QString ELOAD_MAX_CURRENT = "ELoad/MaxCurrent";
    const QString ELOAD_MAX_VOLTAGE = "ELoad/MaxVoltage";
    const QString ELOAD_MAX_POWER = "ELoad/MaxPower";

    // 照度计配置键
    const QString METER_RANGE = "Meter/DefaultRange";
    const QString METER_BACKLIGHT = "Meter/BacklightEnabled";
    const QString METER_AUTORANGE = "Meter/AutoRange";

    // UI配置键
    const QString UI_THEME = "UI/Theme";
    const QString UI_LANGUAGE = "UI/Language";
    const QString UI_CHART_INTERVAL = "UI/ChartUpdateInterval";
    const QString UI_AUTOSAVE_INTERVAL = "UI/AutoSaveInterval";
}

class Config {
public:
    static void loadConfig();
    static void saveConfig();
    static QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant());
    static void setValue(const QString &key, const QVariant &value);
    
    static void LOG_INFO(const QString &message);
    static void LOG_WARNING(const QString &message);
    static void LOG_ERROR(const QString &message);

    
private:
    static QSettings m_settings;
    static void createDefaultConfig();
}; 

#endif // CONFIG_H

/**
// 使用示例：
void MainWindow::loadSettings()
{
    // 加载串口设置
    int baudRate = Config::getValue(ConfigKeys::SERIAL_BAUDRATE, 9600).toInt();
    int dataBits = Config::getValue(ConfigKeys::SERIAL_DATABITS, 8).toInt();
    QString parity = Config::getValue(ConfigKeys::SERIAL_PARITY, "None").toString();
    
    // 加载驱动设置
    bool isHighLevel = Config::getValue(ConfigKeys::DRIVER_LEVEL, "High").toString() == "High";
    int startReg = Config::getValue(ConfigKeys::DRIVER_START_REG, 1).toInt();
    int regCount = Config::getValue(ConfigKeys::DRIVER_REG_COUNT, 8).toInt();
    
    // 加载UI设置
    int chartInterval = Config::getValue(ConfigKeys::UI_CHART_INTERVAL, 100).toInt();
    QString language = Config::getValue(ConfigKeys::UI_LANGUAGE, "zh_CN").toString();
    
    // 应用设置
    m_dataTimer->setInterval(chartInterval);
    // ...
}

void MainWindow::saveSettings()
{
    // 保存当前设置
    Config::setValue(ConfigKeys::DRIVER_LEVEL, m_isHighLevel ? "High" : "Low");
    Config::setValue(ConfigKeys::DRIVER_START_REG, m_startRegister->value());
    Config::setValue(ConfigKeys::DRIVER_REG_COUNT, m_registerCount->value());
    
    // 保存到文件
    Config::saveConfig();
} 
 */