#ifndef PARAMTABLEWIDGET_H
#define PARAMTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>

class ParamTableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ParamTableWidget(int channelCount, QWidget *parent = nullptr);

    void setChannelCount(int count);
    void updateChannelValue(int channel, int value1, int value2, int value3);
    void setPageSize(int size);

private:
    QTableWidget *m_table;
    int m_channelCount;
    int m_pageSize;
    int m_currentPage = 0;

    void initUI();
    void updatePageControls();
};

#endif // PARAMTABLEWIDGET_H 