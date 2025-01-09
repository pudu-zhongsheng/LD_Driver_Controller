#include "paramtablewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>

ParamTableWidget::ParamTableWidget(int channelCount, QWidget *parent)
    : QWidget(parent)
    , m_channelCount(channelCount)
    , m_pageSize(8)  // 默认每页显示8个通道
{
    initUI();
}

void ParamTableWidget::initUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // 创建表格
    m_table = new QTableWidget(m_pageSize, 4, this);  // 4列：通道号、值1、值2、值3
    
    // 设置表头
    QStringList headers;
    headers << "通道" << "通道值一" << "通道值二" << "通道值三";
    m_table->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁止编辑
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    
    // 初始化表格内容
    for (int i = 0; i < m_pageSize; ++i) {
        // 通道号
        auto *channelItem = new QTableWidgetItem(QString("CH%1").arg(i + 1));
        channelItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 0, channelItem);
        
        // 初始值
        for (int j = 1; j < 4; ++j) {
            auto *item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(i, j, item);
        }
    }
    
    mainLayout->addWidget(m_table);

    // 如果通道数大于每页显示数，添加翻页控制
    if (m_channelCount > m_pageSize) {
        auto *pageLayout = new QHBoxLayout();
        auto *prevBtn = new QPushButton("上一页", this);
        auto *nextBtn = new QPushButton("下一页", this);
        
        pageLayout->addStretch();
        pageLayout->addWidget(prevBtn);
        pageLayout->addWidget(nextBtn);
        pageLayout->addStretch();
        
        mainLayout->addLayout(pageLayout);
        
        // 连接翻页按钮信号
        connect(prevBtn, &QPushButton::clicked, this, [this]() {
            if (m_currentPage > 0) {
                m_currentPage--;
                updatePageControls();
            }
        });
        
        connect(nextBtn, &QPushButton::clicked, this, [this]() {
            int maxPage = (m_channelCount + m_pageSize - 1) / m_pageSize;
            if (m_currentPage < maxPage - 1) {
                m_currentPage++;
                updatePageControls();
            }
        });
    }
}

void ParamTableWidget::updatePageControls()
{
    int startChannel = m_currentPage * m_pageSize;
    
    // 更新表格内容
    for (int i = 0; i < m_pageSize; ++i) {
        int channel = startChannel + i;
        bool validChannel = channel < m_channelCount;
        
        // 更新通道号
        m_table->item(i, 0)->setText(validChannel ? QString("CH%1").arg(channel + 1) : "-");
        
        // 隐藏无效通道的行
        m_table->setRowHidden(i, !validChannel);
    }
}

void ParamTableWidget::updateChannelValue(int channel, int value1, int value2, int value3)
{
    // 计算在当前页中的位置
    int pageStart = m_currentPage * m_pageSize;
    int pageEnd = pageStart + m_pageSize;
    
    // 检查通道是否在当前页中
    if (channel >= pageStart && channel < pageEnd) {
        int row = channel - pageStart;
        
        // 更新值
        m_table->item(row, 1)->setText(QString::number(value1));
        m_table->item(row, 2)->setText(QString::number(value2));
        m_table->item(row, 3)->setText(QString::number(value3));
    }
}

void ParamTableWidget::setChannelCount(int count)
{
    m_channelCount = count;
    updatePageControls();
}

void ParamTableWidget::setPageSize(int size)
{
    // 调整表格行数
    while (m_table->rowCount() > size) {
        m_table->removeRow(m_table->rowCount() - 1);
    }
    while (m_table->rowCount() < size) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        
        // 初始化新行
        auto *channelItem = new QTableWidgetItem(QString("CH%1").arg(row + 1));
        channelItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, channelItem);
        
        for (int j = 1; j < 4; ++j) {
            auto *item = new QTableWidgetItem("0");
            item->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(row, j, item);
        }
    }
    
    m_pageSize = size;
    updatePageControls();
} 