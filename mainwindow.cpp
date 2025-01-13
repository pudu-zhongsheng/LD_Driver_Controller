#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "util/ToastMessage.h"
#include "util/config.h"
#include "util/logger.h"
#include "util/errorhandler.h"
#include "communication/eleload_itplus.h"
#include "communication/cl_twozerozeroacom.h"
#include <QFileDialog>
#include <QMessageBox>
#include "util/datamanager.h"

MainWindow::MainWindow(const QString &driver, const QString &load, const QString &meter, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_driverType(driver)
    , m_loadType(load)
    , m_meterType(meter)
    , m_dataTimer(new QTimer(this))
{
    ui->setupUi(this);
    setWindowTitle("LD Driver Controller");
    
    // 初始化驱动区域容器
    m_driverArea = new QStackedWidget(this);
    
    // 根据选择的型号创建对应的设备对象
    if (m_loadType == "IT8512+") {
        auto *protocol = new EleLoad_ITPlus(0x00, this);
        m_loadWidget = new IT8512Plus_Widget(protocol, this);
        
        // 连接所有信号
        connect(m_loadWidget, &LoadBase::serialConnected,
                this, &MainWindow::onLoadSerialConnected);
        connect(m_loadWidget, &LoadBase::serialDisconnected,
                this, &MainWindow::onLoadSerialDisconnected);
        connect(m_loadWidget, &LoadBase::serialError,
                this, &MainWindow::onLoadSerialError);
        connect(m_loadWidget, &LoadBase::statusUpdated,
                this, &MainWindow::updateLoadStatus);
    }
    
    // 根据选择的照度计型号创建对应的对象
    if (meter == "CL-200A") {
        m_meterWidget = new CL200AWidget(this);
        
        // 连接信号
        connect(m_meterWidget, &MeterBase::serialConnected,
                this, &MainWindow::onMeterSerialConnected);
        connect(m_meterWidget, &MeterBase::serialDisconnected,
                this, &MainWindow::onMeterSerialDisconnected);
        connect(m_meterWidget, &MeterBase::measurementUpdated,
                this, &MainWindow::onMeterDataUpdated);
    }
    
    // 根据选择的驱动型号创建对应的对象
    if (driver == "8CH") {
        m_driverWidget = new Driver8CH(this);
        
        // 连接信号
        connect(m_driverWidget, &DriverBase::serialConnected,
                this, &MainWindow::onDriverSerialConnected);
        connect(m_driverWidget, &DriverBase::serialDisconnected,
                this, &MainWindow::onDriverSerialDisconnected);
        connect(m_driverWidget, &DriverBase::serialError,
                this, &MainWindow::onDriverSerialError);
    }
    
    initUI();
    initConnections();
    setupStyles();
}

MainWindow::~MainWindow()
{
    disconnectAllPorts();
    delete ui;
}

void MainWindow::initUI()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主布局
    auto *mainLayout = new QVBoxLayout(m_centralWidget);
    auto *topLayout = new QHBoxLayout();
    auto *bottomLayout = new QHBoxLayout();
    
    // 创建各个区域
    createDriverArea();           // 左上角驱动控制区域
    createConnectionArea();       // 左下角通讯连接状态管理
    createLoadStatusArea();       // 电子负载状态区域
    createSchemeArea();          // 软件设置方案区域
    createChartArea();           // 右侧图表区域
    
    // 组装布局
    topLayout->addWidget(m_driverArea, 3);
    topLayout->addWidget(m_chartWidget, 2);
    
    bottomLayout->addWidget(m_connectionGroup, 3);
    bottomLayout->addWidget(m_loadStatusGroup, 2);
    bottomLayout->addWidget(m_schemeGroup, 1);
    bottomLayout->addStretch(5);
    
    // 返回按钮
    m_backButton = new QPushButton("返回", this);
    m_backButton->setFixedSize(100, 35);
    bottomLayout->addWidget(m_backButton, 0, Qt::AlignRight);
    
    mainLayout->addLayout(topLayout, 4);
    mainLayout->addLayout(bottomLayout, 1);
}

// 创建通讯连接状态管理区域
/*
通讯连接状态管理区域：
显示三个设备的串口连接状态
提供串口选择和连接/断开功能
自动更新可用串口列表
连接状态提示
*/

