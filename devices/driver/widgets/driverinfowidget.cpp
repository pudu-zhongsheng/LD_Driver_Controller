#include "driverinfowidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

DriverInfoWidget::DriverInfoWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnections();
}

void DriverInfoWidget::initUI()
{
    auto *layout = new QGridLayout(this);
    layout->setSpacing(10);
    
    // 客户信息
    layout->addWidget(new QLabel("客户:", this), 0, 0);
    m_customerEdit = new QLineEdit(this);
    layout->addWidget(m_customerEdit, 0, 1);
    
    // 驱动编码
    layout->addWidget(new QLabel("驱动编码:", this), 0, 2);
    m_driverCodeEdit = new QLineEdit(this);
    layout->addWidget(m_driverCodeEdit, 0, 3);
    
    // 测试编码
    layout->addWidget(new QLabel("测试编码:", this), 1, 0);
    m_testCodeEdit = new QLineEdit(this);
    layout->addWidget(m_testCodeEdit, 1, 1);
}

void DriverInfoWidget::initConnections()
{
    // 连接编辑框信号
    connect(m_customerEdit, &QLineEdit::textChanged, this, &DriverInfoWidget::infoChanged);
    connect(m_driverCodeEdit, &QLineEdit::textChanged, this, &DriverInfoWidget::infoChanged);
    connect(m_testCodeEdit, &QLineEdit::textChanged, this, &DriverInfoWidget::infoChanged);
}

QString DriverInfoWidget::getCustomer() const
{
    return m_customerEdit->text();
}

QString DriverInfoWidget::getDriverCode() const
{
    return m_driverCodeEdit->text();
}

QString DriverInfoWidget::getTestCode() const
{
    return m_testCodeEdit->text();
}

void DriverInfoWidget::setCustomer(const QString &customer)
{
    m_customerEdit->setText(customer);
}

void DriverInfoWidget::setDriverCode(const QString &code)
{
    m_driverCodeEdit->setText(code);
}

void DriverInfoWidget::setTestCode(const QString &code)
{
    m_testCodeEdit->setText(code);
} 