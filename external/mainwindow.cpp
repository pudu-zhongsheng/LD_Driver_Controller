#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new SerialPortManager(this))
    , comutil(new ComUtil(this))
    , selSerial(new QTimer(this))
    , connectTimer(new QTimer(this))
    , measureTimer(new QTimer(this))
    , selSerialAble(false)
    , baudRate(9600)
{
    ui->setupUi(this);
    initUI();
    initProgram();  // 定时搜索串口

    connect(serial,&SerialPortManager::dataReceived,this,&MainWindow::parseReceivedData);
    connect(connectTimer,&QTimer::timeout,this,&MainWindow::connectError);
    connect(measureTimer,&QTimer::timeout,this,&MainWindow::on_stopMeasureBtn_clicked);
}

MainWindow::~MainWindow()
{
    disconnect(this,nullptr,nullptr,nullptr);
    serial->deleteLater();
    selSerial->deleteLater();
    connectTimer->deleteLater();
    measureTimer->deleteLater();
    delete ui;
}

void MainWindow::initUI()
{
    // 创建Data01Chart对象
    chart1 = new Data01Chart(this);
    // 将chart1插入到tabwidget中
    ui->tabWidget->addTab(chart1, "X,Y,Z");
    // 如果需要设置默认选择的页面为第一页（chart1所在页），可以加上以下代码
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(chart1));

    // 创建Data02Chart对象
    chart2 = new Data02Chart(this);
    ui->tabWidget->addTab(chart2,"Ev,X,Y");

    // 创建Data03Chart对象
    chart3 = new Data03Chart(this);
    ui->tabWidget->addTab(chart3,"Ev,U',V'");

    // 创建Data08Chart对象
    chart8 = new Data08Chart(this);
    ui->tabWidget->addTab(chart8,"Ev,Tcp,Δuv");

    // 创建Data15Chart对象
    chart15 = new Data15Chart(this);
    ui->tabWidget->addTab(chart15,"Ev,DW,P");

    QIntValidator *validtor = new QIntValidator();
//    ui->measureCount->setValidator(validtor);
    ui->measureTime->setValidator(validtor);
}

// 定时搜索可用串口
void MainWindow::initProgram()
{
    selectSerial(); // 程序开始时进行一次搜索
    // 定时搜索串口
    connect(selSerial,&QTimer::timeout,this,&MainWindow::selectSerial);
    selSerial->start(5000);
}

// 搜索可用串口方法
void MainWindow::selectSerial()
{
    QList<QSerialPortInfo> availablePorts = serial->getAvailablePorts();
    if(availablePorts.isEmpty()){
        qDebug() << "No available serial ports found";
        return;
    }

    // 遍历前将selSerialAble置false
    selSerialAble = false;
    // 遍历并把所有串口写入combobox
    for(const QSerialPortInfo &portInfo : availablePorts){
        QString comboboxValue = portInfo.portName()+":"+portInfo.description();
        int index = ui->serialComboBox->findText(comboboxValue);
        if(index == -1){    // 当前串口未处于combobox中，需要添加
            ui->serialComboBox->addItem(comboboxValue);
        }

        // 检查当前串口是否可连接
        if(ui->serialComboBox->currentText() == comboboxValue){
            selSerialAble = true;
        }
    }

    if(serial->isConnected() && !selSerialAble){
        // 已连接但是可用串口没有当前连接那个
        global.showError("串口"+ui->serialComboBox->currentText()+"不可用");
        // 断开串口连接
        on_disConnectBtn_clicked();
    }
}


// 连接串口按钮槽函数
void MainWindow::on_connectBtn_clicked()
{
    if(serial->isConnected()){
        qDebug() << "已经连接了串口（"+ serial->currentPortName() +"）";
        global.showError("已经连接了串口（"+ serial->currentPortName() +"）");
        return;
    }
    qDebug() << "开始连接串口（"+ ui->serialComboBox->currentText() +"）";

    QString portName;

    QString currentText = ui->serialComboBox->currentText();
    QStringList parts = currentText.split(":");
    if (!parts.isEmpty()) {
        // 假设您想要冒号前的部分（即第一部分）
        portName = parts.first();
    } else {
        // 如果没有找到冒号，可以设置portName为一个默认值或进行错误处理
        qDebug() << "串口（"+ ui->serialComboBox->currentText() +"）名称有误";
        global.showError("串口（"+ ui->serialComboBox->currentText() +"）名称有误");
    }

    connectTimer->start(5000);  // 5秒计时，如果收到应答就算连接成功

    if(SerialConnect(portName)){    // 连接成功

    }
}

