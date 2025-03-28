#include "chartwidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "util/config.h"
#include "util/logger.h"
#include "util/ToastMessage.h"

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_chart(nullptr)
    , m_chartView(nullptr)
{
    setupUI();
    setupConnections();

    // 设置大小策略，允许控件在两个方向上扩展
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 确保图表视图也能够扩展
    if (m_chartView) {
        m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

ChartWidget::~ChartWidget()
{
    // 清理图表资源
    if (m_chart) {
        // 断开系列与坐标轴连接
        for (auto *series : m_chart->series()) {
            const auto axes = m_chart->axes();
            for (auto *axis : axes) {
                series->detachAxis(axis);
            }
        }
        
        // 移除所有系列
        m_chart->removeAllSeries();
        
        // 移除所有坐标轴
        QList<QAbstractAxis*> axes = m_chart->axes();
        for (QAbstractAxis *axis : axes) {
            m_chart->removeAxis(axis);
        }
    }
}

void ChartWidget::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);     // 减小边距


    // 图表类型选择和控制按钮
    auto *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(new QLabel("图表类型:", this));
    m_chartTypeCombo = new QComboBox(this);
    m_chartTypeCombo->addItems({
        "电流-时间",
        "电压/功率/电阻-时间",
        "照度-时间",
        "色温RGB-时间"
    });
    controlLayout->addWidget(m_chartTypeCombo);

    m_clearChartBtn = new QPushButton("清空图表", this);
    m_exportDataBtn = new QPushButton("导出数据", this);
    controlLayout->addWidget(m_clearChartBtn);
    controlLayout->addWidget(m_exportDataBtn);
    mainLayout->addLayout(controlLayout);

    // 创建图表
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(m_chartView, 1);  // 添加拉伸因子

    // 创建数据系列
    m_currentSeries = new QLineSeries(this);
    m_voltageSeries = new QLineSeries(this);
    m_powerSeries = new QLineSeries(this);
    m_resistanceSeries = new QLineSeries(this);
    m_illuminanceSeries = new QLineSeries(this);
    m_colorTempSeries = new QLineSeries(this);
    m_rSeries = new QLineSeries(this);
    m_gSeries = new QLineSeries(this);
    m_bSeries = new QLineSeries(this);

    // 设置系列名称
    m_currentSeries->setName("电流 (A)");
    m_voltageSeries->setName("电压 (V)");
    m_powerSeries->setName("功率 (W)");
    m_resistanceSeries->setName("电阻 (Ω)");
    m_illuminanceSeries->setName("照度 (lx)");
    m_colorTempSeries->setName("色温 (K)");
    m_rSeries->setName("R");
    m_gSeries->setName("G");
    m_bSeries->setName("B");

    // 初始显示照度-时间图表
    updateChartDisplay("照度-时间");

    // 创建数据表格
    m_dataTable = new QTableWidget(this);
    m_dataTable->setColumnCount(10);
    m_dataTable->setHorizontalHeaderLabels({
        "时间", "电流(A)", "电压(V)", "功率(W)", "电阻(Ω)",
        "照度(lx)", "色温(K)", "R", "G", "B"
    });
    m_dataTable->hide(); // 默认隐藏，点击导出时显示
}

void ChartWidget::setupConnections()
{
    // 图表类型变更
    connect(m_chartTypeCombo, &QComboBox::currentTextChanged, 
            this, &ChartWidget::updateChartDisplay);
            
    // 清空图表按钮
    connect(m_clearChartBtn, &QPushButton::clicked,
            this, &ChartWidget::clearChart);
            
    // 导出数据按钮
    connect(m_exportDataBtn, &QPushButton::clicked,
            this, &ChartWidget::exportData);
}

