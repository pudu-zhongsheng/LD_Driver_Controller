#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QSpinBox>

class ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlWidget(QWidget *parent = nullptr);

    void setRegisterRange(int maxRegisters);  // 设置最大寄存器数
    int getStartRegister() const;
    int getRegisterCount() const;
    bool isHighLevel() const;
    int getChannelValue1() const;
    int getChannelValue2() const;
    int getChannelValue3() const;
    void setHighLevel(bool high);
    void setStartRegister(int value);
    void setRegisterCount(int value);
    void setChannelValue1(int value);
    void setChannelValue2(int value);
    void setChannelValue3(int value);

signals:
    void controlChanged();  // 控制参数变化信号

private:
    QButtonGroup *m_levelGroup;    // 电平选择
    QSpinBox *m_channelValue1;     // 通道值一
    QSpinBox *m_channelValue2;     // 通道值二
    QSpinBox *m_channelValue3;     // 通道值三
    QSpinBox *m_startRegister;     // 起始寄存器
    QSpinBox *m_registerCount;     // 寄存器数

    void initUI();
    void initConnections();
    void updateRegisterCountRange();
};

#endif // CONTROLWIDGET_H 