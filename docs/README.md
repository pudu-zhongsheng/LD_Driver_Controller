# LD Driver Controller

## 项目概述
LED驱动控制器上位机是一个用于控制LED驱动器、电子负载和照度计的集成测试平台。支持多种设备协议,提供实时数据采集、分析和可视化功能。

## 系统架构

### 1. 通信层 (communication/)
- Protocol: 通信协议基类
- DriverProtocol: 驱动器通信协议
- EleLoad_ITPlus: 电子负载IT8512+协议
- CL_TwoZeroZeroACOM: 照度计CL-200A协议

### 2. 设备层 (devices/)
- 驱动控制模块 (driver/)
  - DriverBase: 驱动器基类
  - Driver8CH: 8通道驱动实现
  - 控制组件 (widgets/): 信息显示、参数控制、扫描测试等UI组件
- 电子负载模块 (load/)
  - LoadBase: 电子负载基类
  - IT8512Plus: IT8512+型号实现
- 照度计模块 (meter/)
  - MeterBase: 照度计基类
  - CL200A: CL-200A型号实现

### 3. 工具层 (util/)
- SerialUtil: 串口通信工具
- Config: 配置管理
- Logger: 日志记录
- DataManager: 数据管理
- ErrorHandler: 错误处理

### 4. 界面层 (ui/)
- MainWindow: 主窗口
- DriveMenu: 设备选择菜单
- SplashScreen: 启动画面

## 主要功能

### 1. 设备控制
- 驱动器:
  - 8通道独立控制
  - 高低电平切换
  - 扫描测试
  - 参数实时监控
- 电子负载:
  - CC/CV/CR/CW模式
  - 动态测试
  - 状态监控
- 照度计:
  - 照度/色温测量
  - 量程切换
  - 数据记录

### 2. 数据管理
- 实时数据采集
- 图表显示
- 数据导出
- 历史记录查询

### 3. 系统功能
- 多设备协同工作
- 错误处理和日志
- 配置管理
- 状态监控

## 开发环境
- Qt 5.14.2
- C++17
- MinGW 64-bit
- Windows 10/11

## 项目结构
project/
├── communication/ # 通信协议
├── devices/ # 设备控制
│ ├── driver/ # 驱动器
│ ├── load/ # 电子负载
│ └── meter/ # 照度计
├── util/ # 工具类
├── resources/ # 资源文件
└── docs/ # 文档

## 构建说明
1. 安装Qt 5.14.2及以上版本
2. 打开LD_Driver_Controller.pro
3. 配置构建套件
4. 编译运行

## 贡献指南
1. Fork项目
2. 创建特性分支
3. 提交更改
4. 发起Pull Request

## 许可证
MIT License