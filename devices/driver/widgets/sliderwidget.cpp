#include "sliderwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

SliderWidget::SliderWidget(int channelCount, QWidget *parent) 
    : QWidget(parent)
    , m_channelCount(channelCount)
    , m_pageSize(8)  // 默认每页显示8个通道
{
    initUI();
    initConnections();
}

void SliderWidget::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // 1. 控制目标选择
    auto *targetLayout = new QHBoxLayout();
    targetLayout->addWidget(new QLabel("控制目标:", this));
    m_targetSelect = new QComboBox(this);
    m_targetSelect->addItems({"通道值一", "通道值二", "通道值三"});
    targetLayout->addWidget(m_targetSelect);
    targetLayout->addStretch();
    mainLayout->addLayout(targetLayout);

    // 2. ALL滑条
    auto *allGroup = new QGroupBox("ALL控制", this);
    auto *allLayout = new QHBoxLayout(allGroup);
    
    m_allSlider = new QSlider(Qt::Horizontal, this);
    m_allSlider->setRange(0, 255);
    allLayout->addWidget(m_allSlider);
    
    m_allSpinBox = new QSpinBox(this);
    m_allSpinBox->setRange(0, 255);
    allLayout->addWidget(m_allSpinBox);
    
    mainLayout->addWidget(allGroup);

    // 3. 通道滑条区域
    auto *channelGroup = new QGroupBox("通道控制", this);
    auto *channelLayout = new QGridLayout(channelGroup);
    
    // 创建通道滑条和数值框
    for (int i = 0; i < m_channelCount; ++i) {
        auto *slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 255);
        m_sliders.append(slider);
        
        auto *spinBox = new QSpinBox(this);
        spinBox->setRange(0, 255);
        m_spinBoxes.append(spinBox);
        
        // 添加到布局
        int row = i % m_pageSize;
        channelLayout->addWidget(new QLabel(QString("CH%1:").arg(i + 1), this), row, 0);
        channelLayout->addWidget(slider, row, 1);
        channelLayout->addWidget(spinBox, row, 2);
    }
    
    mainLayout->addWidget(channelGroup);

    // 4. 翻页控制(如果通道数大于每页显示数)
    if (m_channelCount > m_pageSize) {
        auto *pageLayout = new QHBoxLayout();
        auto *prevBtn = new QPushButton("上一页", this);
        auto *nextBtn = new QPushButton("下一页", this);
        pageLayout->addWidget(prevBtn);
        pageLayout->addWidget(nextBtn);
        mainLayout->addLayout(pageLayout);
        
        connect(prevBtn, &QPushButton::clicked, this, [this]() {
            if (m_currentPage > 0) {
                onPageChanged(m_currentPage - 1);
            }
        });
        
        connect(nextBtn, &QPushButton::clicked, this, [this]() {
            int maxPage = (m_channelCount + m_pageSize - 1) / m_pageSize;
            if (m_currentPage < maxPage - 1) {
                onPageChanged(m_currentPage + 1);
            }
        });
    }
}

void SliderWidget::initConnections()
{
    // ALL滑条和数值框的双向绑定
    connect(m_allSlider, &QSlider::valueChanged, m_allSpinBox, &QSpinBox::setValue);
    connect(m_allSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_allSlider, &QSlider::setValue);
    connect(m_allSlider, &QSlider::valueChanged, this, &SliderWidget::onAllSliderChanged);

    // 各通道滑条和数值框的双向绑定
    for (int i = 0; i < m_channelCount; ++i) {
        connect(m_sliders[i], &QSlider::valueChanged, m_spinBoxes[i], &QSpinBox::setValue);
        connect(m_spinBoxes[i], QOverload<int>::of(&QSpinBox::valueChanged), m_sliders[i], &QSlider::setValue);
        connect(m_sliders[i], &QSlider::valueChanged, this, [this, i](int value) {
            emit channelValueChanged(i, value);
        });
    }

    // 控制目标变化
    connect(m_targetSelect, &QComboBox::currentTextChanged, this, &SliderWidget::onTargetChanged);
}

void SliderWidget::onAllSliderChanged(int value)
{
    // 更新所有通道的值
    for (auto *slider : m_sliders) {
        slider->setValue(value);
    }
    emit allChannelsValueChanged(value);
}

void SliderWidget::onChannelSliderChanged(int value)
{
    auto *slider = qobject_cast<QSlider*>(sender());
    if (slider) {
        int index = m_sliders.indexOf(slider);
        if (index >= 0) {
            emit channelValueChanged(index, value);
        }
    }
}

void SliderWidget::onTargetChanged(const QString &target)
{
    // 根据目标更新所有滑条的值
    // 具体实现需要与Driver8CH类配合
}

void SliderWidget::onPageChanged(int page)
{
    m_currentPage = page;
    updatePageControls();
}

void SliderWidget::updatePageControls()
{
    // 更新显示的通道控件
    int startIndex = m_currentPage * m_pageSize;
    for (int i = 0; i < m_channelCount; ++i) {
        bool visible = (i >= startIndex && i < startIndex + m_pageSize);
        m_sliders[i]->setVisible(visible);
        m_spinBoxes[i]->setVisible(visible);
    }
}

void SliderWidget::setChannelCount(int count)
{
    // 更新通道数量
    m_channelCount = count;
    updatePageControls();
}

void SliderWidget::setPageSize(int size)
{
    m_pageSize = size;
    updatePageControls();
} 