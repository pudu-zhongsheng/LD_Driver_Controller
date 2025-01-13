#include "itplus.h"
#include "ui_itplus.h"

ITPlus::ITPlus(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ITPlus)
    , serialUtil(new SerialUtil(this))
    , selEleLoad(new QTimer(this))
    , measureEleLoad(new QTimer(this))
{
    ui->setupUi(this);
    initUi();
    initData();

    // 定时器测量
    connect(measureEleLoad,&QTimer::timeout,this,&ITPlus::measureEleLoadOnTime);

    // 创建串口连接线程并启动
    initElectronicLoad();
}

ITPlus::~ITPlus(){
    disconnect(this,nullptr,nullptr,nullptr);
    // 停止所有定时器任务
    QTimer::singleShot(0,[]{}); // 清除Qt定时器队列
    delete ui;
    connectionThread->deleteLater();
    serialUtil->disconnectPort();   // 断开串口连接
    delete serialUtil;
    serialUtil = nullptr;
    eleSerialList.clear();
    delete selEleLoad;
    delete measureEleLoad;
}

// 展示界面
void ITPlus::showWidget()
{
    initData();
}

// 初始化界面
void ITPlus::initUi()
{
    ui->messageFrame->setProperty("form","card");
    ui->maxValueFrame->setProperty("form","card");
    ui->modelFrame->setProperty("form","card");
    ui->valueFrame->setProperty("form","card");
    ui->targetFrame->setProperty("form","card");

    QList<QLineEdit*> lineEditList = ui->mainFrame->findChildren<QLineEdit*>();
    foreach(QLineEdit *line ,lineEditList){
        line->setProperty("form","lineEdit");
    }
    QList<QLineEdit*> lineEditList2 = ui->tabWidget->findChildren<QLineEdit*>();
    foreach(QLineEdit *line ,lineEditList2){
        line->setProperty("form","lineEdit");
        line->setMinimumWidth(100);
        line->setMinimumHeight(30);
        style()->polish(line);
    }
    QList<QPushButton*> btnList = ui->mainFrame->findChildren<QPushButton*>();
    foreach(QPushButton *btn, btnList){
        btn->setProperty("form","white");
        style()->polish(btn);
    }

    // Tab标签页
    ui->tabWidget->setCurrentIndex(0);
    connect(ui->tabWidget,&QTabWidget::currentChanged,this,&ITPlus::switchTabWidget);

    // QToolButton按钮组管理
    // 获取tabwidget下的所有QToolButton并进行响应设置
    QList<QToolButton *> tbtns = ui->tabWidget->findChildren<QToolButton *>();
    foreach(QToolButton *btn, tbtns){
        btn->setCheckable(true);
    }

    // 输入框转型
    QList<QLineEdit *> maxValueLineEdits = ui->maxValueFrame->findChildren<QLineEdit *>();
    foreach(QLineEdit *lineEdit, maxValueLineEdits){
        connect(lineEdit,&QLineEdit::returnPressed,this,&ITPlus::maxValueHandleEnter);
    }
    QList<QLineEdit *> tabLineEidt = ui->tabWidget->findChildren<QLineEdit *>();
    foreach(QLineEdit *lineEdit, tabLineEidt){
        connect(lineEdit,&QLineEdit::returnPressed,this,&ITPlus::tabWidgetHandleEnter);
    }

    // 输入框限制
//    QIntValidator *validator = new QIntValidator();
    QDoubleValidator *validator4 = new QDoubleValidator();
    validator4->setNotation(QDoubleValidator::StandardNotation);
    validator4->setRange(0,1e10,4);  // 设置范围和小数点后四位数

    QDoubleValidator *validator3 = new QDoubleValidator();
    validator3->setNotation(QDoubleValidator::StandardNotation);
    validator3->setRange(0,1e10,3);  // 设置范围和小数点后三位数

    QDoubleValidator *validator1 = new QDoubleValidator();
    validator1->setNotation(QDoubleValidator::StandardNotation);
    validator1->setRange(0,1e10,1);  // 设置范围和小数点后一位数

    ui->maxInputVoltage->setValidator(validator3);
    ui->maxInputCurrent->setValidator(validator4);
    ui->maxInputPower->setValidator(validator3);

    ui->maxInputVoltage->setValidator(validator3);
    ui->maxInputCurrent->setValidator(validator4);
    ui->maxInputPower->setValidator(validator3);

    ui->minTargetVoltage->setValidator(validator3);
    ui->maxTargetVoltage->setValidator(validator3);
    ui->minTargetPower->setValidator(validator3);
    ui->maxTargetPower->setValidator(validator3);
    ui->minTargetCurrent->setValidator(validator4);
    ui->maxTargetCurrent->setValidator(validator4);

    QList<QLineEdit *> lineEdit2 =ui->tabWidget->findChildren<QLineEdit *>();
    foreach(QLineEdit *lineEdit, lineEdit2){
        lineEdit->setValidator(validator3);
    }
    ui->ccValueLineEdit->setValidator(validator4);
    ui->dcAValueLineEdit->setValidator(validator4);
    ui->dcBValueLineEdit->setValidator(validator4);
    ui->dcATimeLineEdit->setValidator(validator4);
    ui->dcBTimeLineEdit->setValidator(validator4);
    ui->dvATimeLineEdit->setValidator(validator4);
    ui->dvBTimeLineEdit->setValidator(validator4);
    ui->dwATimeLineEdit->setValidator(validator4);
    ui->dwBTimeLineEdit->setValidator(validator4);
    ui->drATimeLineEdit->setValidator(validator4);
    ui->drBTimeLineEdit->setValidator(validator4);

    // 按钮组与按钮设置
    baseWorkModelBtn = new QButtonGroup();
    baseWorkModelBtn->addButton(ui->CCBtn,0);
    baseWorkModelBtn->addButton(ui->CVBtn,1);
    baseWorkModelBtn->addButton(ui->CWBtn,2);
    baseWorkModelBtn->addButton(ui->CRBtn,3);
    baseWorkModelBtn->setExclusive(true);
    connect(baseWorkModelBtn,QOverload<int>::of(&QButtonGroup::buttonClicked),
            this,&ITPlus::switchBaseModelPage);
    baseWorkModelBtn->button(0)->setChecked(true);  // 设置默认按钮与显示页面
    ui->baseModelStackedWidget->setCurrentIndex(0);
    serialUtil->sendData(eleLoad_ITPlus.createGetConstantCurrentCommand());

    dynamicModelBtn = new QButtonGroup();
    dynamicModelBtn->addButton(ui->DCBtn,0);
    dynamicModelBtn->addButton(ui->DVBtn,1);
    dynamicModelBtn->addButton(ui->DWBtn,2);
    dynamicModelBtn->addButton(ui->DRBtn,3);
    dynamicModelBtn->setExclusive(true);
    connect(dynamicModelBtn,QOverload<int>::of(&QButtonGroup::buttonClicked),
            this,&ITPlus::switchDynamicModelPage);
    dynamicModelBtn->button(0)->setChecked(true);   // 设置默认按钮与显示页面
    ui->dynamicModelStackedWidget->setCurrentIndex(0);
    serialUtil->sendData(eleLoad_ITPlus.createGetDynamicCurrentParamsCommand());

    controlModeBtn = new QButtonGroup();
    controlModeBtn->addButton(ui->PCControlModeRadioBtn);
    controlModeBtn->addButton(ui->handControlModeRadioBtn);
    controlModeBtn->setExclusive(true);
    connect(ui->PCControlModeRadioBtn,&QRadioButton::clicked,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createControlModeCommand(1));   // 设置为PC控制模式
    });
    connect(ui->handControlModeRadioBtn,&QRadioButton::clicked,[this](){
       serialUtil->sendData( eleLoad_ITPlus.createControlModeCommand(0));   // 设置为面板控制模式
    });

    outPutModeBtn = new QButtonGroup();
    outPutModeBtn->addButton(ui->onOutputModeRadioBtn);
    outPutModeBtn->addButton(ui->offOutPutModeRadioBtn);
    outPutModeBtn->setExclusive(true);
    connect(ui->onOutputModeRadioBtn,&QRadioButton::clicked,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createLoadStateCommand(1));
    });
    connect(ui->offOutPutModeRadioBtn,&QRadioButton::clicked,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createLoadStateCommand(0));
    });
}

