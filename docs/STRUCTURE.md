# LD Driver Controller 项目结构

## 项目概述

LD Driver Controller是一个基于Qt框架开发的驱动控制器应用程序，用于管理和控制不同类型的设备驱动、电子负载和照度计。该应用程序提供了直观的用户界面，实时数据监控，通讯连接管理，数据采集与分析等功能。

## 目录结构
LD_Driver_Controller/
├── communication/                  // 通信协议实现
│ ├── drivergeneral.cpp             // 通用驱动通信实现
│ ├── drivergeneral.h               // 通用驱动通信接口
│ ├── eleload_itplus.cpp            // 电子负载IT8512+通信实现
│ ├── eleload_itplus.h              // 电子负载通信接口
│ ├── cl_twozerozeroacom.cpp        // CL-200A照度计通信实现
│ ├── cl_twozerozeroacom.h          // 照度计通信接口
│ ├── driverprotocol.cpp            // 驱动协议实现
│ ├── driverprotocol.h              // 驱动协议接口
│ ├── protocol.cpp                  // 基础协议实现
│ └── protocol.h                    // 基础协议接口
├── chart/                          // 图表工具
│ ├── chartwidget.cpp               // 图表绘制工具实现
│ └── chartwidget.h                 // 图表绘制工具接口
├── devices/                        // 设备控制模块
│ ├── driver/                       // 驱动控制
│ │ ├── driver8ch/                  // 8通道驱动实现
│ │ │ └── driver8ch.h/cpp
│ │ ├── driverbase.h/cpp            // 驱动基类
│ │ ├── driverwidget.h/cpp          // 驱动控制UI组件
│ │ └── widgets/                    // 驱动UI子组件
│ │ ├── controlwidget.h/cpp         // 控制界面组件
│ │ ├── driverinfowidget.h/cpp      // 驱动信息组件
│ │ ├── scanwidget.h/cpp            // 扫描控制组件
│ │ ├── sliderwidget.h/cpp          // 滑块控制组件
│ │ └── paramtablewidget.h/cpp      // 参数表格组件
│ ├── load/                         // 电子负载控制
│ │ ├── it8512plus/                 // IT8512+型号实现
│ │ │ └── it8512plus_widget.h/cpp
│ │ └── load_base.h                 // 电子负载基类
│ └── meter/                        // 照度计控制
│ ├── cl200a/                       // CL-200A型号实现
│ │ └── cl200awidget.h/cpp
│ └── meterbase.h/cpp               // 照度计基类
├── docs/                           // 文档
│ ├── STRUCTURE.md                  // 项目结构文档
│ ├── DEPLOYMENT.md                 // 部署文档
│ ├── PROTOCOL.md                   // 协议文档
│ ├── TESTING.md                    // 测试文档
│ ├── USER_MANUAL.md                // 用户手册
│ ├── DEVELOPMENT.md                // 开发文档
│ └── README.md                     // 项目说明
├── logo/                           // 应用图标资源
│ └── LD_Control.ico
├── menu/                           // 菜单界面
│ ├── drivemenu.cpp                 // 驱动选择菜单实现
│ └── drivemenu.h                   // 驱动选择菜单接口
├── serial/                         // 串口通信
│ ├── serialutil.cpp                // 串口工具实现
│ └── serialutil.h                  // 串口工具接口
├── splash/                         // 启动界面
│ ├── splashscreen.cpp              // 启动画面实现
│ └── splashscreen.h                // 启动画面接口
├── styles/                         // 样式文件
│ ├── style.qss                     // 全局样式表
│ └── driver.qss                    // 驱动界面样式
├── util/                           // 工具类
│ ├── ToastMessage.h                // 提示消息组件
│ ├── config.cpp                    // 配置管理实现
│ ├── config.h                      // 配置管理接口
│ ├── datamanager.cpp               // 数据管理实现
│ ├── datamanager.h                 // 数据管理接口
│ ├── errorhandler.cpp              // 错误处理实现
│ ├── errorhandler.h                // 错误处理接口
│ ├── logger.cpp                    // 日志工具实现
│ └── logger.h                      // 日志工具接口
├── mainwindow.cpp                  // 主窗口实现
├── mainwindow.h                    // 主窗口接口
├── mainwindow.ui                   // 主窗口UI定义
├── main.cpp                        // 应用程序入口
├── LD_Driver_Controller.pro        // Qt项目文件
├── logo.qrc                        // Logo资源文件
└── resources.qrc                   // 应用资源文件