void MainWindow::createConnectionArea()
{
    m_connectionGroup = new QGroupBox("通讯连接状态", this);
    auto *layout = new QHBoxLayout(m_connectionGroup);
    layout->setSpacing(20); // 设置布局间距

    // 驱动连接状态
    auto *driverConnBox = new QGroupBox("驱动连接", this);
    auto *driverLayout = new QVBoxLayout(driverConnBox);
    auto *driverPortCombo = new QComboBox(this);
    auto *driverConnBtn = new QPushButton("连接", this);
    driverConnBtn->setObjectName("driverConnBtn");
    driverConnBtn->setCheckable(true);
    driverLayout->addWidget(driverPortCombo);
    driverLayout->addWidget(driverConnBtn);

    // 电子负载连接状态
    auto *loadConnBox = new QGroupBox("电子负载连接", this);
    auto *loadLayout = new QVBoxLayout(loadConnBox);
    auto *loadPortCombo = new QComboBox(this);
    auto *loadConnBtn = new QPushButton("连接", this);
    loadConnBtn->setObjectName("loadConnBtn");
    loadConnBtn->setCheckable(true);
    loadLayout->addWidget(loadPortCombo);
    loadLayout->addWidget(loadConnBtn);

    // 照度计连接状态
    auto *meterConnBox = new QGroupBox("照度计连接", this);
    auto *meterLayout = new QVBoxLayout(meterConnBox);
    auto *meterPortCombo = new QComboBox(this);
    auto *meterConnBtn = new QPushButton("连接", this);
    meterConnBtn->setObjectName("meterConnBtn");
    meterConnBtn->setCheckable(true);
    meterLayout->addWidget(meterPortCombo);
    meterLayout->addWidget(meterConnBtn);

    layout->addWidget(driverConnBox);
    layout->addWidget(loadConnBox);
    layout->addWidget(meterConnBox);

    // 连接按钮点击事件
    connect(driverConnBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            if (m_driverWidget) {
                m_driverWidget->connectToPort(driverPortCombo->currentText());
            }
        } else {
            if (m_driverWidget) {
                m_driverWidget->disconnectPort();
            }
        }
    });

    // 电子负载连接按钮处理
    connect(loadConnBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            // 只发送连接请求
            if (m_loadWidget) {
                loadConnBtn->setEnabled(false); // 禁用按钮,等待连接结果
                m_loadWidget->connectToPort(loadPortCombo->currentText());
            }
        } else {
            if (m_loadWidget) {
                m_loadWidget->disconnectPort();
            }
        }
    });

    connect(meterConnBtn, &QPushButton::clicked, this, [=](bool checked) {
        if (checked) {
            if (m_meterWidget) {
                m_meterWidget->connectToPort(meterPortCombo->currentText());
            }
        } else {
            if (m_meterWidget) {
                m_meterWidget->disconnectPort();
            }
        }
    });

    // 定时更新可用串口列表
    auto *portUpdateTimer = new QTimer(this);
    connect(portUpdateTimer, &QTimer::timeout, this, [=]() {
        // 获取当前选中的串口
        QString currentDriverPort = driverPortCombo->currentText();
        QString currentLoadPort = loadPortCombo->currentText();
        QString currentMeterPort = meterPortCombo->currentText();
        
        // 获取可用串口列表
        QList<QSerialPortInfo> ports = SerialUtil::getAvailablePorts();
        
        // 更新驱动串口列表
        bool driverConnected = false;
        if (m_driverType == "8CH") {
            driverConnected = m_driverWidget && m_driverWidget->isConnected();
        }
        if (!driverConnected) {
            driverPortCombo->clear();
            for (const auto &port : ports) {
                driverPortCombo->addItem(port.portName());
            }
            // 如果之前选中的串口仍然可用，则保持选中
            int index = driverPortCombo->findText(currentDriverPort);
            if (index >= 0) {
                driverPortCombo->setCurrentIndex(index);
            }
        }
        
        // 更新电子负载串口列表
        bool loadConnected = false;
        if (m_loadType == "IT8512+") {
            loadConnected = m_loadWidget && m_loadWidget->isConnected();
        }
        if (!loadConnected) {
            loadPortCombo->clear();
            for (const auto &port : ports) {
                loadPortCombo->addItem(port.portName());
            }
            int index = loadPortCombo->findText(currentLoadPort);
            if (index >= 0) {
                loadPortCombo->setCurrentIndex(index);
            }
        }
        
        // 更新照度计串口列表
        bool meterConnected = false;
        if (m_meterType == "CL-200A") {
            meterConnected = m_meterWidget && m_meterWidget->isConnected();
        }
        if (!meterConnected) {
            meterPortCombo->clear();
            for (const auto &port : ports) {
                meterPortCombo->addItem(port.portName());
            }
            int index = meterPortCombo->findText(currentMeterPort);
            if (index >= 0) {
                meterPortCombo->setCurrentIndex(index);
            }
        }
    });

    // 启动定时器，每秒更新一次
    portUpdateTimer->start(1000);
}

