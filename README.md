# QOMIPPlatform - Qt OpenCV 多算法图像处理平台

## 项目简介

QOMIPPlatform是一个基于Qt6和OpenCV的多算法图像处理平台，支持实时视频处理、多种图像算法应用以及AI模型集成。该平台提供了可视化的图像处理流水线，支持多窗口显示、算法参数调节和视频导出功能。

## 技术栈

### 核心框架
- **Qt6** (6.5+) - GUI框架和多媒体支持
  - Qt Core - 核心功能
  - Qt Widgets - 传统GUI控件
  - Qt QuickWidgets - QML集成
  - Qt Multimedia - 多媒体处理
- **OpenCV** - 计算机视觉和图像处理库
- **CMake** - 构建系统

### 开发语言
- **C++** - 主要开发语言
- **QML** - 用户界面组件

## 项目架构

### 主要模块

#### 1. 核心视频处理模块
- **Reader** (`Reader.h/cpp`) - 视频读取工作类
  - 支持视频文件和摄像头输入
  - 多线程视频帧读取
  - 帧率控制和播放控制

#### 2. 图像处理框架
- **FrameProcessor** (`frameprocessor.h/cpp`) - 帧处理器
  - 多算法处理队列
  - 线程安全的算法管理
  - 实时图像处理流水线

- **Algorithm** (`Algorithms/algorithm.h`) - 算法基类
  - 统一的算法接口
  - 参数化配置支持
  - 算法深拷贝机制

#### 3. 用户界面模块
- **MainWindow** (`mainwindow.h/cpp`) - 主窗口
  - 多视频源管理
  - 摄像头设备管理
  - 视频导出功能
  - 拖拽文件支持

- **BasicViewWidget** (`basicviewwidget.h/cpp`) - 基础视图组件
  - 图像显示和缩放
  - 算法处理结果展示
  - 右键菜单操作

#### 4. 算法管理系统
- **AlgorithmListModel** (`algorithmlistmodel.h/cpp`) - 算法列表模型
- **AlgorithmFactory** (`Algorithms/algorithmfactory.h/cpp`) - 算法工厂
- 支持动态算法加载和参数调节

### 内置算法库 (Algorithms目录)

#### 基础图像处理
- **原始算法** (`originalalgorithm`) - 无处理直通
- **灰度转换** (`grayscalealgorithm`) - 彩色到灰度转换
- **模糊滤波** (`blurfilter`) - 图像模糊处理
- **中值滤波** (`medianblur`) - 噪声去除

#### 阈值化处理
- **阈值滤波** (`thresholdfilter`) - 基础阈值化
- **自适应阈值** (`adaptivethreshold`) - 局部自适应阈值
- **Otsu阈值** (`otsuthreshold`) - 自动阈值选择

#### 边缘检测
- **Sobel边缘检测** (`sobeledgedetector`) - 梯度边缘检测
- **Canny边缘检测** (`cannyedgedetector`) - 多级边缘检测

#### 形态学操作
- **形态学运算** (`morphologicaloperation`) - 开闭运算、腐蚀膨胀

#### 色彩处理
- **HSV颜色提取** (`hsvcolorextraction`) - 基于HSV色彩空间的颜色分割

#### 特征检测
- **ORB特征检测** (`orbfeaturedetector`) - 快速特征点检测
- **Haar人脸检测** (`haarfacedetector`) - 基于Haar特征的人脸识别
- **HOG行人检测** (`hogpedestriandetector`) - 行人检测

#### 运动分析
- **帧差法检测** (`framedifferencedetector`) - 运动目标检测
- **MOG2背景减除** (`mog2backgroundsubtractor`) - 背景建模
- **Farneback光流** (`farnebackopticalflow`) - 密集光流计算

#### 图像质量评估
- **模糊检测** (`blurdetector`) - 图像清晰度评估

### AI模型集成 (CustomerAlg目录)

#### 深度学习模型支持
- **MobileNet SSD** (`mobilenetssdprocessor`) - 目标检测
  - 人员、车辆等多类别检测
  - 实时检测结果可视化
  - 可配置的检测参数

- **AI模型处理器** (`aimodelprocessor`) - 通用AI模型加载框架
- **AI结果可视化** (`airesultvisualizer`) - 检测结果绘制和标注

## 主要功能

### 1. 多源视频输入
- 支持视频文件播放（拖拽加载）
- 摄像头实时采集
- 自动设备检测和管理

### 2. 实时图像处理
- 多窗口并行处理
- 算法链式组合
- 实时参数调节
- 处理结果即时预览

### 3. 算法管理
- 图形化算法选择界面
- 参数可视化编辑
- 算法动态加载
- 自定义算法导入

### 4. 视频导出
- 处理后视频导出
- 多窗口批量导出
- 进度显示和控制
- 支持多种视频格式

### 5. 界面功能
- 多窗口分离显示
- 图像缩放和查看
- 摄像头设备管理
- 直观的操作界面

## 处理流程

### 整体数据流
```
视频源输入 → Reader线程 → 帧分发 → 多个BasicViewWidget
                             ↓
算法参数配置 ← 用户界面 ← FrameProcessor ← 算法处理链
                             ↓
                        处理结果显示 → 视频导出
```

### 详细处理步骤

1. **输入阶段**
   - Reader从视频文件或摄像头读取帧
   - 帧数据通过信号传递给主窗口

2. **分发阶段**  
   - MainWindow将帧分发给各个BasicViewWidget
   - 每个Widget独立处理相同的输入帧

3. **处理阶段**
   - BasicViewWidget调用FrameProcessor处理帧
   - FrameProcessor按算法链顺序应用算法
   - 算法参数通过UI实时调节

4. **显示阶段**
   - 处理结果在GraphicsView中显示
   - 支持图像缩放和交互操作

5. **导出阶段**
   - 收集各窗口处理结果
   - VideoExporter执行视频编码和文件保存

## 编译和运行

### 环境要求
- Qt6.5或更高版本
- OpenCV 4.x
- CMake 3.19+
- 支持C++17的编译器

### 编译步骤
```bash
mkdir build
cd build
cmake ..
make
```

### 运行
```bash
./QOMIPPlatform
```

## 扩展开发

### 添加新算法
1. 继承Algorithm基类
2. 实现必要的虚函数
3. 在AlgorithmFactory中注册
4. 添加到CMakeLists.txt

### 集成新AI模型
1. 参考MobileNetSSDProcessor实现
2. 创建模型配置对话框
3. 实现结果可视化
4. 集成到主界面

## 特色功能

- **多算法流水线**: 支持多个算法串联处理
- **实时参数调节**: QML界面提供直观的参数控制
- **多窗口并行**: 同时查看不同算法效果
- **AI模型支持**: 内置深度学习模型集成框架
- **视频导出**: 完整的处理结果保存功能
- **设备管理**: 自动摄像头检测和切换

QOMIPPlatform为图像处理研究、算法开发和实际应用提供了一个完整的可视化平台。