void ITPlus::initData()
{
    // 重置告警标识
    targetVoltage = false;
    targetCurrent = false;
    targetPower = false;

    // 获取产品基本信息
    serialUtil->sendData(eleLoad_ITPlus.createGetProductMessage());
    // 获取最大输入电压电流等
    serialUtil->sendData(eleLoad_ITPlus.createGetMaxVoltageCommand());
    serialUtil->sendData(eleLoad_ITPlus.createGetMaxCurrentCommand());
    serialUtil->sendData(eleLoad_ITPlus.createGetMaxPowerCommand());
    // 获取负载当前工作模式
    serialUtil->sendData(eleLoad_ITPlus.createGetWorkModel());
    // 设置负载输入模式（默认是OFF）
    ui->offOutPutModeRadioBtn->setChecked(true);
    // 获取定电流值、获取动态电流值
    serialUtil->sendData(eleLoad_ITPlus.createGetConstantCurrentCommand());
    serialUtil->sendData(eleLoad_ITPlus.createGetDynamicVoltageParamsCommand());
}

void ITPlus::initElectronicLoad()
{
    connectionThread = new ITPlusSerialHandler(this);
    connect(connectionThread, &ITPlusSerialHandler::connectionSuccess, this, &ITPlus::onSerialConnectionSuccess);
//    connect(connectionThread, &ITPlusSerialHandler::connectionFailed, this, &MainWindow::onSerialConnectionFailed);
//    connect(connectionThread, &ITPlusSerialHandler::connectionLost, this, &MainWindow::onSerialConnectionLost);
    connectionThread->startScan();
}

