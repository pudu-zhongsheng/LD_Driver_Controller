#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QString>
#include <QObject>
#include "logger.h"

/**
 * 错误处理
 * 错误处理：
 * 错误级别分类（INFO/WARNING/ERROR/CRITICAL）
 * 错误信息格式化（时间戳、级别、消息、上下文）
 * 错误信息输出（日志、对话框）
 * 错误信息传播（信号）
 */
class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    enum class ErrorLevel {
        Info,
        Warning,
        Error,
        Critical
    };

    static ErrorHandler* instance();

    void handleError(const QString &message, ErrorLevel level = ErrorLevel::Error);
    void handleError(const QString &context, const QString &message, ErrorLevel level = ErrorLevel::Error);
    
    
signals:
    void errorOccurred(const QString &message, ErrorLevel level);

private:
    void showErrorDialog(const QString &message, ErrorLevel level);

    explicit ErrorHandler(QObject *parent = nullptr);
    static ErrorHandler* m_instance;
};

// 便捷宏
#define ERROR_HANDLER ErrorHandler::instance()
#define LOG_INFO(msg) do { Logger::info(msg); } while(0)
#define LOG_WARNING(msg) do { Logger::warning(msg); } while(0)
#define LOG_ERROR(msg) do { Logger::error(msg); } while(0)
#define HANDLE_ERROR(msg) do { ERROR_HANDLER->handleError(msg); } while(0)
#define HANDLE_ERROR_CTX(ctx, msg) do { ERROR_HANDLER->handleError(ctx, msg); } while(0)

#endif // ERRORHANDLER_H 