// 断开连接串口按钮槽函数
void MainWindow::on_disConnectBtn_clicked()
{
    if(serial->isConnected()){
        qDebug() << "断开串口（"+ serial->currentPortName() +"）的连接";
        global.showSuccess("断开串口（"+ serial->currentPortName() +"）的连接");
        serial->disconnectPort();
    }else{
        qDebug() << "当前没有连接串口";
        global.showError("当前没有连接串口");
    }
}

// 建立串口连接
bool MainWindow::SerialConnect(const QString &portName)
{
//    int baudRate = ui->baudrateLineEdit->text().toInt();

    bool connectState = serial->connectToPort(portName,baudRate);

    if(connectState){
        qDebug() << "连接"+portName+"成功";
//        global.showSuccess("连接"+portName+"成功");

        // 需要将照度计设置为PC模式
        serial->sendData(comutil->setPCConnect54());

        return true;
    }else{
        qDebug() << "连接"+portName+"失败";
        global.showError("连接"+portName+"失败");
        return false;
    }
}

// 连接错误串口，断开连接
void MainWindow::connectError()
{
    if(serial->isConnected()){
        qDebug() << "连接错误串口，断开连接";
        global.showError("连接错误串口，断开连接");
        serial->disconnectPort();
    }
}

// 发送读取数据指令，
void MainWindow::sendInitialData()
{
    quint8 header = 0x00;
    char CFable;
    if(ui->CFModeComboBox->currentIndex() == 0){
        CFable = '2';   // 禁用
    }else {
        CFable = '3';   // 启用
    }
    char calibrationMode = '0'; // 0规范，1多重

    // 进行EXT测量
    serial->sendData(comutil->takeEXT40());
    QTimer::singleShot(500,[this,header,CFable,calibrationMode](){
        if(measureState == 0){
            serial->sendData(comutil->readMeasure("01",header,CFable,calibrationMode));
        } else if(measureState == 1){
            serial->sendData(comutil->readMeasure("02",header,CFable,calibrationMode));
        } else if(measureState == 2){
            serial->sendData(comutil->readMeasure("03",header,CFable,calibrationMode));
        } else if(measureState == 3){
            serial->sendData(comutil->readMeasure("08",header,CFable,calibrationMode));
        } else if(measureState == 4){
            serial->sendData(comutil->readMeasure("15",header,CFable,calibrationMode));
        } else if(measureState == 5){
            serial->sendData(comutil->readMeasure("45",header,'0','0'));
        }
    });
}