// 串口连接成功
void ITPlus::onSerialConnectionSuccess(SerialUtil* serial)
{
    qDebug() << "Connection successful";
    serialUtil = serial;

    emit connectSerial();   // 发出串口连接成功信号
    connectionThread->stopScan();

    // 连接后开始接收数据
    connect(serialUtil, &SerialUtil::dataReceived, this, &ITPlus::eleLoadDataReceived);
    connect(serialUtil, &SerialUtil::portDisconnected, this, &ITPlus::disconnectEleLoadSerial);

//    measureEleLoad->start(1000);    // 启动定时器，每秒获取数据

    // 初始是PC控制模式
    ui->PCControlModeRadioBtn->setChecked(true);
    // 获取初始化数据
    initData();

    on_measureBtn_clicked();    // 开始测量
}

// 串口断开信号
void ITPlus::disconnectEleLoadSerial(const QString &portName)
{
    ToastMessage *toast = new ToastMessage("电子负载"+portName+"断开连接",this);
    toast->showToast(2000);
    measureEleLoad->stop();

    emit disConnectSerial();    // 发出断开串口信号

    // 断开后重新遍历可连接串口
    initElectronicLoad();
}

// 电子负载数据接收
void ITPlus::eleLoadDataReceived(const QByteArray &data)
{
    eleBuffer.append(data);
    QString nowTime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    while (eleBuffer.size() >= 26) {
        if(static_cast<quint8>(eleBuffer[0]) == 0xAA &&
           static_cast<quint8>(eleBuffer[25]) == sum_verify(eleBuffer.mid(0,25))){
            quint8 functionCode = static_cast<quint8>(eleBuffer[2]);
            if(functionCode == 0x12){  // 设置帧
                quint8 state = static_cast<quint8>(eleBuffer[3]);
                QString showText;

                if(state == 0x90){
                    showText = "校验和错误";
                }else if(state == 0xA0){
                    showText = "设置参数错误或参数溢出";
                }else if(state == 0xB0){
                    showText = "命令不能被执行";
                }else if(state == 0xC0){
                    showText = "命令是无效的";
                }else if(state == 0xD0){
                    showText = "命令是未知的";
                }else if(state == 0x80){
                    showText = "设置成功";
                }
                ToastMessage *toast = new ToastMessage(showText,this);
                toast->showToast(1000);
            }

            else {  // 非12H指令的校验指令
                if(static_cast<quint8>(eleBuffer[3]) == 0x90){
                    ToastMessage *toast = new ToastMessage("指令校验和错误",this);
                    toast->showToast(1000);
                }
                else{
                    QByteArray getData = eleBuffer.mid(0,26);

                    if(functionCode == 0x23){    // 读取最大输入电压
                        float maxVoltage = eleLoad_ITPlus.analyseMaxVoltage(getData);
                        ui->maxInputVoltage->setText(QString::number(maxVoltage));
                    } else if(functionCode == 0x25){    // 读取最大输入电流
                        float maxCurrent = eleLoad_ITPlus.analyseMaxCurrent(getData);
                        ui->maxInputCurrent->setText(QString::number(maxCurrent));
                    } else if(functionCode == 0x27){    // 读取最大输入功率
                        float maxPower = eleLoad_ITPlus.analyseMaxPower(getData);
                        ui->maxInputPower->setText(QString::number(maxPower));
                    } else if(functionCode == 0x29){    // 读取当前负载的工作模式Fixed下的(0为CC，1为CV，2为CW，3为CR)
                        quint8 workModel = eleLoad_ITPlus.analyseLoadMode(getData);
                        QString categoryName;
                        if(workModel == 0){
                            categoryName = "CC";
                        } else if(workModel == 1){
                            categoryName = "CV";
                        } else if(workModel == 2){
                            categoryName = "CW";
                        } else if(workModel == 3){
                            categoryName = "CR";
                        }
                        ui->actionCategory->setText(categoryName);
                    } else if(functionCode == 0x2B){    // 读取定电流值
                        float ccCurrent = eleLoad_ITPlus.analyseConstantCurrent(getData);
                        ui->ccValue->setText(QString::number(ccCurrent)+"A");
                    } else if(functionCode == 0x2D){    // 读取定电压值
                        float ccVoltage = eleLoad_ITPlus.analyseConstantVoltage(getData);
                        ui->cvValue->setText(QString::number(ccVoltage)+"V");
                    } else if(functionCode == 0x2F){    // 读取定功率值
                        float ccPower = eleLoad_ITPlus.analyseConstantPower(getData);
                        ui->cwValue->setText(QString::number(ccPower)+"W");
                    } else if(functionCode == 0x31){    // 读取定电阻值
                        float ccResistance = eleLoad_ITPlus.analyseConstantResistance(getData);
                        ui->crValue->setText(QString::number(ccResistance)+"R");
                    } else if(functionCode == 0x33){    // 读取动态电流参数值
                        EleLoad_ITPlus::DynamicParams dcCurrent = eleLoad_ITPlus.analyseDynamicCurrentParams(getData);
                        ui->dcAValue->setText(QString::number(dcCurrent.valueA)+"A");
                        ui->dcATime->setText(QString::number(dcCurrent.timeA)+"S");
                        ui->dcBValue->setText(QString::number(dcCurrent.valueB)+"A");
                        ui->dcBTime->setText(QString::number(dcCurrent.timeB)+"S");
                    } else if(functionCode == 0x35){    // 读取动态电压参数值
                        EleLoad_ITPlus::DynamicParams dvCurrent = eleLoad_ITPlus.analyseDynamicVoltageParams(getData);
                        ui->dvAValue->setText(QString::number(dvCurrent.valueA)+"V");
                        ui->dvATime->setText(QString::number(dvCurrent.timeA)+"S");
                        ui->dvBValue->setText(QString::number(dvCurrent.valueB)+"V");
                        ui->dvBTime->setText(QString::number(dvCurrent.timeB)+"S");
                    } else if(functionCode == 0x37){    // 读取动态功率参数值
                        EleLoad_ITPlus::DynamicParams dwCurrent = eleLoad_ITPlus.analyseDynamicPowerParams(getData);
                        ui->dwAValue->setText(QString::number(dwCurrent.valueA)+"W");
                        ui->dwATime->setText(QString::number(dwCurrent.timeA)+"S");
                        ui->dwBValue->setText(QString::number(dwCurrent.valueB)+"W");
                        ui->dwBTime->setText(QString::number(dwCurrent.timeB)+"S");
                    } else if(functionCode == 0x39){    // 读取动态电阻参数值
                        EleLoad_ITPlus::DynamicParams drCurrent = eleLoad_ITPlus.analyseDynamicResistanceParams(getData);
                        ui->drAValue->setText(QString::number(drCurrent.valueA)+"R");
                        ui->drATime->setText(QString::number(drCurrent.timeA)+"S");
                        ui->drBValue->setText(QString::number(drCurrent.valueB)+"R");
                        ui->drBTime->setText(QString::number(drCurrent.timeB)+"S");
                    } else if(functionCode == 0x5E){    // 读取负载工作模式
                        quint8 workModel = eleLoad_ITPlus.analyseWorkModel(getData);
                        QString workModelText;
                        if(workModel == 0){
                            workModelText = "FIXED";
                            // 获取FIXED工作模式的类型
                            serialUtil->sendData(eleLoad_ITPlus.createGetLoadModeCommand());
                        } else if(workModel == 1){
                            workModelText = "SHORT";
                        } else if(workModel == 2){
                            workModelText = "TRANSITION";
                        } else if(workModel == 3){
                            workModelText = "LIST";
                        }
                        ui->actionModel->setText(workModelText);
                    } else if(functionCode == 0x5F){   // 读取实时vcp以及温度等信息
                        emit receivedEleLoadData(0x5F,getData);  // 发出5F数据

                        EleLoad_ITPlus::InputParams receivedData = eleLoad_ITPlus.analyseInputParams(eleBuffer.mid(0,26));

                        ui->vValue->setText(QString::number(receivedData.voltage)+"V");
                        ui->aValue->setText(QString::number(receivedData.current)+"A");
                        ui->pValue->setText(QString::number(receivedData.power)+"W");
                        ui->temperature->setText(QString::number(receivedData.radiatorTemperature));
                        if(receivedData.handleStateRegister.out && (!ui->onOutputModeRadioBtn->isChecked())){   // 0：off，1：on
                            // 如果判断当前输出模式是on，并且当前没有选中on按钮，则进行一次勾选
                            ui->onOutputModeRadioBtn->setChecked(true);
                        }
                        if((!receivedData.handleStateRegister.out) && (!ui->offOutPutModeRadioBtn->isChecked())){   // 0：off，1：on
                            // 如果判断当前输出模式是off，并且当前没有选中off按钮，则进行一次勾选
                            ui->offOutPutModeRadioBtn->setChecked(true);
                        }
                        if(receivedData.handleStateRegister.rem && (!ui->PCControlModeRadioBtn->isChecked())){   // 0：面板，1：pc
                            // 如果判断当前控制模式是PC，并且当前没有选中PC按钮，则进行一次勾选
                            ui->PCControlModeRadioBtn->setChecked(true);
                        }
                        if((!receivedData.handleStateRegister.rem) && (!ui->handControlModeRadioBtn->isChecked())){   // 0：面板，1：pc
                            // 如果判断当前控制模式是面板，并且当前没有选中面板按钮，则进行一次勾选
                            ui->handControlModeRadioBtn->setChecked(true);
                        }
                        qsoundWarn(receivedData.voltage,receivedData.current,receivedData.power); // 获取数据达到目标值就告警提示

                    } else if(functionCode == 0x6A){    // 读取产品基本信息
                        EleLoad_ITPlus::ProductMessage receivedData = eleLoad_ITPlus.analyseProductMessage(getData);
                        qDebug() << "productModel:" << receivedData.productModel <<
                                    ", softwareVersion:" << receivedData.softwareVersion <<
                                    ", productSerial:" << receivedData.productSerial <<
                                    ", nowTime:" << nowTime;

                        ui->productModel->setText(receivedData.productModel);
                        ui->softwareVersion->setText(receivedData.softwareVersion);
                        ui->productSerial->setText(receivedData.productSerial);
                    } else if(functionCode == 0x5E){    // 读取当前负载的工作模式(0为CC，1为CV，2为CW，3为CR)
                        quint8 workModel = eleLoad_ITPlus.analyseWorkModel(getData);
                        QString modelName;
                        if(workModel == 0){
                            modelName = "FIXED";
                        } else if(workModel == 1){
                            modelName = "SHORT";
                        } else if(workModel == 2){
                            modelName = "TRANSITION";
                        } else if(workModel == 3){
                            modelName = "LIST";
                        }
                        ui->actionModel->setText(modelName);
                    }

                }
            }

            eleBuffer.remove(0,26);

        }else{
            eleBuffer.remove(0,1);
        }
    }


}

