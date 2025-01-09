#include "datamanager.h"
#include "config.h"
#include <QDir>
#include <QTextStream>
#include <QApplication>
#include <algorithm>

DataManager* DataManager::m_instance = nullptr;

DataManager::DataManager(QObject *parent) : QObject(parent)
{
}

DataManager* DataManager::instance()
{
    if (!m_instance) {
        m_instance = new DataManager(qApp);
    }
    return m_instance;
}

void DataManager::addMeasurement(const MeasurementData &data)
{
    m_data.append(data);
    trimData();
    emit dataAdded(data);
}

void DataManager::clearData()
{
    m_data.clear();
    emit dataCleared();
}

QVector<MeasurementData> DataManager::getData(const QDateTime &start, const QDateTime &end) const
{
    QVector<MeasurementData> result;
    for (const auto &data : m_data) {
        if (data.timestamp >= start && data.timestamp <= end) {
            result.append(data);
        }
    }
    return result;
}

bool DataManager::exportToCSV(const QString &filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Config::LOG_ERROR("无法打开文件进行CSV导出: " + filename);
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // 写入表头
    stream << "时间戳,电流(A),电压(V),功率(W),电阻(Ω),照度(lx),色温(K),R,G,B\n";

    // 写入数据
    for (const auto &data : m_data) {
        stream << measurementToCSV(data) << "\n";
    }

    file.close();
    Config::LOG_INFO("数据成功导出到CSV: " + filename);
    return true;
}

bool DataManager::exportToJSON(const QString &filename) const
{
    QJsonArray array;
    for (const auto &data : m_data) {
        array.append(measurementToJSON(data));
    }

    QJsonDocument doc(array);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        Config::LOG_ERROR("无法打开文件进行JSON导出: " + filename);
        return false;
    }

    file.write(doc.toJson());
    file.close();
    Config::LOG_INFO("数据成功导出到JSON: " + filename);
    return true;
}

DataManager::DataAnalysis DataManager::analyzeData(const QDateTime &start, const QDateTime &end) const
{
    DataAnalysis analysis = {0};
    auto data = getData(start, end);
    if (data.isEmpty()) return analysis;

    // 计算平均值
    for (const auto &m : data) {
        analysis.avgCurrent += m.current;
        analysis.avgVoltage += m.voltage;
        analysis.avgPower += m.power;
    }
    analysis.avgCurrent /= data.size();
    analysis.avgVoltage /= data.size();
    analysis.avgPower /= data.size();

    // 计算最大最小值
    analysis.maxCurrent = analysis.minCurrent = data.first().current;
    analysis.maxVoltage = analysis.minVoltage = data.first().voltage;
    analysis.maxPower = analysis.minPower = data.first().power;

    for (const auto &m : data) {
        analysis.maxCurrent = std::max(analysis.maxCurrent, m.current);
        analysis.minCurrent = std::min(analysis.minCurrent, m.current);
        analysis.maxVoltage = std::max(analysis.maxVoltage, m.voltage);
        analysis.minVoltage = std::min(analysis.minVoltage, m.voltage);
        analysis.maxPower = std::max(analysis.maxPower, m.power);
        analysis.minPower = std::min(analysis.minPower, m.power);
    }

    return analysis;
}

bool DataManager::backup(const QString &filename) const
{
    bool success = exportToJSON(filename);
    emit backupCompleted(success);
    return success;
}

bool DataManager::restore(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        Config::LOG_ERROR("无法打开备份文件: " + filename);
        emit restoreCompleted(false);
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) {
        Config::LOG_ERROR("备份文件格式错误: " + filename);
        emit restoreCompleted(false);
        return false;
    }

    clearData();
    QJsonArray array = doc.array();
    for (const auto &value : array) {
        if (value.isObject()) {
            m_data.append(jsonToMeasurement(value.toObject()));
        }
    }

    Config::LOG_INFO("成功从备份文件恢复数据: " + filename);
    emit restoreCompleted(true);
    return true;
}

void DataManager::trimData()
{
    while (m_data.size() > MAX_DATA_POINTS) {
        m_data.removeFirst();
    }
}

QString DataManager::measurementToCSV(const MeasurementData &data) const
{
    return QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10")
        .arg(data.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(data.current, 0, 'f', 3)
        .arg(data.voltage, 0, 'f', 3)
        .arg(data.power, 0, 'f', 3)
        .arg(data.resistance, 0, 'f', 3)
        .arg(data.illuminance, 0, 'f', 3)
        .arg(data.colorTemp, 0, 'f', 3)
        .arg(data.r, 0, 'f', 3)
        .arg(data.g, 0, 'f', 3)
        .arg(data.b, 0, 'f', 3);
}

QJsonObject DataManager::measurementToJSON(const MeasurementData &data) const
{
    QJsonObject obj;
    obj["timestamp"] = data.timestamp.toString(Qt::ISODate);
    obj["current"] = data.current;
    obj["voltage"] = data.voltage;
    obj["power"] = data.power;
    obj["resistance"] = data.resistance;
    obj["illuminance"] = data.illuminance;
    obj["colorTemp"] = data.colorTemp;
    obj["r"] = data.r;
    obj["g"] = data.g;
    obj["b"] = data.b;
    return obj;
}

MeasurementData DataManager::jsonToMeasurement(const QJsonObject &json) const
{
    MeasurementData data;
    data.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    data.current = json["current"].toDouble();
    data.voltage = json["voltage"].toDouble();
    data.power = json["power"].toDouble();
    data.resistance = json["resistance"].toDouble();
    data.illuminance = json["illuminance"].toDouble();
    data.colorTemp = json["colorTemp"].toDouble();
    data.r = json["r"].toDouble();
    data.g = json["g"].toDouble();
    data.b = json["b"].toDouble();
    return data;
}