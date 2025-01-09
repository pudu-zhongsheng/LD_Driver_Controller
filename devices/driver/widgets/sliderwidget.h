#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QVector>

class SliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SliderWidget(int channelCount, QWidget *parent = nullptr);

    void setChannelCount(int count);
    void setPageSize(int size);

signals:
    void channelValueChanged(int channel, int value);  // 通道值变化信号
    void allChannelsValueChanged(int value);          // 所有通道值变化信号

private slots:
    void onAllSliderChanged(int value);
    void onChannelSliderChanged(int value);
    void onTargetChanged(const QString &target);
    void onPageChanged(int page);

private:
    QComboBox *m_targetSelect;     // 控制目标选择
    QSlider *m_allSlider;          // ALL滑条
    QSpinBox *m_allSpinBox;        // ALL数值框
    QVector<QSlider*> m_sliders;   // 通道滑条
    QVector<QSpinBox*> m_spinBoxes;// 通道数值框
    
    int m_channelCount;            // 通道总数
    int m_pageSize;                // 每页显示数量
    int m_currentPage = 0;         // 当前页码

    void initUI();
    void initConnections();
    void updatePageControls();
};

#endif // SLIDERWIDGET_H 