// 获取数据达到目标值即告警提示
void ITPlus::qsoundWarn(float voltage, float current, float power)
{
    bool nomal = true;
    // 目标电压
    if((!ui->minTargetVoltage->text().isEmpty()) && (!ui->maxTargetVoltage->text().isEmpty())){
        if((voltage > ui->minTargetVoltage->text().toFloat()) && (voltage < ui->maxTargetVoltage->text().toFloat())){
            if(!targetVoltage){
                nomal = false;
                targetVoltage = true;   // 标记为已经告警过
            }
        } else{
            targetVoltage = false;
        }
    }
    // 目标电流
    if((!ui->minTargetCurrent->text().isEmpty()) && (!ui->maxTargetCurrent->text().isEmpty())){
        if((current > ui->minTargetCurrent->text().toFloat()) && (current < ui->maxTargetCurrent->text().toFloat())){
            if(!targetCurrent){
                nomal = false;
                targetCurrent = true;
            }
        } else{
            targetCurrent = false;
        }
    }
    // 目标功率
    if((!ui->minTargetPower->text().isEmpty()) && (!ui->maxTargetPower->text().isEmpty())){
        if((power > ui->minTargetPower->text().toFloat()) && (power < ui->maxTargetPower->text().toFloat())){
            if(!targetPower){
                nomal = false;
                targetPower = true;
            }
        } else {
            targetPower = false;
        }
    }
    if(!nomal){ // 如果有异常数值
        emit warnMusic();
        // 语音播放
        QSound::play(":/res/music/Windows Background.wav");
    }
}