// 创建电子负载状态区域
/*
实时显示电压、电流、功率值
提供跳转到电子负载设置页面的按钮
美观的数值显示样式
*/
void MainWindow::createLoadStatusArea()
{
    m_loadStatusGroup = new QGroupBox("电子负载状态", this);
    auto *layout = new QVBoxLayout(m_loadStatusGroup);
    layout->setSpacing(15);

    // 电压显示
    auto *voltageLayout = new QHBoxLayout();
    auto *voltageLabel = new QLabel("电压:", this);
    auto *voltageValue = new QLabel("0.000 V", this);
    voltageValue->setObjectName("voltageValue");
    voltageLayout->addWidget(voltageLabel);
    voltageLayout->addWidget(voltageValue);

    // 电流显示
    auto *currentLayout = new QHBoxLayout();
    auto *currentLabel = new QLabel("电流:", this);
    auto *currentValue = new QLabel("0.000 A", this);
    currentValue->setObjectName("currentValue");
    currentLayout->addWidget(currentLabel);
    currentLayout->addWidget(currentValue);

    // 功率显示
    auto *powerLayout = new QHBoxLayout();
    auto *powerLabel = new QLabel("功率:", this);
    auto *powerValue = new QLabel("0.000 W", this);
    powerValue->setObjectName("powerValue");
    powerLayout->addWidget(powerLabel);
    powerLayout->addWidget(powerValue);

    // 跳转按钮
    auto *jumpButton = new QPushButton("电子负载设置", this);
    
    // 修改这里，添加按钮点击事件处理
    connect(jumpButton, &QPushButton::clicked, this, [=]() {
        bool loadConnected = false;
        if (m_loadType == "IT8512+") {
            loadConnected = m_loadWidget && m_loadWidget->isConnected();
        }
        
        if (!loadConnected) {
            ToastMessage *toast = new ToastMessage("请先连接电子负载", this);
            toast->showToast(1000);
            return;
        }

        if (m_loadType == "IT8512+") {
            // 创建新窗口
            QDialog *dialog = new QDialog(this);
            dialog->setWindowTitle("IT8512+ 电子负载设置");
            dialog->setMinimumSize(800, 600);
            
            // 创建布局
            auto *layout = new QVBoxLayout(dialog);
            
            // 将电子负载控件从当前父对象移除并添加到对话框
            if (m_loadWidget) {
                m_loadWidget->setParent(nullptr);  // 先断开原有的父子关系
                layout->addWidget(m_loadWidget);
                m_loadWidget->show();  // 确保控件可见
            }
            
            // 处理对话框关闭事件
            connect(dialog, &QDialog::finished, this, [this, dialog, layout]() {
                if (m_loadWidget) {
                    layout->removeWidget(m_loadWidget);  // 使用局部 layout 而不是 layout()
                    m_loadWidget->setParent(this);
                    m_loadWidget->hide();
                }
                dialog->deleteLater();
            });
            
            // 错误处理
            connect(m_loadWidget, &LoadBase::serialError, this, [=](const QString &msg) {
                ToastMessage *toast = new ToastMessage(msg, this);
                toast->showToast(1000);
            });
            
            dialog->show();
        } else {
            ToastMessage *toast = new ToastMessage("不支持的电子负载型号", this);
            toast->showToast(1000);
        }
    });

    layout->addLayout(voltageLayout);
    layout->addLayout(currentLayout);
    layout->addLayout(powerLayout);
    layout->addWidget(jumpButton);

    // 设置数值标签样式
    QString valueStyle = "QLabel { font-size: 16px; font-weight: bold; color: #2c3e50; }";
    voltageValue->setStyleSheet(valueStyle);
    currentValue->setStyleSheet(valueStyle);
    powerValue->setStyleSheet(valueStyle);

    // 更新数值的槽函数
    connect(this, &MainWindow::updateLoadStatus, this, [=](float voltage, float current, float power) {
        voltageValue->setText(QString::number(voltage, 'f', 3) + " V");
        currentValue->setText(QString::number(current, 'f', 3) + " A");
        powerValue->setText(QString::number(power, 'f', 3) + " W");
    });
}

