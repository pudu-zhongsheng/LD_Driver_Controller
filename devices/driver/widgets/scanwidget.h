#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QTimer>

class ScanWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScanWidget(QWidget *parent = nullptr);

    QString currentTarget() const { return m_targetSelect->currentText(); }

signals:
    void scanValueChanged(int value);  // 扫描值变化信号

private slots:
    void startScan();
    void stopScan();
    void onScanTimeout();

private:
    QComboBox *m_targetSelect;     // 扫描目标选择
    QSpinBox *m_stepValue;         // 步长值
    QSpinBox *m_stepInterval;      // 步长间隔(ms)
    QSpinBox *m_endValue;          // 终止值
    QPushButton *m_startBtn;       // 开始按钮
    QPushButton *m_stopBtn;        // 停止按钮
    QTimer *m_scanTimer;           // 扫描定时器
    
    int m_currentValue = 0;        // 当前值

    void initUI();
    void initConnections();
};

#endif // SCANWIDGET_H 