// 定时读取电子负载测量数据
void ITPlus::measureEleLoadOnTime()
{
    /*** 后续可以根据选择模式不同，来发送不同的测量指令 ***/

    if(serialUtil->isConnected()){    // 只有在连接状态才进行数据发送
        serialUtil->sendData(eleLoad_ITPlus.createGetInputCommand()); // 5F指令
    } else{
        // 如果当前没有在连接状态，则停止定时器。
        measureEleLoad->stop();
    }
}

// 切换tab标签页来改变负载工作模式
void ITPlus::switchTabWidget(int tabNum)
{
    if(tabNum == 0){    // FIXED固定模式
        serialUtil->sendData(eleLoad_ITPlus.createSetWorkModel(0));
        ui->label_56->setText("工作类型：");
    } else if(tabNum == 1){ // TRANSITION模式
        serialUtil->sendData(eleLoad_ITPlus.createSetWorkModel(2));
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicCurrentParamsCommand());
        ui->actionCategory->setText("");
        ui->label_56->setText("");
    }
    // 读取工作模式
    serialUtil->sendData(eleLoad_ITPlus.createGetWorkModel());
}

// 切换基础模式的页面
void ITPlus::switchBaseModelPage(int pageNum)
{
    // 读取对应页面的值并设置模式
    if(pageNum == 0){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantCurrentCommand());
    }else if(pageNum == 1){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantVoltageCommand());
    }else if(pageNum == 2){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantPowerCommand());
    }else if(pageNum == 3){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantResistanceCommand());
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetLoadModeCommand(pageNum));
    serialUtil->sendData(eleLoad_ITPlus.createGetLoadModeCommand());
    ui->baseModelStackedWidget->setCurrentIndex(pageNum);
}

