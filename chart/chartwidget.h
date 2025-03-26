#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include "util/datamanager.h"

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

    void clearChart();
    void updateChartData(const MeasurementData &data);

public slots:
    void updateChartDisplay(const QString &type);
    void exportData();

signals:
    void chartTypeChanged(const QString &type);

private:
    // UI组件
    QComboBox *m_chartTypeCombo;
    QPushButton *m_clearChartBtn;
    QPushButton *m_exportDataBtn;
    QTableWidget *m_dataTable;
    
    // 图表相关
    QChart *m_chart;
    QChartView *m_chartView;
    
    // 数据系列
    QLineSeries *m_currentSeries;
    QLineSeries *m_voltageSeries;
    QLineSeries *m_powerSeries;
    QLineSeries *m_resistanceSeries;
    QLineSeries *m_illuminanceSeries;
    QLineSeries *m_colorTempSeries;
    QLineSeries *m_rSeries;
    QLineSeries *m_gSeries;
    QLineSeries *m_bSeries;
    
    // 数据存储
    QVector<MeasurementData> m_measurementData;

    void setupUI();
    void setupConnections();
};

#endif // CHARTWIDGET_H 