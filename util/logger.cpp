#include "logger.h"
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QMutex>
#include <QApplication>

namespace {
    QMutex logMutex;  // 用于多线程日志写入同步
    const QString LOG_PATH = "logs";
    const QString LOG_FILE_FORMAT = "yyyy-MM-dd.log";
    const int MAX_LOG_DAYS = 30;  // 日志保留天数
}

void Logger::info(const QString &message)
{
    writeLog("INFO", message);
    qDebug() << "[INFO]" << message;
}

void Logger::warning(const QString &message)
{
    writeLog("WARNING", message);
    qWarning() << "[WARNING]" << message;
}

void Logger::error(const QString &message)
{
    writeLog("ERROR", message);
    qCritical() << "[ERROR]" << message;
}

void Logger::writeLog(const QString &level, const QString &message)
{
    QMutexLocker locker(&logMutex);

    // 创建日志目录
    QString logDir = QApplication::applicationDirPath() + "/" + LOG_PATH;
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 清理旧日志
    cleanOldLogs(dir);

    // 获取当前日期作为文件名
    QString fileName = QDateTime::currentDateTime().toString(LOG_FILE_FORMAT);
    QFile file(dir.filePath(fileName));

    // 以追加模式打开文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");

        // 写入日志
        QString logLine = QString("%1 [%2] %3\n")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(level)
            .arg(message);
        stream << logLine;
        file.close();
    }
}

void Logger::cleanOldLogs(const QDir &logDir)
{
    QDateTime now = QDateTime::currentDateTime();
    QFileInfoList files = logDir.entryInfoList(QStringList() << "*.log", QDir::Files);

    for (const QFileInfo &file : files) {
        QDateTime fileDate = QDateTime::fromString(file.baseName(), "yyyy-MM-dd");
        if (fileDate.isValid() && fileDate.daysTo(now) > MAX_LOG_DAYS) {
            QFile::remove(file.filePath());
        }
    }
} 