// 切换动态模式的页面
void ITPlus::switchDynamicModelPage(int pageNum)
{
    // 读取对应页面的值
    if(pageNum == 0){   // 读取动态电流值
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicCurrentParamsCommand());
        ui->actionCategory->setText("DC");
    }else if(pageNum == 1){   // 读取动态电压值
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicVoltageParamsCommand());
        ui->actionCategory->setText("DV");
    }else if(pageNum == 2){   // 读取动态功率值
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicPowerParamsCommand());
        ui->actionCategory->setText("DW");
    }else if(pageNum == 3){   // 读取动态电阻值
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicResistanceParamsCommand());
        ui->actionCategory->setText("DR");
    }
    ui->dynamicModelStackedWidget->setCurrentIndex(pageNum);
}

// 计算累加和
quint8 ITPlus::sum_verify(QByteArray dataBytes)
{
    quint8 checksum = 0;
    for(char byte : dataBytes){
        checksum += static_cast<quint8>(byte);
    }
    return checksum;
}

// 修改最大电压电流功率按钮槽函数
void ITPlus::on_changeBtn_clicked()
{
    if(ui->maxInputVoltage->text().isEmpty() ||
            ui->maxInputCurrent->text().isEmpty() ||
            ui->maxInputPower->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("最大电压、电流、功率不能为空",this);
        toast->showToast(1000);
        return;
    }

    serialUtil->sendData(eleLoad_ITPlus.createSetMaxVoltageCommand(ui->maxInputVoltage->text().toFloat()));
    serialUtil->sendData(eleLoad_ITPlus.createSetMaxCurrentCommand(ui->maxInputCurrent->text().toFloat()));
    serialUtil->sendData(eleLoad_ITPlus.createSetMaxPowerCommand(ui->maxInputPower->text().toFloat()));

    QTimer::singleShot(500,[this](){
        // 获取最大输入电压电流等
        serialUtil->sendData(eleLoad_ITPlus.createGetMaxVoltageCommand());
        serialUtil->sendData(eleLoad_ITPlus.createGetMaxCurrentCommand());
        serialUtil->sendData(eleLoad_ITPlus.createGetMaxPowerCommand());
    });
}