// 解析接收到的数据
void MainWindow::parseReceivedData(const QByteArray &data)
{
//    qDebug() << "收到报文：" << data.toHex();

    if(data.size() == 38){  // 特殊报文45、47
        ComUtil::MeasurementData resultData = comutil->handleReceivedMeasurementDataSpecial(data);
        if(resultData.registerCode == "45" || resultData.registerCode == "47"){

            if(!(resultData.fixedValue == '1' || resultData.fixedValue == '5')){
                qDebug() << "接收异常";
//                return;
            }

            if(resultData.err == ' ' || resultData.err == '4' || resultData.err == '6' || resultData.err == '7'){
                // 接收数据成功
            } else if(resultData.err == '1' || resultData.err == '2' || resultData.err == '3'){
                global.showError("受体头电源关闭，请重启CL-200A并重新连接");
//                return;
            } else if(resultData.registerCode == "45" && resultData.err == '5'){
                global.showError("超过误差的测量值，请降低亮度或增加光源与测量仪器之间的距离。");
//                return;
            }
            if(resultData.registerCode == "45"){    // 只有45指令会判断这个
                if(resultData.rng == '0'){
                    global.showError("请设置正确的等待时间，然后再进行测量");
//                    return;
                } else if(resultData.rng == '1' || resultData.rng == '2' || resultData.rng == '3' || resultData.rng == '4'){
                    // 正常的
                } else if(resultData.rng == '6'){
                    qDebug() <<  "超出范围，测量结果不能在一个合适的范围内进行";
                    // 设置EXT模式
//                    serial->sendData(comutil->setEXT40(0x00));
//                    return;
                }

                if(resultData.ba == '1'){
                    global.showError("电池电量不足");
//                    return;
                }
            }


            // 能走到这里的都不容易，我们要相信数据具备一定的可信度，那么就把数据存起来吧
            if(resultData.registerCode == "45"){
                QMap<QString,QVariant> map;
                map.insert("X2",resultData.data1);
                map.insert("Y",resultData.data2);
                map.insert("Z",resultData.data3);
                data45.append(map);
                qDebug() << "data45.size() == " << data45.size();
                qDebug() << "X2 == " << resultData.data1;
                qDebug() << "Y == " << resultData.data2;
                qDebug() << "Z == " << resultData.data3;
                if(continueSend){
                    sendInitialData();
                }
            } else if(resultData.registerCode == "47"){
                QMap<QString,QVariant> map;
                map.insert("Ev",resultData.data1);
                map.insert("x",resultData.data2);
                map.insert("y",resultData.data3);
                data47 = map;
                qDebug() << "data47.size() == " << data47.size();
                qDebug() << "Ev == " << resultData.data1;
                qDebug() << "x == " << resultData.data2;
                qDebug() << "y == " << resultData.data3;
                if(continueSend){
                    sendInitialData();
                }
            }
        }
    } else if(data.size() == 32){   // 长报文01、02、03、08、15
        ComUtil::MeasurementData resultData = comutil->handleReceivedMeasurementData(data);
        if(resultData.registerCode == "01" || resultData.registerCode == "02" ||
           resultData.registerCode == "03" || resultData.registerCode == "08" ||
           resultData.registerCode == "15"){
            // 开始测量的时候，把数据缓存清空
            if(!(resultData.fixedValue == '1' || resultData.fixedValue == '5')){
                qDebug() << "接收异常";
//                return;
            }

            if(resultData.err == ' ' || resultData.err == '4'){
                // 接收数据成功
            } else if((resultData.registerCode == "02" || resultData.registerCode == "03")
                      && resultData.err == '6'){
                global.showError("低亮度误差，请增加光源亮度或移动受体头更接近光源。");
            } else if(resultData.registerCode == "08" && resultData.err == '7'){
                global.showError("TCP, ∆uv测量超出范围");
            }
            else if(resultData.err == '1' || resultData.err == '2' || resultData.err == '3'){
                global.showError("受体头电源关闭，请重启CL-200A并重新连接");
//                return;
            } else if(resultData.err == '5'){
                global.showError("超过误差的测量值，请降低亮度或增加光源与测量仪器之间的距离。");
//                return;
            }

            if(resultData.rng == '0'){
                global.showError("请设置正确的等待时间，然后再进行测量");
//                return;
            } else if(resultData.rng == '1' || resultData.rng == '2' || resultData.rng == '3' || resultData.rng == '4'){
                // 正常的
            } else if(resultData.rng == '6'){
                qDebug() <<  "超出范围，测量结果不能在一个合适的范围内进行";
                // 设置EXT模式
//                serial->sendData(comutil->setEXT40(0x00));
//                return;
            }

            if(resultData.ba == '1'){
                global.showError("电池电量不足");
//                return;
            }

            // 能走到这里的都不容易，我们要相信数据具备一定的可信度，那么就把数据存起来吧
            if(resultData.registerCode == "01"){
                QMap<QString,QVariant> map;
                map.insert("X",resultData.data1);
                map.insert("Y",resultData.data2);
                map.insert("Z",resultData.data3);
                data01.append(map);
                qDebug() << "data01.size() == " << data01.size();
                qDebug() << "X == " << resultData.data1;
                qDebug() << "Y == " << resultData.data2;
                qDebug() << "Z == " << resultData.data3;
                int secs;
                if(ui->measureTime->text().isEmpty()){
                    secs = 10;
                }else {
                    secs = ui->measureTime->text().toInt();
                }
                chart1->updateChartData(resultData.data1,resultData.data2,resultData.data3,secs);
                if(continueSend){
                    QTimer::singleShot(418,[this](){    // 一般是575毫秒完成一次测试，加上延时后就是1秒
                        sendInitialData();
                    });
                }
            } else if(resultData.registerCode == "02"){
                QMap<QString,QVariant> map;
                map.insert("Ev",resultData.data1);
                map.insert("x",resultData.data2);
                map.insert("y",resultData.data3);
                data02.append(map);
                qDebug() << "data02.size() == " << data02.size();
                qDebug() << "Ev == " << resultData.data1;
                qDebug() << "x == " << resultData.data2;
                qDebug() << "y == " << resultData.data3;
                int secs;
                if(ui->measureTime->text().isEmpty()){
                    secs = 10;
                }else {
                    secs = ui->measureTime->text().toInt();
                }
                chart2->updateChartData(resultData.data1,resultData.data2,resultData.data3,secs);
                if(continueSend){
                    QTimer::singleShot(421,[this](){
                        sendInitialData();
                    });
                }
            } else if(resultData.registerCode == "03"){
                QMap<QString,QVariant> map;
                map.insert("Ev",resultData.data1);
                map.insert("u'",resultData.data2);
                map.insert("v'",resultData.data3);
                data03.append(map);
                qDebug() << "data03.size() == " << data03.size();
                qDebug() << "Ev == " << resultData.data1;
                qDebug() << "u' == " << resultData.data2;
                qDebug() << "v' == " << resultData.data3;
                int secs;
                if(ui->measureTime->text().isEmpty()){
                    secs = 60;
                }else {
                    secs = ui->measureTime->text().toInt();
                }
                chart3->updateChartData(resultData.data1,resultData.data2,resultData.data3,secs);
                if(continueSend){
                    QTimer::singleShot(418,[this](){
                        sendInitialData();
                    });
                }
            } else if(resultData.registerCode == "08"){
                QMap<QString,QVariant> map;
                map.insert("Ev",resultData.data1);
                map.insert("Tcp'",resultData.data2);
                map.insert("∆uv'",resultData.data3);
                data08.append(map);
                qDebug() << "data08.size() == " << data08.size();
                qDebug() << "Ev == " << resultData.data1;
                qDebug() << "Tcp == " << resultData.data2;
                qDebug() << "∆uv == " << resultData.data3;
                int secs;
                if(ui->measureTime->text().isEmpty()){
                    secs = 60;
                }else {
                    secs = ui->measureTime->text().toInt();
                }
                chart8->updateChartData(resultData.data1,resultData.data2,resultData.data3,secs);
                if(continueSend){
                    QTimer::singleShot(418,[this](){
                        sendInitialData();
                    });
                }
            } else if(resultData.registerCode == "15"){
                QMap<QString,QVariant> map;
                map.insert("Ev",resultData.data1);
                map.insert("DW'",resultData.data2);
                map.insert("P'",resultData.data3);
                data15.append(map);
                qDebug() << "data15.size() == " << data15.size();
                qDebug() << "Ev == " << resultData.data1;
                qDebug() << "DW == " << resultData.data2;
                qDebug() << "P' == " << resultData.data3;
                int secs;
                if(ui->measureTime->text().isEmpty()){
                    secs = 60;
                }else {
                    secs = ui->measureTime->text().toInt();
                }
                chart15->updateChartData(resultData.data1,resultData.data2,resultData.data3,secs);
                if(continueSend){
                    QTimer::singleShot(418,[this](){
                        sendInitialData();
                    });
                }
            }

        }
    } else if(data.size() == 14){   // 短报文40、48、54
        ComUtil::ShortMeasurementData resultData = comutil->handleReceivedSettingResult(data);
        if(resultData.registerCode == "40"){    // 设置EXT模式
            if(resultData.err == ' ' || resultData.err == '5' || resultData.err == '6' || resultData.err == '7'){
                // 设置成功
                qDebug() << "40指令设置EXT模式成功";

//                QTimer::singleShot(175,[this](){
//                    serial->sendData(comutil->takeEXT40()); // 进行EXT测量
//                });
            }
            else if(resultData.err == '1' || resultData.err == '2' || resultData.err == '3'){
                // Receptor head power is switched off.
                global.showError("受体头电源关闭，请重启CL-200A并重新连接");
            } else if(resultData.err == '4'){
                // EXT错误，需要发送55命令设置为“保持状态”
                // 设置保持状态
                serial->sendData(comutil->setHoldState55());
                // 无应答，等待500毫秒继续发下一帧
                QTimer::singleShot(500,[this](){
                    // 设置EXT模式
                    serial->sendData(comutil->setEXT40(0x00));
                });
            }
        }

        else if(resultData.registerCode == "48"){    // 写入用户校准系数
            if(resultData.err == ' ' || resultData.err == '5' || resultData.err == '6' || resultData.err == '7'){
                // 设置成功
                qDebug() << "48指令写入用户校准系数成功";
//                QTimer::singleShot(175,[this](){
//                    serial->sendData(comutil->takeEXT40()); // 进行EXT测量
//                });
            }
            else if(resultData.err == '1' || resultData.err == '2' || resultData.err == '3'){
                // Receptor head power is switched off.
                global.showError("受体头电源关闭或其他错误，请重启CL-200A并重新连接");
            } else if(resultData.err == '4'){
                global.showError("输入的系数值超出了有效的设置范围");
            }
        }

        else if(resultData.registerCode == "54"){    // 设置为PC模式
            // 应答是固定的，所以不做其他判断，直接执行下一帧指令
            QTimer::singleShot(500,[this](){
                // 设置保持状态
                serial->sendData(comutil->setHoldState55());
                // 无应答，等待500毫秒继续发下一帧
                QTimer::singleShot(500,[this](){
                    // 设置EXT模式
                    serial->sendData(comutil->setEXT40(0x00));
                    connectTimer->stop();   // 停止连接正确串口定时器
                    global.showSuccess("连接照度计成功。");
                });
            });
        }
    }
}