// 创建软件设置方案区域
/*
方案选择下拉框
加载和保存按钮
自动扫描schemes目录下的方案文件
JSON格式保存和加载方案数据
用户友好的交互提示
*/
void MainWindow::createSchemeArea()
{
    m_schemeGroup = new QGroupBox("软件设置方案", this);
    auto *layout = new QVBoxLayout(m_schemeGroup);
    layout->setSpacing(10);

    // 方案选择下拉框
    auto *schemeLayout = new QHBoxLayout();
    auto *schemeLabel = new QLabel("选择方案:", this);
    auto *schemeCombo = new QComboBox(this);
    schemeCombo->setObjectName("schemeCombo");
    schemeLayout->addWidget(schemeLabel);
    schemeLayout->addWidget(schemeCombo);

    // 按钮布局
    auto *buttonLayout = new QHBoxLayout();
    auto *loadButton = new QPushButton("加载方案", this);
    auto *saveButton = new QPushButton("保存方案", this);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);

    layout->addLayout(schemeLayout);
    layout->addLayout(buttonLayout);

    // 更新方案列表
    auto updateSchemeList = [=]() {
        QString schemePath = QApplication::applicationDirPath() + "/schemes";
        QDir dir(schemePath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        schemeCombo->clear();
        schemeCombo->addItem("请选择方案...");
        
        QStringList filters;
        filters << "*.json";
        dir.setNameFilters(filters);
        
        for (const QString &file : dir.entryList(QDir::Files)) {
            schemeCombo->addItem(file.section('.', 0, 0));
        }
    };

    // 初始加载方案列表
    updateSchemeList();

    // 加载方案
    connect(loadButton, &QPushButton::clicked, this, [=]() {
        QString schemeName = schemeCombo->currentText();
        if (schemeName == "请选择方案...") {
            ToastMessage *toast = new ToastMessage("请先选择一个方案", this);
            toast->showToast(1000);
            return;
        }
        loadScheme(schemeName);
    });

    // 保存方案
    connect(saveButton, &QPushButton::clicked, this, [=]() {
        bool ok;
        QString schemeName = QInputDialog::getText(this, "保存方案",
                                                 "请输入方案名称:", QLineEdit::Normal,
                                                 "", &ok);
        if (ok && !schemeName.isEmpty()) {
            saveScheme(schemeName);
            updateSchemeList();
            schemeCombo->setCurrentText(schemeName);
        }
    });
}

// 保存方案
void MainWindow::saveScheme(const QString &name)
{
    QString schemePath = QApplication::applicationDirPath() + "/schemes/" + name + ".json";
    QFile file(schemePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        ToastMessage *toast = new ToastMessage("保存方案失败", this);
        toast->showToast(1000);
        return;
    }

    QJsonObject schemeData;
    
    // 保存电子负载设置
    if (m_loadWidget) {
        schemeData["load_settings"] = m_loadWidget->saveSettings();
    }
    
    // 保存图表设置
    QJsonObject chartSettings;
    chartSettings["type"] = m_chartTypeCombo->currentText();
    schemeData["chart"] = chartSettings;
    
    QJsonDocument doc(schemeData);
    file.write(doc.toJson());
    file.close();

    ToastMessage *toast = new ToastMessage("方案保存成功", this);
    toast->showToast(1000);
}

// 加载方案
void MainWindow::loadScheme(const QString &name)
{
    QString schemePath = QApplication::applicationDirPath() + "/schemes/" + name + ".json";
    QFile file(schemePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        ToastMessage *toast = new ToastMessage("加载方案失败", this);
        toast->showToast(1000);
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        ToastMessage *toast = new ToastMessage("方案文件格式错误", this);
        toast->showToast(1000);
        return;
    }

    QJsonObject schemeData = doc.object();
    
    // 加载电子负载设置
    if (m_loadWidget && schemeData.contains("load_settings")) {
        m_loadWidget->loadSettings(schemeData["load_settings"].toObject());
    }
    
    // 加载图表设置
    if (schemeData.contains("chart")) {
        QJsonObject chartSettings = schemeData["chart"].toObject();
        m_chartTypeCombo->setCurrentText(chartSettings["type"].toString());
    }

    ToastMessage *toast = new ToastMessage("方案加载成功", this);
    toast->showToast(1000);
}

/*
支持4种图表类型切换
实时数据显示
图表清空功能
数据导出功能（CSV格式）
*/
void MainWindow::createChartArea()
{
    auto *chartGroup = new QGroupBox("数据图表", this);
    auto *layout = new QVBoxLayout(chartGroup);

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
    layout->addLayout(controlLayout);

    // 创建图表
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(m_chartView);

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

    // 初始显示电流-时间图表
    updateChartDisplay("电流-时间");

    // 创建数据表格
    m_dataTable = new QTableWidget(this);
    m_dataTable->setColumnCount(10);
    m_dataTable->setHorizontalHeaderLabels({
        "时间", "电流(A)", "电压(V)", "功率(W)", "电阻(Ω)",
        "照度(lx)", "色温(K)", "R", "G", "B"
    });
    m_dataTable->hide(); // 默认隐藏，点击导出时显示

    m_chartWidget = chartGroup;
}