// 修改定电流值
void ITPlus::on_ccChangeBtn_clicked()
{
    if(ui->ccValueLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("定电流值不能为空",this);
        toast->showToast(1000);
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetConstantCurrentCommand(ui->ccValueLineEdit->text().toFloat()));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantCurrentCommand());
    });

}
// 修改定电压值
void ITPlus::on_cvChangeBtn_clicked()
{
    if(ui->cvValueLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("定电压值不能为空",this);
        toast->showToast(1000);
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetConstantVoltageCommand(ui->cvValueLineEdit->text().toFloat()));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantVoltageCommand());
    });
}
// 修改定功率值
void ITPlus::on_cwChangeBtn_clicked()
{
    if(ui->cwValueLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("定功率值不能为空",this);
        toast->showToast(1000);
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetConstantPowerCommand(ui->cwValueLineEdit->text().toFloat()));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantPowerCommand());
    });
}
// 修改定电阻值
void ITPlus::on_crChangeBtn_clicked()
{
    if(ui->crValueLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("定电阻值不能为空",this);
        toast->showToast(1000);
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetConstantResistanceCommand(ui->crValueLineEdit->text().toFloat()));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetConstantResistanceCommand());
    });
}
// 修改动态电流值
void ITPlus::on_dcValueChangeBtn_clicked()
{
    if(ui->dcAValueLineEdit->text().isEmpty() || ui->dcATimeLineEdit->text().isEmpty()
            || ui->dcATimeLineEdit->text().isEmpty() || ui->dcBTimeLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("动态电流设置输入框不能为空",this);
        toast->showToast(1000);
        return;
    }
    QString modelName = ui->handleModelComboBox->currentText();
    quint8 modelIndex;
    if(modelName == "连续"){
        modelIndex = 0x00;
    } else if(modelName == "脉冲"){
        modelIndex = 0x01;
    } else{ // 触发
        modelIndex = 0x02;
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetDynamicCurrentParamsCommand(ui->dcAValueLineEdit->text().toFloat(),
                                                                             ui->dcATimeLineEdit->text().toFloat(),
                                                                             ui->dcBValueLineEdit->text().toFloat(),
                                                                             ui->dcBTimeLineEdit->text().toFloat(),
                                                                             modelIndex));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicCurrentParamsCommand());
    });
}
// 修改动态电压值
void ITPlus::on_dvValueChangeBtn_clicked()
{
    if(ui->dvAValueLineEdit->text().isEmpty() || ui->dvATimeLineEdit->text().isEmpty()
            || ui->dvATimeLineEdit->text().isEmpty() || ui->dvBTimeLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("动态电压设置输入框不能为空",this);
        toast->showToast(1000);
        return;
    }
    QString modelName = ui->handleModelComboBox->currentText();
    quint8 modelIndex;
    if(modelName == "连续"){
        modelIndex = 0x00;
    } else if(modelName == "脉冲"){
        modelIndex = 0x01;
    } else{ // 触发
        modelIndex = 0x02;
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetDynamicVoltageParamsCommand(ui->dvAValueLineEdit->text().toFloat(),
                                                                             ui->dvATimeLineEdit->text().toFloat(),
                                                                             ui->dvBValueLineEdit->text().toFloat(),
                                                                             ui->dvBTimeLineEdit->text().toFloat(),
                                                                             modelIndex));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicVoltageParamsCommand());
    });
}
// 修改动态功率值
void ITPlus::on_dwValueChangeBtn_clicked()
{
    if(ui->dwAValueLineEdit->text().isEmpty() || ui->dwATimeLineEdit->text().isEmpty()
            || ui->dwATimeLineEdit->text().isEmpty() || ui->dwBTimeLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("动态功率设置输入框不能为空",this);
        toast->showToast(1000);
        return;
    }
    QString modelName = ui->handleModelComboBox->currentText();
    quint8 modelIndex;
    if(modelName == "连续"){
        modelIndex = 0x00;
    } else if(modelName == "脉冲"){
        modelIndex = 0x01;
    } else{ // 触发
        modelIndex = 0x02;
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetDynamicPowerParamsCommand(ui->dwAValueLineEdit->text().toFloat(),
                                                                           ui->dwATimeLineEdit->text().toFloat(),
                                                                           ui->dwBValueLineEdit->text().toFloat(),
                                                                           ui->dwBTimeLineEdit->text().toFloat(),
                                                                           modelIndex));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicPowerParamsCommand());
    });
}
// 修改动态电阻值
void ITPlus::on_drValueChangeBtn_clicked()
{
    if(ui->drAValueLineEdit->text().isEmpty() || ui->drATimeLineEdit->text().isEmpty()
            || ui->drATimeLineEdit->text().isEmpty() || ui->drBTimeLineEdit->text().isEmpty()){
        ToastMessage *toast = new ToastMessage("动态电阻设置输入框不能为空",this);
        toast->showToast(1000);
        return;
    }
    QString modelName = ui->handleModelComboBox->currentText();
    quint8 modelIndex;
    if(modelName == "连续"){
        modelIndex = 0x00;
    } else if(modelName == "脉冲"){
        modelIndex = 0x01;
    } else{ // 触发
        modelIndex = 0x02;
    }
    serialUtil->sendData(eleLoad_ITPlus.createSetDynamicResistanceParamsCommand(ui->drAValueLineEdit->text().toFloat(),
                                                                                ui->drATimeLineEdit->text().toFloat(),
                                                                                ui->drBValueLineEdit->text().toFloat(),
                                                                                ui->drBTimeLineEdit->text().toFloat(),
                                                                                modelIndex));
    QTimer::singleShot(500,[this](){
        serialUtil->sendData(eleLoad_ITPlus.createGetDynamicResistanceParamsCommand());
    });
}

