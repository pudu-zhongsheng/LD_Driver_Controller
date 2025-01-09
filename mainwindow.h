#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>
#include "serial/serialutil.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QInputDialog>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QTableWidget>
#include "util/datamanager.h"
#include "devices/load/it8512plus/it8512plus_widget.h"
#include "devices/meter/meterbase.h"
#include "devices/meter/cl200a/cl200awidget.h"
#include "devices/driver/driverbase.h"
#include "devices/driver/driver8ch/driver8ch.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &driver, const QString &load, const QString &meter, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBackButtonClicked(); // 返回按钮点击事件
    void onSerialPortError(const QString &portName);
    void updateLoadStatus(float voltage, float current, float power);
    void onChartTypeChanged(const QString &type);
    void clearChart();
    void exportData();
    void onExportData();        // 导出数据
    void onAnalyzeData();       // 分析数据
    void onBackupData();        // 备份数据
    void onRestoreData();       // 恢复数据
    void onDataAdded(const MeasurementData &data);  // 数据添加时更新UI
    void onLoadSerialConnected(const QString &portName);
    void onLoadSerialDisconnected();
    void onLoadSerialError(const QString &error);
    void onMeterSerialConnected(const QString &portName);
    void onMeterSerialDisconnected();
    void onMeterDataUpdated(float illuminance, float colorTemp, float r, float g, float b);
    void onDriverSerialConnected(const QString &portName);
    void onDriverSerialDisconnected();
    void onDriverSerialError(const QString &error);

signals:
    void backToMenu();

private:
    Ui::MainWindow *ui;
    
    // 设备类型
    QString m_driverType;
    QString m_loadType;
    QString m_meterType;
    
    // UI组件
    QWidget *m_centralWidget;
    QStackedWidget *m_driverArea;    // 驱动控制区域
    QGroupBox *m_connectionGroup;      // 通讯连接状态管理
    QGroupBox *m_loadStatusGroup;      // 电子负载状态
    QGroupBox *m_schemeGroup;          // 软件设置方案
    QWidget *m_chartWidget;            // 图表区域
    QPushButton *m_backButton;         // 返回按钮
        
    // 添加图表相关成员
    QChart *m_chart;
    QChartView *m_chartView;
    QComboBox *m_chartTypeCombo;
    QTableWidget *m_dataTable;
    QPushButton *m_clearChartBtn;
    QPushButton *m_exportDataBtn;
    
    // 数据存储
    QVector<MeasurementData> m_measurementData;
    
    // 数据采集定时器
    QTimer *m_dataTimer;
    
    // 图表系列
    QLineSeries *m_currentSeries;
    QLineSeries *m_voltageSeries;
    QLineSeries *m_powerSeries;
    QLineSeries *m_resistanceSeries;
    QLineSeries *m_illuminanceSeries;
    QLineSeries *m_colorTempSeries;
    QLineSeries *m_rSeries;
    QLineSeries *m_gSeries;
    QLineSeries *m_bSeries;
    
    // 设备对象
    IT8512Plus_Widget *m_loadWidget = nullptr;    // 电子负载对象
    MeterBase *m_meterWidget = nullptr; // 照度计对象
    DriverBase *m_driverWidget = nullptr;         // 驱动对象
    
    // 最新的电子负载数据
    float m_lastVoltage = 0.0f;
    float m_lastCurrent = 0.0f;
    float m_lastPower = 0.0f;
    
    void initUI();
    void initConnections();
    void setupStyles();
    void initSerialPorts();
    
    // UI初始化辅助函数
    void createDriverArea();           // 创建驱动控制区域
    void createConnectionArea();       // 创建通讯连接状态管理区域
    void createLoadStatusArea();       // 创建电子负载状态区域
    void createSchemeArea();           // 创建软件设置方案区域
    void createChartArea();            // 创建图表区域
    
    // 数据处理相关
    void saveScheme(const QString &name);
    void loadScheme(const QString &name);
    void updateConnectionStatus();
    void disconnectAllPorts();
    void startDataCollection();

    // 图表相关
    void updateChartDisplay(const QString &type);
    void updateChartData(const MeasurementData &data);

    void setupDataManagement();  // 设置数据管理相关UI和连接
};

#endif // MAINWINDOW_H
