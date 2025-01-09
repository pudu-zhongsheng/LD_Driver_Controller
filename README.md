<<<<<<< HEAD
# LD_Driver_Controller
LED驱动及电子负载、照度计控制测试系统
=======
# LD Driver Controller

## 项目概述

LD Driver Controller 是一个基于 Qt 的应用程序，用于与电子负载、照度计和设备驱动进行串口通讯。该项目支持不同型号的设备，并提供数据读取、记录和图表展示功能。

## 功能模块

### 1. 基本界面

- **启动画面 (SplashScreen)**: 显示应用程序启动时的加载进度。
- **驱动选择菜单 (DriveMenu)**: 允许用户选择当前连接的驱动、电子负载和照度计型号，并进入主界面。
- **主界面 (MainWindow)**: 负责整体界面布局和功能协调。

### 2. 主界面管理 (MainWindow)

- **驱动控制板块 (Driver)**: 根据选择的驱动型号实例化相应的驱动类并添加到界面。
- **通讯连接状态管理**: 管理驱动、电子负载、照度计的串口连接状态。
- **电子负载板块 (ElectronicLoad)**: 实时显示电子负载的电压、电流、功率，并提供跳转按钮。
- **软件设置方案模块**: 提供数据方案的读取、加载和保存功能。
- **图表功能**: 提供数据图表展示和导出功能。
- **返回按钮**: 清理缓存与连接，返回驱动选择菜单。

### 3. 串口通信管理 (SerialUtil)

- 管理设备连接
- 处理数据收发
- 错误处理和重连机制

### 4. 设备通信模块

- **驱动通讯 (DriverCom)**: 预留实现
- **电子负载通信 (EleLoad)**: 支持 IT8512+ 型号，提供参数设置和读取功能。
- **照度计通信 (Illuminometer)**: 支持 CL-200A 型号，提供参数测量功能。

## 技术栈

- **编程语言**: C++
- **框架**: Qt
- **标准**: C++11
- **工具**: Qt Creator, CMake

## 目录结构 
project/
├── splash/
│ ├── splashscreen.h
│ └── splashscreen.cpp
├── menu/
│ ├── drivemenu.h
│ └── drivemenu.cpp
├── driver/
│ └── (驱动相关类)
├── communication/
│ └── (通信相关类)
├── util/
│ └── ToastMessage.h
└── resources/
├── images/
│ └── splash.png
└── styles/
└── style.qss


## 更新日志

- **v1.0**: 初始版本，包含基本界面和串口通信管理功能。

请在完成关键节点后及时更新此文档。
>>>>>>> 530e40f (first commit)