// 处理回车信号
void ITPlus::maxValueHandleEnter()
{
    on_changeBtn_clicked();
}

void ITPlus::tabWidgetHandleEnter()
{
    if(ui->tabWidget->currentIndex() == 0){ // 基础模式
        if(ui->baseModelStackedWidget->currentIndex() == 0){
            on_ccChangeBtn_clicked();
        } else if(ui->baseModelStackedWidget->currentIndex() == 1){
            on_cvChangeBtn_clicked();
        } else if(ui->baseModelStackedWidget->currentIndex() == 2){
            on_cwChangeBtn_clicked();
        } else if(ui->baseModelStackedWidget->currentIndex() == 3){
            on_crChangeBtn_clicked();
        }
    }else if(ui->tabWidget->currentIndex() == 1){   // 动态模式
        if(ui->dynamicModelStackedWidget->currentIndex() == 0){
            on_dcValueChangeBtn_clicked();
        } else if(ui->dynamicModelStackedWidget->currentIndex() == 1){
            on_dvValueChangeBtn_clicked();
        } else if(ui->dynamicModelStackedWidget->currentIndex() == 2){
            on_dwValueChangeBtn_clicked();
        } else if(ui->dynamicModelStackedWidget->currentIndex() == 3){
            on_drValueChangeBtn_clicked();
        }
    }
}

// 触发按钮槽函数
void ITPlus::on_handleBtn_clicked()
{
    // 发送触发信号
//    serialUtil->sendData(eleLoad_ITPlus.createBusTriggerSignal());    // 无法执行
    serialUtil->sendData(eleLoad_ITPlus.createNewTriggerSignal());
//    serialUtil->sendData(eleLoad_ITPlus.createAnalogKeyboardPress(16));   // shift
//    serialUtil->sendData(eleLoad_ITPlus.createAnalogKeyboardPress(98));   // 2
}

// 开始测试按钮槽函数
void ITPlus::on_measureBtn_clicked()
{
    measureEleLoad->start(1000);    // 启动定时器，每秒获取数据
}
