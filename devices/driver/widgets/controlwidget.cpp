#include "controlwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>

ControlWidget::ControlWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnections();
}

void ControlWidget::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // 1. 电平控制区域
    auto *levelGroup = new QGroupBox("电平控制", this);
    auto *levelLayout = new QHBoxLayout(levelGroup);
    m_levelGroup = new QButtonGroup(this);
    
    auto *highLevel = new QRadioButton("高电平", this);
    auto *lowLevel = new QRadioButton("低电平", this);
    m_levelGroup->addButton(highLevel, 0);
    m_levelGroup->addButton(lowLevel, 1);
    levelLayout->addWidget(highLevel);
    levelLayout->addWidget(lowLevel);
    mainLayout->addWidget(levelGroup);

    // 2. 通道值设置区域
    auto *valueGroup = new QGroupBox("通道值设置", this);
    auto *valueLayout = new QHBoxLayout(valueGroup);
    
    // 通道值一
    auto *value1Layout = new QVBoxLayout();
    value1Layout->addWidget(new QLabel("通道值一:", this));
    m_channelValue1 = new QSpinBox(this);
    m_channelValue1->setRange(0, 255);
    value1Layout->addWidget(m_channelValue1);
    valueLayout->addLayout(value1Layout);

    // 通道值二
    auto *value2Layout = new QVBoxLayout();
    value2Layout->addWidget(new QLabel("通道值二:", this));
    m_channelValue2 = new QSpinBox(this);
    m_channelValue2->setRange(0, 255);
    value2Layout->addWidget(m_channelValue2);
    valueLayout->addLayout(value2Layout);

    // 通道值三
    auto *value3Layout = new QVBoxLayout();
    value3Layout->addWidget(new QLabel("通道值三:", this));
    m_channelValue3 = new QSpinBox(this);
    m_channelValue3->setRange(0, 255);
    value3Layout->addWidget(m_channelValue3);
    valueLayout->addLayout(value3Layout);

    mainLayout->addWidget(valueGroup);

    // 3. 寄存器管理区域
    auto *registerGroup = new QGroupBox("寄存器管理", this);
    auto *registerLayout = new QHBoxLayout(registerGroup);
    
    registerLayout->addWidget(new QLabel("起始寄存器:", this));
    m_startRegister = new QSpinBox(this);
    m_startRegister->setRange(1, 8);
    registerLayout->addWidget(m_startRegister);
    
    registerLayout->addWidget(new QLabel("寄存器数:", this));
    m_registerCount = new QSpinBox(this);
    m_registerCount->setRange(1, 8);
    registerLayout->addWidget(m_registerCount);
    
    mainLayout->addWidget(registerGroup);
}

void ControlWidget::initConnections()
{
    // 连接所有控件的信号
    connect(m_levelGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &ControlWidget::controlChanged);
            
    connect(m_channelValue1, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlWidget::controlChanged);
    connect(m_channelValue2, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlWidget::controlChanged);
    connect(m_channelValue3, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlWidget::controlChanged);
            
    connect(m_startRegister, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlWidget::updateRegisterCountRange);
    connect(m_registerCount, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlWidget::controlChanged);
}

void ControlWidget::updateRegisterCountRange()
{
    // 根据起始寄存器更新寄存器数的范围
    int maxCount = 9 - m_startRegister->value();  // 8通道，最大值为8
    m_registerCount->setRange(1, maxCount);
    emit controlChanged();
}

bool ControlWidget::isHighLevel() const
{
    return m_levelGroup->checkedId() == 0;
}

int ControlWidget::getStartRegister() const
{
    return m_startRegister->value();
}

int ControlWidget::getRegisterCount() const
{
    return m_registerCount->value();
}

int ControlWidget::getChannelValue1() const
{
    return m_channelValue1->value();
}

int ControlWidget::getChannelValue2() const
{
    return m_channelValue2->value();
}

int ControlWidget::getChannelValue3() const
{
    return m_channelValue3->value();
}

void ControlWidget::setRegisterRange(int maxRegisters)
{
    m_startRegister->setRange(1, maxRegisters);
    updateRegisterCountRange();
}

void ControlWidget::setHighLevel(bool high)
{
    m_levelGroup->button(high ? 0 : 1)->setChecked(true);
}

void ControlWidget::setStartRegister(int value)
{
    m_startRegister->setValue(value);
}

void ControlWidget::setRegisterCount(int value)
{
    m_registerCount->setValue(value);
}

void ControlWidget::setChannelValue1(int value)
{
    m_channelValue1->setValue(value);
}

void ControlWidget::setChannelValue2(int value)
{
    m_channelValue2->setValue(value);
}

void ControlWidget::setChannelValue3(int value)
{
    m_channelValue3->setValue(value);
}

// ... 其他getter方法实现 