void MainWindow::updateChartDisplay(const QString &type)
{
    m_chart->removeAllSeries();
    
    // 设置X轴（时间）
    auto *axisX = new QDateTimeAxis;
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText("时间");
    m_chart->addAxis(axisX, Qt::AlignBottom);

    // 设置Y轴
    auto *axisY = new QValueAxis;
    
    if (type == "电流-时间") {
        m_chart->setTitle("电流-时间曲线");
        m_chart->addSeries(m_currentSeries);
        axisY->setTitleText("电流 (A)");
    }
    else if (type == "电压/功率/电阻-时间") {
        m_chart->setTitle("电压/功率/电阻-时间曲线");
        m_chart->addSeries(m_voltageSeries);
        m_chart->addSeries(m_powerSeries);
        m_chart->addSeries(m_resistanceSeries);
        axisY->setTitleText("数值");
    }
    else if (type == "照度-时间") {
        m_chart->setTitle("照度-时间曲线");
        m_chart->addSeries(m_illuminanceSeries);
        m_chart->addSeries(m_colorTempSeries);
        axisY->setTitleText("照度 (lx)");
    }
    else if (type == "色温RGB-时间") {
        m_chart->setTitle("RGB-时间曲线");
        m_chart->addSeries(m_rSeries);
        m_chart->addSeries(m_gSeries);
        m_chart->addSeries(m_bSeries);
        axisY->setTitleText("RGB值");
    }

    m_chart->addAxis(axisY, Qt::AlignLeft);

    // 将所有系列附加到坐标轴
    const auto series = m_chart->series();
    for (auto *s : series) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }
}

void MainWindow::clearChart()
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

void MainWindow::exportData()
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

void MainWindow::onBackButtonClicked()
{
    // 断开所有连接
    disconnectAllPorts();
    
    // 停止数据采集
    if (m_dataTimer) {
        m_dataTimer->stop();
    }
    
    // 清空数据
    m_measurementData.clear();
    
    // 发送返回信号
    emit backToMenu();
}

void MainWindow::initConnections()
{
    // ... 其他连接 ...

    // 图表相关连接
    connect(m_chartTypeCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::updateChartDisplay);
    connect(m_clearChartBtn, &QPushButton::clicked,
            this, &MainWindow::clearChart);
    connect(m_exportDataBtn, &QPushButton::clicked,
            this, &MainWindow::exportData);
    
    // 返回按钮
    connect(m_backButton, &QPushButton::clicked,
            this, &MainWindow::onBackButtonClicked);
}

void MainWindow::startDataCollection()
{
    // 创建数据采集定时器
    m_dataTimer = new QTimer(this);
    
    // 从配置文件读取采集间隔
    int interval = Config::getValue(ConfigKeys::UI_CHART_INTERVAL, 100).toInt();
    m_dataTimer->setInterval(interval);

    connect(m_dataTimer, &QTimer::timeout, this, [this]() {
        // 采集照度计数据
        // 照度计数据采集已经由 CL200AWidget 类通过信号 measurementUpdated 处理
    });

    // 启动定时器
    m_dataTimer->start();
    LOG_INFO("数据采集已启动");
}

void MainWindow::updateChartData(const MeasurementData &data)
{
    // 添加数据到存储
    m_measurementData.append(data);
    
    // 限制数据点数量（保留最近1小时的数据）
    while (m_measurementData.size() > 36000) { // 100ms * 36000 = 1小时
        m_measurementData.removeFirst();
    }

    // 更新图表
    qint64 timestamp = data.timestamp.toMSecsSinceEpoch();
    
    m_currentSeries->append(timestamp, data.current);
    m_voltageSeries->append(timestamp, data.voltage);
    m_powerSeries->append(timestamp, data.power);
    m_resistanceSeries->append(timestamp, data.resistance);
    m_illuminanceSeries->append(timestamp, data.illuminance);
    m_colorTempSeries->append(timestamp, data.colorTemp);
    m_rSeries->append(timestamp, data.r);
    m_gSeries->append(timestamp, data.g);
    m_bSeries->append(timestamp, data.b);

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
        // ... 其他图表类型的范围设置
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

void MainWindow::setupDataManagement()
{
    // 在工具栏或菜单栏添加数据管理按钮
    auto *dataMenu = menuBar()->addMenu("数据管理");
    
    auto *exportAction = new QAction("导出数据", this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportData);
    dataMenu->addAction(exportAction);
    
    auto *analyzeAction = new QAction("分析数据", this);
    connect(analyzeAction, &QAction::triggered, this, &MainWindow::onAnalyzeData);
    dataMenu->addAction(analyzeAction);
    
    dataMenu->addSeparator();
    
    auto *backupAction = new QAction("备份数据", this);
    connect(backupAction, &QAction::triggered, this, &MainWindow::onBackupData);
    dataMenu->addAction(backupAction);
    
    auto *restoreAction = new QAction("恢复数据", this);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreData);
    dataMenu->addAction(restoreAction);

    // 连接数据管理器信号
    connect(DataManager::instance(), &DataManager::dataAdded,
            this, &MainWindow::onDataAdded);
    connect(DataManager::instance(), &DataManager::dataCleared,
            this, &MainWindow::clearChart);
}