// 时间测量按钮槽函数
void MainWindow::on_timeMeasureBtn_clicked()
{
    if(!serial->isConnected()){
        global.showError("当前串口未连接");
        return;
    }

    // 先中断当前进行中的测量
    on_stopMeasureBtn_clicked();

    // 开始下一次测量
    continueSend = true;

    // 根据标签页当前所处页数做判断
    measureState = ui->tabWidget->currentIndex();
    int secs;
    if(ui->measureTime->text().isEmpty()){
        secs = 10;
    }else {
        secs = ui->measureTime->text().toInt();
    }
    if(measureState == 0){
        data01.clear();
        chart1->initSeries(secs);   // 初始化几个图表的横坐标
    } else if(measureState == 1){
        data02.clear();
        chart2->initSeries(secs);   // 初始化几个图表的横坐标
    } else if(measureState == 2){
        data03.clear();
        chart3->initSeries(secs);
    } else if(measureState == 3){
        data08.clear();
        chart8->initSeries(secs);
    } else if(measureState == 4){
        data15.clear();
        chart15->initSeries(secs);
    } else if(measureState == 5){
        data45.clear();
    }

    if(!ui->measureTime->text().isEmpty()){
        // 如果输入定时测量值不为空，那么时间到了就停止
        measureTimer->start(secs*1000);
    }

    sendInitialData();  // 首次发送测量数据请求

}

