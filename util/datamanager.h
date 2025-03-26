#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "logger.h"

/**
 * 测量数据结构体
 *  1. 数据存储：
    内存中存储测量数据
    自动限制数据点数量
    按时间范围查询数据
    2. 数据导出：
    CSV格式导出
    JSON格式导出
    自定义格式转换
    3. 数据分析：
    计算平均值
    计算最大最小值
    时间范围分析
    4. 数据备份：
    数据备份到文件
    从备份文件恢复
    错误处理和日志记录
 */
struct MeasurementData {
    QDateTime timestamp;
    double current;
    double voltage;
    double power;
    double resistance;
    double illuminance;
    double colorTemp;
    double r;
    double g;
    double b;
};

class DataManager : public QObject
{
    Q_OBJECT
public:
    static DataManager* instance();
    
    // 数据操作
    void addMeasurement(const MeasurementData &data);
    void clearData();
    QVector<MeasurementData> getData(const QDateTime &start, const QDateTime &end) const;
    
    // 数据导出
    bool exportToCSV(const QString &filename) const;
    bool exportToJSON(const QString &filename) const;
    
    // 数据分析
    struct DataAnalysis {
        double avgCurrent;
        double avgVoltage;
        double avgPower;
        double maxCurrent;
        double maxVoltage;
        double maxPower;
        double minCurrent;
        double minVoltage;
        double minPower;
    };
    DataAnalysis analyzeData(const QDateTime &start, const QDateTime &end) const;
    
    // 数据备份
    bool backup(const QString &filename) const;
    bool restore(const QString &filename);

signals:
    void dataAdded(const MeasurementData &data);
    void dataCleared();
    void backupCompleted(bool success) const;
    void restoreCompleted(bool success);

private:
    explicit DataManager(QObject *parent = nullptr);
    static DataManager* m_instance;
    
    QVector<MeasurementData> m_data;
    static const int MAX_DATA_POINTS = 36000; // 保存1小时的数据(100ms采样)
    
    // 辅助函数
    void trimData();
    QString measurementToCSV(const MeasurementData &data) const;
    QJsonObject measurementToJSON(const MeasurementData &data) const;
    MeasurementData jsonToMeasurement(const QJsonObject &json) const;
};

#endif // DATAMANAGER_H 

/**
// 在MainWindow中使用
void MainWindow::updateChartData(const MeasurementData &data)
{
    // 添加数据到管理器
    DataManager::instance()->addMeasurement(data);
    
    // 更新图表显示
    // ...
}

void MainWindow::exportData()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "导出数据",
        QDir::homePath(),
        "CSV文件 (*.csv);;JSON文件 (*.json)"
    );
    
    if (filename.isEmpty()) return;
    
    bool success;
    if (filename.endsWith(".csv")) {
        success = DataManager::instance()->exportToCSV(filename);
    } else {
        success = DataManager::instance()->exportToJSON(filename);
    }
    
    if (success) {
        new ToastMessage("数据导出成功", this);
    } else {
        new ToastMessage("数据导出失败", this);
    }
}

void MainWindow::analyzeData()
{
    QDateTime start = QDateTime::currentDateTime().addSecs(-300); // 最近5分钟
    QDateTime end = QDateTime::currentDateTime();
    
    auto analysis = DataManager::instance()->analyzeData(start, end);
    
    QString report = QString("数据分析报告\n"
                           "平均电流: %1A\n"
                           "平均电压: %2V\n"
                           "平均功率: %3W\n"
                           "最大电流: %4A\n"
                           "最大电压: %5V\n"
                           "最大功率: %6W")
        .arg(analysis.avgCurrent, 0, 'f', 3)
        .arg(analysis.avgVoltage, 0, 'f', 3)
        .arg(analysis.avgPower, 0, 'f', 3)
        .arg(analysis.maxCurrent, 0, 'f', 3)
        .arg(analysis.maxVoltage, 0, 'f', 3)
        .arg(analysis.maxPower, 0, 'f', 3);
        
    QMessageBox::information(this, "数据分析", report);
}
 */
