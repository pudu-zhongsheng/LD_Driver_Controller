#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "communication/comutil.h"
#include "serial/serialportmanager.h"
#include "util/globals.h"
#include "chart/data01chart.h"
#include "chart/data02chart.h"
#include "chart/data03chart.h"
#include "chart/data08chart.h"
#include "chart/data15chart.h"
#include <QMainWindow>
#include <QTimer>
#include <QVariant>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 外部信号
    void parseReceivedData(const QByteArray &data); // 解析接收到的数据
    // 自身信号
    void connectError();    // 连接错误串口，断开连接
    // 按钮
    void on_connectBtn_clicked();   // 连接按钮槽函数
    void on_disConnectBtn_clicked();    // 断开连接按钮槽函数
    void on_countMeasureBtn_clicked();  // 开始计数测量按钮槽函数
    void on_timeMeasureBtn_clicked();   // 开始定时测量按钮槽函数
    void on_stopMeasureBtn_clicked();   // 中断测量按钮槽函数

private:
    Ui::MainWindow *ui;
    SerialPortManager *serial;
    ComUtil *comutil;
    QTimer *selSerial;  // 定时搜索串口
    QTimer *connectTimer;   // 连接正确串口定时器
    QTimer *measureTimer;   // 定时测量数据，时间到了就停止
    bool selSerialAble; // 可否连接
    bool continueSend;  // 是否持续发送请求
    int baudRate;
    Globals global;

    // 图表对象
    Data01Chart *chart1;
    Data02Chart *chart2;
    Data03Chart *chart3;
    Data08Chart *chart8;
    Data15Chart *chart15;

    // 存放查询获取数据
    QList<QMap<QString,QVariant>> data01;
    QList<QMap<QString,QVariant>> data02;
    QList<QMap<QString,QVariant>> data03;
    QList<QMap<QString,QVariant>> data08;
    QList<QMap<QString,QVariant>> data15;
    QList<QMap<QString,QVariant>> data45;
    QMap<QString,QVariant> data47;  // 用户校准系数

    int measureState;   // 开始查询时的状态页

    void initUI();  // 初始化界面
    void initProgram();    // 定时搜索可用串口
    void selectSerial();   // 搜索可用串口
    bool SerialConnect(const QString &portName); // 建立串口连接
    void sendInitialData(); // 发送数据
};
#endif // MAINWINDOW_H