## 模块功能说明

### 1. 通信协议模块 (communication/)

管理与各种设备的通信协议实现：

- **Protocol**: 基础通信协议接口，定义了通信的基本方法
- **DriverProtocol**: 驱动器专用通信协议
- **EleLoad_ITPlus**: IT8512+电子负载通信协议实现
- **CL_TwoZeroZeroACom**: CL-200A照度计通信协议实现
- **DriverGeneral**: 通用驱动通信实现，支持多种通道数的驱动

### 2. 设备控制模块 (devices/)

包含各种设备的控制逻辑和界面组件：

#### 2.1 驱动控制 (driver/)

- **DriverBase**: 驱动器的基类，定义了驱动器的基本接口
- **Driver8CH**: 8通道驱动器的特定实现
- **DriverWidget**: 通用驱动控制界面，支持1、2、4、5、6、8、10、20通道的驱动控制
- 专用控制组件 (widgets/): 包含控制界面、信息展示、参数设置等子组件

#### 2.2 电子负载控制 (load/)

- **LoadBase**: 电子负载基类，定义了电子负载的基本接口
- **IT8512Plus_Widget**: IT8512+电子负载的具体实现和控制界面

#### 2.3 照度计控制 (meter/)

- **MeterBase**: 照度计基类，定义了照度计的基本接口
- **CL200AWidget**: CL-200A照度计的具体实现和控制界面

### 3. 用户界面模块

#### 3.1 启动界面 (splash/)

- **SplashScreen**: 应用启动时显示的加载画面，包含进度条指示应用加载状态

#### 3.2 菜单界面 (menu/)

- **DriveMenu**: 主菜单界面，用于选择驱动器、电子负载和照度计型号

#### 3.3 主界面 (mainwindow)

- **MainWindow**: 应用的主界面，整合了所有功能模块：
  - 驱动控制区域
  - 通讯连接状态管理
  - 电子负载状态显示
  - 软件设置方案管理
  - 数据图表显示和分析
  - 多种图表类型支持（电流-时间、电压/功率/电阻-时间、照度-时间、色温RGB-时间）
  - 数据导出和分析功能

### 4. 工具模块 (util/)

提供全局实用工具：

- **Config**: 应用配置管理，处理应用设置的保存和加载
- **DataManager**: 数据管理器，处理测量数据的存储、分析、导出和备份
- **Logger**: 日志系统，记录应用运行信息和错误
- **ErrorHandler**: 错误处理，提供统一的错误处理机制
- **ToastMessage**: 轻量级通知组件，用于显示临时提示消息

### 5. 串口通信 (serial/)

- **SerialUtil**: 串口通信工具，提供设备连接、数据收发、错误处理等功能

## 启动流程

1. 应用程序启动 (main.cpp)
2. 显示启动画面 (SplashScreen)
3. 初始化系统资源
4. 显示驱动选择菜单 (DriveMenu)
5. 用户选择设备型号后进入主界面 (MainWindow)
6. 主界面根据选择的设备型号创建对应的控制模块
7. 用户可以管理连接、控制设备、查看数据图表

## 数据流

1. 用户通过界面设置设备参数
2. 参数通过通信协议发送给设备
3. 设备返回状态和测量数据
4. 应用程序将数据显示在界面上，并保存到数据管理器
5. 数据管理器更新图表和表格
6. 用户可以导出、分析或备份数据

## 配置系统

应用程序使用Config模块管理配置，支持：

- 设备设置的保存和恢复
- 界面布局和首选项
- 不同通道数驱动器的独立配置

## 错误处理

系统使用以下机制处理错误：

- 集中式错误处理器 (ErrorHandler)
- 日志系统 (Logger)
- 用户界面通知 (ToastMessage)
- 连接超时处理
- 设备通信错误恢复

## 外观与样式

应用使用QSS样式表管理界面外观：
- style.qss: 全局样式定义
- driver.qss: 驱动界面专用样式