#include "drivemenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

DriveMenu::DriveMenu(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("驱动选择");
    // 设置窗口固定大小
    setFixedSize(500, 400);
    
    initUI();
    initConnections();
    setupStyles();
}

DriveMenu::~DriveMenu()
{
}

void DriveMenu::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // 标题
    m_titleLabel = new QLabel("设备选择", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_titleLabel);

    // 创建设备选择组
    auto *deviceGroup = new QGroupBox(this);
    deviceGroup->setObjectName("deviceGroup");
    auto *deviceLayout = new QVBoxLayout(deviceGroup);
    deviceLayout->setSpacing(20);

    // 驱动选择
    auto *driverContainer = new QFrame(this);
    auto *driverLayout = new QHBoxLayout(driverContainer);
    driverLayout->setContentsMargins(0, 0, 0, 0);
    auto *driverLabel = new QLabel("驱动型号:", this);
    driverLabel->setFixedWidth(80);
    m_driverSelect = new QComboBox(this);
    m_driverSelect->addItems({"1CH", "2CH", "4CH", "5CH", "6CH", "8CH", "10CH", "20CH"});
    driverLayout->addWidget(driverLabel);
    driverLayout->addWidget(m_driverSelect);

    // 电子负载选择
    auto *loadContainer = new QFrame(this);
    auto *loadLayout = new QHBoxLayout(loadContainer);
    loadLayout->setContentsMargins(0, 0, 0, 0);
    auto *loadLabel = new QLabel("电子负载:", this);
    loadLabel->setFixedWidth(80);
    m_loadSelect = new QComboBox(this);
    m_loadSelect->addItems({"IT8512+", "其他型号"});
    loadLayout->addWidget(loadLabel);
    loadLayout->addWidget(m_loadSelect);

    // 照度计选择
    auto *meterContainer = new QFrame(this);
    auto *meterLayout = new QHBoxLayout(meterContainer);
    meterLayout->setContentsMargins(0, 0, 0, 0);
    auto *meterLabel = new QLabel("照度计:", this);
    meterLabel->setFixedWidth(80);
    m_meterSelect = new QComboBox(this);
    m_meterSelect->addItems({"CL-200A", "其他型号"});
    meterLayout->addWidget(meterLabel);
    meterLayout->addWidget(m_meterSelect);

    deviceLayout->addWidget(driverContainer);
    deviceLayout->addWidget(loadContainer);
    deviceLayout->addWidget(meterContainer);

    // 添加一些空白空间
    deviceLayout->addStretch();

    // 进入按钮
    m_enterButton = new QPushButton("进入主界面", this);
    m_enterButton->setObjectName("enterButton");
    m_enterButton->setFixedSize(200, 45);

    mainLayout->addWidget(deviceGroup);
    mainLayout->addWidget(m_enterButton, 0, Qt::AlignHCenter);
}

void DriveMenu::setupStyles()
{
    // 设置标题样式
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   color: #2c3e50;"
        "   font-weight: bold;"
        "   margin-bottom: 20px;"
        "}"
    );

    // 为设备选择组添加阴影效果
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    findChild<QGroupBox*>("deviceGroup")->setGraphicsEffect(shadow);

    // 设置窗口背景
    setStyleSheet(
        "DriveMenu {"
        "   background-color: #f5f6fa;"
        "}"
        "QGroupBox {"
        "   border: 1px solid #e1e1e1;"
        "   border-radius: 10px;"
        "   background-color: white;"
        "   padding: 20px;"
        "}"
    );
}

void DriveMenu::initConnections()
{
    // 按钮点击动画效果
    connect(m_enterButton, &QPushButton::pressed, this, [this]() {
        auto *animation = new QPropertyAnimation(m_enterButton, "geometry", this);
        animation->setDuration(100);
        QRect geometry = m_enterButton->geometry();
        animation->setStartValue(geometry);
        animation->setEndValue(QRect(geometry.x(), geometry.y() + 2, geometry.width(), geometry.height()));
        animation->start(QPropertyAnimation::DeleteWhenStopped);
    });

    connect(m_enterButton, &QPushButton::released, this, [this]() {
        auto *animation = new QPropertyAnimation(m_enterButton, "geometry", this);
        animation->setDuration(100);
        QRect geometry = m_enterButton->geometry();
        animation->setStartValue(geometry);
        animation->setEndValue(QRect(geometry.x(), geometry.y() - 2, geometry.width(), geometry.height()));
        animation->start(QPropertyAnimation::DeleteWhenStopped);

        // 在动画完成后发送信号
        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            emit enterMainWindow(
                m_driverSelect->currentText(),
                m_loadSelect->currentText(),
                m_meterSelect->currentText()
            );
        });
    });
} 