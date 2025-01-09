#include "config.h"
#include <QApplication>
#include <QDir>
#include <QDebug>
#include "logger.h"

namespace {
    const QString CONFIG_FILE = "config.ini";
    const QString CONFIG_PATH = "config";
    // 串口配置键
    const QString SERIAL_BAUDRATE = "SerialPort/BaudRate";
    const QString SERIAL_DATABITS = "SerialPort/DataBits";
}

// 初始化静态成员
QSettings Config::m_settings(
    QApplication::applicationDirPath() + "/" + CONFIG_PATH + "/" + CONFIG_FILE,
    QSettings::IniFormat
);

void Config::loadConfig()
{
    // 确保配置目录存在
    QDir configDir(QApplication::applicationDirPath() + "/" + CONFIG_PATH);
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }

    // 检查配置文件是否存在，不存在则创建默认配置
    if (!QFile::exists(m_settings.fileName())) {
        LOG_INFO("配置文件不存在，创建默认配置");
        createDefaultConfig();
    }

    // 加载配置
    m_settings.sync();
    
    if (m_settings.status() != QSettings::NoError) {
        LOG_ERROR("加载配置文件失败");
        return;
    }

    LOG_INFO("配置文件加载成功");
}

void Config::saveConfig()
{
    m_settings.sync();
    
    if (m_settings.status() != QSettings::NoError) {
        LOG_ERROR("保存配置文件失败");
        return;
    }

    LOG_INFO("配置文件保存成功");
}

QVariant Config::getValue(const QString &key, const QVariant &defaultValue)
{
    return m_settings.value(key, defaultValue);
}

void Config::setValue(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

void Config::createDefaultConfig()
{
    // 串口设置
    m_settings.beginGroup("SerialPort");
    m_settings.setValue("BaudRate", 9600);
    m_settings.setValue("DataBits", 8);
    m_settings.setValue("Parity", "None");
    m_settings.setValue("StopBits", 1);
    m_settings.setValue("FlowControl", "None");
    m_settings.endGroup();

    // 驱动设置
    m_settings.beginGroup("Driver");
    m_settings.setValue("DefaultLevel", "High");
    m_settings.setValue("StartRegister", 1);
    m_settings.setValue("RegisterCount", 8);
    m_settings.endGroup();

    // 电子负载设置
    m_settings.beginGroup("ELoad");
    m_settings.setValue("DefaultMode", "CC");
    m_settings.setValue("DefaultRange", "HIGH");
    m_settings.setValue("MaxCurrent", 5.0);
    m_settings.setValue("MaxVoltage", 30.0);
    m_settings.setValue("MaxPower", 150.0);
    m_settings.endGroup();

    // 照度计设置
    m_settings.beginGroup("Meter");
    m_settings.setValue("DefaultRange", 2);
    m_settings.setValue("BacklightEnabled", true);
    m_settings.setValue("AutoRange", true);
    m_settings.endGroup();

    // UI设置
    m_settings.beginGroup("UI");
    m_settings.setValue("Theme", "Default");
    m_settings.setValue("Language", "zh_CN");
    m_settings.setValue("ChartUpdateInterval", 100);
    m_settings.setValue("AutoSaveInterval", 300);
    m_settings.endGroup();

    // 保存默认配置
    m_settings.sync();
}

void Config::LOG_INFO(const QString &message)
{
    Logger::info(message);
}

void Config::LOG_WARNING(const QString &message)
{
    Logger::warning(message);
}

void Config::LOG_ERROR(const QString &message)
{
    Logger::error(message);
}
