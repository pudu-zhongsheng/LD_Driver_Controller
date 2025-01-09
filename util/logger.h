#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDir>

/**
 * 日志记录器
    分级日志（INFO/WARNING/ERROR）
    自动日志轮转（保留30天）
    线程安全的日志写入
    日志格式化（时间戳、级别、消息）
 */
class Logger {
public:
    static void info(const QString &message);
    static void warning(const QString &message);
    static void error(const QString &message);
    
private:
    static void writeLog(const QString &level, const QString &message);
    static void cleanOldLogs(const QDir &dir);
}; 

#endif // LOGGER_H

/**
 * 使用示例

// 在主窗口中使用
void MainWindow::initConnections()
{
    // ... 其他连接 ...

    // 连接错误处理器
    connect(ERROR_HANDLER, &ErrorHandler::errorOccurred,
            this, [this](const QString &message, ErrorHandler::ErrorLevel level) {
        // 在状态栏显示错误信息
        if (level >= ErrorHandler::ErrorLevel::Warning) {
            statusBar()->showMessage(message, 5000);
        }
    });
}

// 在串口通信中使用
void SerialUtil::handleError(const QString &error)
{
    HANDLE_ERROR_CTX("串口通信", error);
}

// 在驱动控制中使用
void Driver8CH::onChannelValueChanged()
{
    QByteArray cmd;
    try {
        // 构造命令
        // ...
    } catch (const std::exception &e) {
        HANDLE_ERROR_CTX("驱动控制", QString("设置通道值失败: %1").arg(e.what()));
        return;
    }
    
    // 记录操作日志
    LOG_INFO(QString("设置通道值: %1").arg(value));
}

// 在电子负载控制中使用
void EleLoad_ITPlus::makeSetValueCommand(double value)
{
    if (value < 0 || value > maxValue) {
        HANDLE_ERROR_CTX("电子负载", QString("设定值超出范围: %1").arg(value));
        return QByteArray();
    }
    LOG_INFO(QString("设置电子负载值: %1").arg(value));
    // ...
}
 */
