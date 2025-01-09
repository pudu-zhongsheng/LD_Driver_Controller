#include "errorhandler.h"
#include <QMessageBox>
#include <QApplication>
#include <QThread>

ErrorHandler* ErrorHandler::m_instance = nullptr;

ErrorHandler::ErrorHandler(QObject *parent) : QObject(parent)
{
}

ErrorHandler* ErrorHandler::instance()
{
    if (!m_instance) {
        m_instance = new ErrorHandler(qApp);
    }
    return m_instance;
}

void ErrorHandler::handleError(const QString &message, ErrorLevel level)
{
    handleError("", message, level);
}

void ErrorHandler::handleError(const QString &context, const QString &message, ErrorLevel level)
{
    QString fullMessage = context.isEmpty() ? message : context + ": " + message;

    // 记录日志
    switch (level) {
        case ErrorLevel::Info:
            Logger::info(fullMessage);
            break;
        case ErrorLevel::Warning:
            Logger::warning(fullMessage);
            break;
        case ErrorLevel::Error:
        case ErrorLevel::Critical:
            Logger::error(fullMessage);
            break;
    }

    // 发送信号
    emit errorOccurred(fullMessage, level);

    // 如果不在主线程，将错误处理转移到主线程
    if (QThread::currentThread() != qApp->thread()) {
        QMetaObject::invokeMethod(this, [=]() {
            showErrorDialog(fullMessage, level);
        }, Qt::QueuedConnection);
    } else {
        showErrorDialog(fullMessage, level);
    }
}

void ErrorHandler::showErrorDialog(const QString &message, ErrorLevel level)
{
    switch (level) {
        case ErrorLevel::Info:
            // 信息级别不显示对话框
            break;
        case ErrorLevel::Warning:
            QMessageBox::warning(nullptr, "警告", message);
            break;
        case ErrorLevel::Error:
            QMessageBox::critical(nullptr, "错误", message);
            break;
        case ErrorLevel::Critical:
            QMessageBox::critical(nullptr, "严重错误", message);
            qApp->exit(1);  // 严重错误直接退出程序
            break;
    }
} 