void ChartWidget::updateChartDisplay(const QString &type)
{
    // 首先断开所有系列与坐标轴的连接
    auto series = m_chart->series();
    for(auto *s : series) {
        // 从所有坐标轴断开连接
        const auto axes= m_chart->axes();
        for(auto *axis : axes) {
            s->detachAxis(axis);
        }
    }

    // 移除所有系列
    m_chart->removeAllSeries();

    // 移除所有坐标轴
    for(QAbstractAxis *axis : m_chart->axes()) {
    m_chart->removeAxis(axis);
    }

    // 设置X轴（时间）
    auto *axisX = new QDateTimeAxis(this);
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText("时间");

    // 设置Y轴
    auto *axisY = new QValueAxis(this);
    
    if (type == "电流-时间") {
        m_chart->setTitle("电流-时间曲线");
        m_currentSeries = new QLineSeries(this);
        m_chart->addSeries(m_currentSeries);
        axisY->setTitleText("电流 (A)");
    }
    else if (type == "电压/功率/电阻-时间") {
        m_chart->setTitle("电压/功率/电阻-时间曲线");
        m_voltageSeries = new QLineSeries(this);
        m_powerSeries = new QLineSeries(this);
        m_resistanceSeries = new QLineSeries(this);
        m_chart->addSeries(m_voltageSeries);
        m_chart->addSeries(m_powerSeries);
        m_chart->addSeries(m_resistanceSeries);
        axisY->setTitleText("数值");
    }
    else if (type == "照度-时间") {
        m_chart->setTitle("照度-时间曲线");
        m_illuminanceSeries = new QLineSeries(this);
        m_colorTempSeries = new QLineSeries(this);
        m_chart->addSeries(m_illuminanceSeries);
        m_chart->addSeries(m_colorTempSeries);
        axisY->setTitleText("照度 (lx)");
    }
    else if (type == "色温RGB-时间") {
        m_chart->setTitle("RGB-时间曲线");
        m_rSeries = new QLineSeries(this);
        m_gSeries = new QLineSeries(this);
        m_bSeries = new QLineSeries(this);
        m_chart->addSeries(m_rSeries);
        m_chart->addSeries(m_gSeries);
        m_chart->addSeries(m_bSeries);
        axisY->setTitleText("RGB值");
    }

    // 添加坐标轴到图表
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);

    // 将所有系列附加到坐标轴
    series = m_chart->series();
    for (auto *s : series) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    // 设置默认范围
    QDateTime now = QDateTime::currentDateTime();
    axisX->setRange(now.addSecs(-300), now);

    if(type == "电流-时间") {
        double maxCurrent = Config::getValue(ConfigKeys::ELOAD_MAX_CURRENT, 5.0).toDouble();
        axisY->setRange(0, maxCurrent);
    }
    else if(type == "电压/功率/电阻-时间") {
        double maxVoltage = Config::getValue(ConfigKeys::ELOAD_MAX_VOLTAGE, 30.0).toDouble();
        axisY->setRange(0, maxVoltage);
    }
    else if(type == "照度-时间") {
        axisY->setRange(0, 10000);
    }
    else if(type == "色温RGB-时间") {
        axisY->setRange(0, 255);
    }
}

void ChartWidget::clearChart()
{
    m_measurementData.clear();
    m_currentSeries->clear();
    m_voltageSeries->clear();
    m_powerSeries->clear();
    m_resistanceSeries->clear();
    m_illuminanceSeries->clear();
    m_colorTempSeries->clear();
    m_rSeries->clear();
    m_gSeries->clear();
    m_bSeries->clear();
    m_dataTable->setRowCount(0);
}

void ChartWidget::exportData()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "导出数据", "", "CSV文件 (*.csv)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件进行写入");
        return;
    }

    QTextStream out(&file);
    // 写入表头
    out << "时间,电流(A),电压(V),功率(W),电阻(Ω),照度(lx),色温(K),R,G,B\n";

    // 写入数据
    for (const auto &data : m_measurementData) {
        out << data.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz") << ","
            << QString::number(data.current, 'f', 3) << ","
            << QString::number(data.voltage, 'f', 3) << ","
            << QString::number(data.power, 'f', 3) << ","
            << QString::number(data.resistance, 'f', 3) << ","
            << QString::number(data.illuminance, 'f', 3) << ","
            << QString::number(data.colorTemp, 'f', 3) << ","
            << QString::number(data.r, 'f', 3) << ","
            << QString::number(data.g, 'f', 3) << ","
            << QString::number(data.b, 'f', 3) << "\n";
    }

    file.close();
    ToastMessage *toast = new ToastMessage("数据导出成功", this);
    toast->showToast(1000);
}

