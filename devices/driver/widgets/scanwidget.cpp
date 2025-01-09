#include "scanwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>

ScanWidget::ScanWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnections();
}

void ScanWidget::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // 1. 扫描目标选择
    auto *targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel("扫描目标:", this));
    m_targetSelect = new QComboBox(this);
    m_targetSelect->addItems({"通道值一", "通道值二", "通道值三"});
    targetLayout->addWidget(m_targetSelect);
    targetLayout->addStretch();
    mainLayout->addLayout(targetLayout);

    // 2. 扫描参数设置
    auto *paramGroup = new QGroupBox("扫描参数", this);
    auto *paramLayout = new QGridLayout(paramGroup);
    
    // 步长值
    paramLayout->addWidget(new QLabel("步长值:", this), 0, 0);
    m_stepValue = new QSpinBox(this);
    m_stepValue->setRange(1, 255);
    paramLayout->addWidget(m_stepValue, 0, 1);
    
    // 步长间隔
    paramLayout->addWidget(new QLabel("间隔(ms):", this), 0, 2);
    m_stepInterval = new QSpinBox(this);
    m_stepInterval->setRange(100, 10000);
    m_stepInterval->setValue(1000);
    paramLayout->addWidget(m_stepInterval, 0, 3);
    
    // 终止值
    paramLayout->addWidget(new QLabel("终止值:", this), 1, 0);
    m_endValue = new QSpinBox(this);
    m_endValue->setRange(0, 255);
    m_endValue->setValue(255);
    paramLayout->addWidget(m_endValue, 1, 1);
    
    mainLayout->addWidget(paramGroup);

    // 3. 控制按钮
    auto *btnLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("开始扫描", this);
    m_stopBtn = new QPushButton("停止扫描", this);
    m_stopBtn->setEnabled(false);
    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_stopBtn);
    mainLayout->addLayout(btnLayout);

    // 创建定时器
    m_scanTimer = new QTimer(this);
}

void ScanWidget::initConnections()
{
    connect(m_startBtn, &QPushButton::clicked, this, &ScanWidget::startScan);
    connect(m_stopBtn, &QPushButton::clicked, this, &ScanWidget::stopScan);
    connect(m_scanTimer, &QTimer::timeout, this, &ScanWidget::onScanTimeout);
}

void ScanWidget::startScan()
{
    m_currentValue = 0;  // 从0开始扫描
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    
    // 设置定时器间隔并启动
    m_scanTimer->setInterval(m_stepInterval->value());
    m_scanTimer->start();
    
    // 发送初始值
    emit scanValueChanged(m_currentValue);
}

void ScanWidget::stopScan()
{
    m_scanTimer->stop();
    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
}

void ScanWidget::onScanTimeout()
{
    // 更新当前值
    m_currentValue += m_stepValue->value();
    
    // 检查是否达到终止值
    if (m_currentValue >= m_endValue->value()) {
        m_currentValue = m_endValue->value();
        stopScan();
    }
    
    // 发送当前值
    emit scanValueChanged(m_currentValue);
} 