// 开始计数测量按钮槽函数
void MainWindow::on_countMeasureBtn_clicked()
{
    if(!serial->isConnected()){
        global.showError("当前串口未连接");
        return;
    }
    // 先中断当前进行中的测量
    continueSend = false;
    measureTimer->stop();

    // 开始下一次测量
    continueSend = true;

    // 根据标签页当前所处页数做判断
    measureState = ui->tabWidget->currentIndex();
    int secs;
    if(ui->measureTime->text().isEmpty()){
        secs = 60;
    }else {
        secs = ui->measureTime->text().toInt();
    }
    if(measureState == 0){
        data01.clear();
        chart1->initSeries(secs);   // 初始化几个图表的横坐标
    } else if(measureState == 1){
        data02.clear();
        chart2->initSeries(secs);   // 初始化几个图表的横坐标
    } else if(measureState == 2){
        data03.clear();
        chart3->initSeries(secs);
    } else if(measureState == 3){
        data08.clear();
        chart8->initSeries(secs);
    } else if(measureState == 4){
        data15.clear();
        chart15->initSeries(secs);
    } else if(measureState == 5){
        data45.clear();
    }

    sendInitialData();  // 首次发送测量数据请求
}

// 中断测量
void MainWindow::on_stopMeasureBtn_clicked()
{
    measureTimer->stop();
    continueSend = false;
}