void ChartWidget::updateChartData(const MeasurementData &data)
{
    // 添加数据到存储
    m_measurementData.append(data);
    
    // 限制数据点数量（保留最近1小时的数据）
    while (m_measurementData.size() > 36000) { // 100ms * 36000 = 1小时
        m_measurementData.removeFirst();
    }

    // 更新图表
    qint64 timestamp = data.timestamp.toMSecsSinceEpoch();
    
    // 添加照度和色温数据点
    m_currentSeries->append(timestamp, data.current);
    m_voltageSeries->append(timestamp, data.voltage);
    m_powerSeries->append(timestamp, data.power);
    m_resistanceSeries->append(timestamp, data.resistance);
    if(data.illuminance != 0){
        m_illuminanceSeries->append(timestamp, data.illuminance);
    }
    if(data.r != 0 && data.g != 0 && data.b != 0){
        m_rSeries->append(timestamp, data.r);
        m_gSeries->append(timestamp, data.g);
        m_bSeries->append(timestamp, data.b);
    }

    // 更新X轴范围（显示最近5分钟的数据）
    auto *axisX = qobject_cast<QDateTimeAxis*>(m_chart->axes(Qt::Horizontal).first());
    if (axisX) {
        QDateTime now = QDateTime::currentDateTime();
        axisX->setRange(now.addSecs(-300), now);
    }

    // 更新Y轴范围
    auto *axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    if (axisY) {
        // 根据当前显示的图表类型调整Y轴范围
        QString chartType = m_chartTypeCombo->currentText();
        if (chartType == "电流-时间") {
            double maxCurrent = Config::getValue(ConfigKeys::ELOAD_MAX_CURRENT, 5.0).toDouble();
            axisY->setRange(0, maxCurrent);
        }
        else if (chartType == "电压/功率/电阻-时间") {
            double maxVoltage = Config::getValue(ConfigKeys::ELOAD_MAX_VOLTAGE, 30.0).toDouble();
            axisY->setRange(0, maxVoltage);
        }
        else if (chartType == "照度-时间") {
            // 动态调整照度范围
            double maxIlluminance = 1000.0;  // 默认值
            for (const auto &item : m_measurementData) {
                if (item.illuminance > maxIlluminance) {
                    maxIlluminance = item.illuminance * 1.1; // 留一些余量
                }
            }
            axisY->setRange(0, maxIlluminance);
        }
        else if (chartType == "色温RGB-时间") {
            // 动态调整RGB范围
            axisY->setRange(0, 255);
        }
    }

    // 更新数据表格
    int row = m_dataTable->rowCount();
    m_dataTable->insertRow(row);
    m_dataTable->setItem(row, 0, new QTableWidgetItem(data.timestamp.toString("hh:mm:ss.zzz")));
    m_dataTable->setItem(row, 1, new QTableWidgetItem(QString::number(data.current, 'f', 3)));
    m_dataTable->setItem(row, 2, new QTableWidgetItem(QString::number(data.voltage, 'f', 3)));
    m_dataTable->setItem(row, 3, new QTableWidgetItem(QString::number(data.power, 'f', 3)));
    m_dataTable->setItem(row, 4, new QTableWidgetItem(QString::number(data.resistance, 'f', 3)));
    m_dataTable->setItem(row, 5, new QTableWidgetItem(QString::number(data.illuminance, 'f', 3)));
    m_dataTable->setItem(row, 6, new QTableWidgetItem(QString::number(data.colorTemp, 'f', 3)));
    m_dataTable->setItem(row, 7, new QTableWidgetItem(QString::number(data.r, 'f', 3)));
    m_dataTable->setItem(row, 8, new QTableWidgetItem(QString::number(data.g, 'f', 3)));
    m_dataTable->setItem(row, 9, new QTableWidgetItem(QString::number(data.b, 'f', 3)));

    // 限制表格行数
    while (m_dataTable->rowCount() > 1000) { // 保留最近1000条记录
        m_dataTable->removeRow(0);
    }

    // 自动滚动到最新数据
    m_dataTable->scrollToBottom();
} 