void MainWindow::onExportData()
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
        ToastMessage *toast = new ToastMessage("数据导出成功", this);
        toast->showToast(1000);
    } else {
        ToastMessage *toast = new ToastMessage("数据导出失败", this);
        toast->showToast(1000);
    }
}

void MainWindow::onAnalyzeData()
{
    // 创建分析对话框
    QDialog dialog(this);
    dialog.setWindowTitle("数据分析");
    auto *layout = new QVBoxLayout(&dialog);
    
    // 时间范围选择
    auto *timeGroup = new QGroupBox("时间范围", &dialog);
    auto *timeLayout = new QHBoxLayout(timeGroup);
    auto *startTime = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-300), &dialog);
    auto *endTime = new QDateTimeEdit(QDateTime::currentDateTime(), &dialog);
    timeLayout->addWidget(new QLabel("开始时间:"));
    timeLayout->addWidget(startTime);
    timeLayout->addWidget(new QLabel("结束时间:"));
    timeLayout->addWidget(endTime);
    layout->addWidget(timeGroup);
    
    // 分析按钮
    auto *analyzeBtn = new QPushButton("分析", &dialog);
    layout->addWidget(analyzeBtn);
    
    // 结果显示
    auto *resultText = new QTextEdit(&dialog);
    resultText->setReadOnly(true);
    layout->addWidget(resultText);
    
    // 连接分析按钮
    connect(analyzeBtn, &QPushButton::clicked, [=]() {
        auto analysis = DataManager::instance()->analyzeData(
            startTime->dateTime(), endTime->dateTime());
            
        QString report = QString(
            "数据分析报告\n"
            "时间范围: %1 至 %2\n\n"
            "电流分析:\n"
            "  平均值: %3 A\n"
            "  最大值: %4 A\n"
            "  最小值: %5 A\n\n"
            "电压分析:\n"
            "  平均值: %6 V\n"
            "  最大值: %7 V\n"
            "  最小值: %8 V\n\n"
            "功率分析:\n"
            "  平均值: %9 W\n"
            "  最大值: %10 W\n"
            "  最小值: %11 W\n")
            .arg(startTime->dateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(endTime->dateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(analysis.avgCurrent, 0, 'f', 3)
            .arg(analysis.maxCurrent, 0, 'f', 3)
            .arg(analysis.minCurrent, 0, 'f', 3)
            .arg(analysis.avgVoltage, 0, 'f', 3)
            .arg(analysis.maxVoltage, 0, 'f', 3)
            .arg(analysis.minVoltage, 0, 'f', 3)
            .arg(analysis.avgPower, 0, 'f', 3)
            .arg(analysis.maxPower, 0, 'f', 3)
            .arg(analysis.minPower, 0, 'f', 3);
            
        resultText->setText(report);
    });
    
    dialog.resize(600, 400);
    dialog.exec();
}

void MainWindow::onBackupData()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "备份数据",
        QDir::homePath(),
        "备份文件 (*.bak)"
    );
    
    if (filename.isEmpty()) return;
    
    if (DataManager::instance()->backup(filename)) {
        ToastMessage *toast = new ToastMessage("数据备份成功", this);
        toast->showToast(1000);
    } else {
        ToastMessage *toast = new ToastMessage("数据备份失败", this);
        toast->showToast(1000);
    }
}

void MainWindow::onRestoreData()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "恢复数据",
        QDir::homePath(),
        "备份文件 (*.bak)"
    );
    
    if (filename.isEmpty()) return;
    
    if (DataManager::instance()->restore(filename)) {
        ToastMessage *toast = new ToastMessage("数据恢复成功", this);
        toast->showToast(1000);
    } else {
        ToastMessage *toast = new ToastMessage("数据恢复失败", this);
        toast->showToast(1000);
    }
}

void MainWindow::onDataAdded(const MeasurementData &data)
{
    // 更新图表
    updateChartData(data);
    
    // 更新状态栏
    QString status = QString("最新数据 - 电流: %1A  电压: %2V  功率: %3W  照度: %4lx")
        .arg(data.current, 0, 'f', 3)
        .arg(data.voltage, 0, 'f', 3)
        .arg(data.power, 0, 'f', 3)
        .arg(data.illuminance, 0, 'f', 1);
    statusBar()->showMessage(status, 2000);
}

void MainWindow::setupStyles()
{
    // 加载样式表
    QFile file(":/styles/style.qss");
    if (file.open(QFile::ReadOnly)) {
        setStyleSheet(file.readAll());
        file.close();
    }
}

void MainWindow::initSerialPorts()
{
    // 初始化串口设置
    updateConnectionStatus();
}

void MainWindow::disconnectAllPorts()
{
    // 断开驱动串口
    if (m_driverWidget) {
        m_driverWidget->disconnectPort();
    }

    // 断开电子负载
    if (m_loadWidget) {
        m_loadWidget->disconnectPort();
    }

    // 断开照度计
    if (m_meterWidget) {
        m_meterWidget->disconnectPort();
    }
}

void MainWindow::createDriverArea()
{
    // 创建驱动控制区域
    auto *driverLayout = new QVBoxLayout();
    driverLayout->setSpacing(20);

    // 根据选择的驱动型号创建对应的控制界面
    if (m_driverType == "8CH") {
        // 如果已经存在驱动对象，先删除
        if (m_driverWidget) {
            m_driverWidget->deleteLater();
        }

        // 创建8通道驱动对象
        m_driverWidget = new Driver8CH(this);
        
        // 连接信号
        connect(m_driverWidget, &DriverBase::serialConnected,
                this, &MainWindow::onDriverSerialConnected);
        connect(m_driverWidget, &DriverBase::serialDisconnected,
                this, &MainWindow::onDriverSerialDisconnected);
        connect(m_driverWidget, &DriverBase::serialError,
                this, &MainWindow::onDriverSerialError);

        // 加载上次的配置
        QVariant value = Config::getValue("driver_settings");
        QJsonObject lastSettings = value.toJsonObject();
        if (!lastSettings.isEmpty()) {
            m_driverWidget->applySettings(lastSettings);
        }

        // 添加到布局
        driverLayout->addWidget(m_driverWidget);
    } else {
        // 如果没有匹配的驱动类型，显示提示信息
        auto *label = new QLabel("不支持的驱动类型: " + m_driverType, this);
        label->setAlignment(Qt::AlignCenter);
        driverLayout->addWidget(label);
    }

    // 创建一个容器widget来包含布局
    auto *driverContainer = new QWidget(this);
    driverContainer->setLayout(driverLayout);
    driverContainer->setObjectName("driverContainer");
    driverContainer->setStyleSheet(R"(
        #driverContainer {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
        }
    )");

    // 添加到驱动区域
    m_driverArea->addWidget(driverContainer);
    m_driverArea->setCurrentWidget(driverContainer);

    // 保存配置
    connect(m_driverWidget, &DriverBase::settingsChanged, this, [this]() {
        if (auto *driver = qobject_cast<Driver8CH*>(m_driverWidget)) {
            QJsonObject settings = driver->getSettings();
            Config::setValue("driver_settings", settings);
        }
    });
}

void MainWindow::updateLoadStatus(float voltage, float current, float power)
{
    // 保存最新的测量值
    m_lastVoltage = voltage;
    m_lastCurrent = current;
    m_lastPower = power;
    
    // 更新状态显示
    if (m_loadStatusGroup) {
        if (auto *voltageLabel = m_loadStatusGroup->findChild<QLabel*>("voltageLabel")) {
            voltageLabel->setText(QString::number(voltage, 'f', 3) + " V");
        }
        if (auto *currentLabel = m_loadStatusGroup->findChild<QLabel*>("currentLabel")) {
            currentLabel->setText(QString::number(current, 'f', 3) + " A");
        }
        if (auto *powerLabel = m_loadStatusGroup->findChild<QLabel*>("powerLabel")) {
            powerLabel->setText(QString::number(power, 'f', 3) + " W");
        }
    }
}

void MainWindow::onSerialPortError(const QString &portName)
{
    ToastMessage *toast = new ToastMessage(QString("串口错误: %1").arg(portName), this);
    toast->showToast(1000);
}

void MainWindow::onChartTypeChanged(const QString &type)
{
    updateChartDisplay(type);
}

void MainWindow::updateConnectionStatus()
{
    // 更新驱动连接状态
    if (auto *driverConnBtn = m_connectionGroup->findChild<QPushButton*>("driverConnBtn")) {
        bool connected = false;
        if (m_driverType == "8CH") {
            connected = m_driverWidget && m_driverWidget->isConnected();
        }
        driverConnBtn->setText(connected ? "断开" : "连接");
        driverConnBtn->setChecked(connected);
        driverConnBtn->setStyleSheet(connected ? "background-color: #4CAF50;" : "");
    }

    // 更新电子负载连接状态
    if (auto *loadConnBtn = m_connectionGroup->findChild<QPushButton*>("loadConnBtn")) {
        bool connected = false;
        if (m_loadType == "IT8512+") {
            connected = m_loadWidget && m_loadWidget->isConnected();
        }
        loadConnBtn->setText(connected ? "断开" : "连接");
        loadConnBtn->setChecked(connected);
        loadConnBtn->setStyleSheet(connected ? "background-color: #4CAF50;" : "");
    }

    // 更新照度计连接状态
    if (auto *meterConnBtn = m_connectionGroup->findChild<QPushButton*>("meterConnBtn")) {
        bool connected = false;
        if (m_meterType == "CL-200A") {
            connected = m_meterWidget && m_meterWidget->isConnected();
        }
        meterConnBtn->setText(connected ? "断开" : "连接");
        meterConnBtn->setChecked(connected);
        meterConnBtn->setStyleSheet(connected ? "background-color: #4CAF50;" : "");
    }
}

void MainWindow::onLoadSerialConnected(const QString &portName)
{
    updateConnectionStatus();
    
    if (auto *loadConnBtn = m_connectionGroup->findChild<QPushButton*>("loadConnBtn")) {
        loadConnBtn->setEnabled(true);
        loadConnBtn->setChecked(true);
        loadConnBtn->setText("断开");
    }
    
    ToastMessage *toast = new ToastMessage("电子负载已连接到 " + portName, this);
    toast->showToast(1000);
}

void MainWindow::onLoadSerialDisconnected()
{
    updateConnectionStatus();
    
    if (auto *loadConnBtn = m_connectionGroup->findChild<QPushButton*>("loadConnBtn")) {
        loadConnBtn->setEnabled(true);
        loadConnBtn->setChecked(false);
        loadConnBtn->setText("连接");
    }
    
    ToastMessage *toast = new ToastMessage("电子负载已断开连接", this);
    toast->showToast(1000);
}

void MainWindow::onLoadSerialError(const QString &error)
{
    updateConnectionStatus();
    
    if (auto *loadConnBtn = m_connectionGroup->findChild<QPushButton*>("loadConnBtn")) {
        loadConnBtn->setEnabled(true);
        loadConnBtn->setChecked(false);
        loadConnBtn->setText("连接");
    }
    
    LOG_ERROR("电子负载错误: " + error);
    ToastMessage *toast = new ToastMessage("电子负载错误: " + error, this);
    toast->showToast(1000);
}

void MainWindow::onMeterSerialConnected(const QString &portName)
{
    updateConnectionStatus();
    ToastMessage *toast = new ToastMessage("照度计已连接到 " + portName, this);
    toast->showToast(1000);
}

void MainWindow::onMeterSerialDisconnected()
{
    updateConnectionStatus();
    
    if (auto *meterConnBtn = m_connectionGroup->findChild<QPushButton*>("meterConnBtn")) {
        meterConnBtn->setChecked(false);
        meterConnBtn->setText("连接");
    }
    
    ToastMessage *toast = new ToastMessage("照度计已断开连接", this);
    toast->showToast(1000);
}

void MainWindow::onMeterDataUpdated(float illuminance, float colorTemp, float r, float g, float b)
{
    // 更新测量数据
    MeasurementData measurementData;
    measurementData.timestamp = QDateTime::currentDateTime();
    measurementData.illuminance = illuminance;
    measurementData.colorTemp = colorTemp;
    measurementData.r = r;
    measurementData.g = g;
    measurementData.b = b;
    
    // 如果电子负载已连接，添加电子负载数据
    if (m_loadWidget && m_loadWidget->isConnected()) {
        measurementData.voltage = m_lastVoltage;
        measurementData.current = m_lastCurrent;
        measurementData.power = m_lastPower;
        measurementData.resistance = m_lastVoltage / m_lastCurrent;
    }
    
    updateChartData(measurementData);
}

void MainWindow::onDriverSerialConnected(const QString &portName)
{
    updateConnectionStatus();
    ToastMessage *toast = new ToastMessage("驱动已连接到 " + portName, this);
    toast->showToast(1000);
}

void MainWindow::onDriverSerialDisconnected()
{
    updateConnectionStatus();
    if (auto *driverConnBtn = m_connectionGroup->findChild<QPushButton*>("driverConnBtn")) {
        driverConnBtn->setChecked(false);
        driverConnBtn->setText("连接");
    }
    ToastMessage *toast = new ToastMessage("驱动已断开连接", this);
    toast->showToast(1000);
}

void MainWindow::onDriverSerialError(const QString &error)
{
    LOG_ERROR("驱动串口错误: " + error);
    ToastMessage *toast = new ToastMessage("驱动错误: " + error, this);
    toast->showToast(1000);
    updateConnectionStatus();
}