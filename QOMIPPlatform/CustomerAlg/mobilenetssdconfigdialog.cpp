#include "mobilenetssdconfigdialog.h"
#include "../resourceextractor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QGridLayout>
#include <QSplitter>
#include <QHeaderView>

MobileNetSSDConfigDialog::MobileNetSSDConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("MobileNet SSD 目标检测配置");
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(600, 500);
    resize(800, 600);
    
    // 初始化COCO类别列表
    m_cocoClasses = getCOCOClassNames();
    
    setupUI();
    
    // 设置默认配置
    setConfiguration(MobileNetSSDConfig());
}

MobileNetSSDConfigDialog::~MobileNetSSDConfigDialog()
{
}

void MobileNetSSDConfigDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    
    setupFileSelection();
    setupPreprocessingParams();
    setupPostprocessingParams();
    setupPerformanceParams();
    setupSceneParams();
    setupDisplayParams();
    setupButtons();
    
    m_mainLayout->addWidget(m_tabWidget);
    
    setLayout(m_mainLayout);
}

void MobileNetSSDConfigDialog::setupFileSelection()
{
    m_fileTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_fileTab);
    
    // 内置模型选项
    m_useBuiltinModelCheckBox = new QCheckBox("使用内置 MobileNet SSD 模型（外部文件版本）");
    m_useBuiltinModelCheckBox->setChecked(true);
    m_useBuiltinModelCheckBox->setStyleSheet("QCheckBox { font-weight: bold; color: #2196F3; }");
    layout->addRow(m_useBuiltinModelCheckBox);
    
    // 分隔线
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addRow(line);
    
    // 自定义模型标签
    QLabel* customLabel = new QLabel("或选择自定义模型文件:");
    customLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addRow(customLabel);
    
    // 模型文件选择
    QHBoxLayout* modelLayout = new QHBoxLayout();
    m_modelPathEdit = new QLineEdit();
    m_modelPathEdit->setPlaceholderText("选择 frozen_inference_graph.pb 文件...");
    m_browseModelButton = new QPushButton("浏览");
    m_browseModelButton->setFixedWidth(80);
    modelLayout->addWidget(m_modelPathEdit);
    modelLayout->addWidget(m_browseModelButton);
    
    // 配置文件选择
    QHBoxLayout* configLayout = new QHBoxLayout();
    m_configPathEdit = new QLineEdit();
    m_configPathEdit->setPlaceholderText("选择 ssd_mobilenet_v2_coco_2018_03_29.pbtxt 文件...");
    m_browseConfigButton = new QPushButton("浏览");
    m_browseConfigButton->setFixedWidth(80);
    configLayout->addWidget(m_configPathEdit);
    configLayout->addWidget(m_browseConfigButton);
    
    // 状态标签
    m_modelStatusLabel = new QLabel("将使用内置 MobileNet SSD 模型");
    m_modelStatusLabel->setStyleSheet("QLabel { color: green; font-style: italic; }");
    
    layout->addRow("模型文件 (.pb):", modelLayout);
    layout->addRow("配置文件 (.pbtxt):", configLayout);
    layout->addRow("状态:", m_modelStatusLabel);
    
    // 说明文本
    QLabel* infoLabel = new QLabel();
    infoLabel->setText(
        "MobileNet SSD 是一个轻量级的目标检测模型，支持 COCO 数据集的 80 个类别。\\n\\n"
        "所需文件：\\n"
        "• frozen_inference_graph.pb - TensorFlow 模型文件\\n"
        "• ssd_mobilenet_v2_coco_2018_03_29.pbtxt - 网络配置文件\\n\\n"
        "支持检测：人、车辆、动物、日常物品等 80 个类别"
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }");
    layout->addRow(infoLabel);
    
    // 连接信号
    connect(m_useBuiltinModelCheckBox, &QCheckBox::toggled, this, &MobileNetSSDConfigDialog::onUseBuiltinModelToggled);
    connect(m_browseModelButton, &QPushButton::clicked, this, &MobileNetSSDConfigDialog::onBrowseModelFile);
    connect(m_browseConfigButton, &QPushButton::clicked, this, &MobileNetSSDConfigDialog::onBrowseConfigFile);
    
    m_tabWidget->addTab(m_fileTab, "文件选择");
}

void MobileNetSSDConfigDialog::setupPreprocessingParams()
{
    m_preprocessTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_preprocessTab);
    
    // 输入尺寸（固定）
    m_inputSizeLabel = new QLabel("300 × 300 像素");
    m_inputSizeLabel->setStyleSheet("font-weight: bold;");
    
    // 均值减法（固定）
    m_meanLabel = new QLabel("R: 127.5, G: 127.5, B: 127.5");
    
    // 缩放因子（固定）
    m_scaleLabel = new QLabel("1/127.5 ≈ 0.00787");
    
    // BGR->RGB转换
    m_swapRBCheckBox = new QCheckBox("启用 BGR → RGB 转换");
    m_swapRBCheckBox->setChecked(true);
    m_swapRBCheckBox->setEnabled(false);  // MobileNet SSD 必须启用
    
    layout->addRow("输入尺寸:", m_inputSizeLabel);
    layout->addRow("均值减法:", m_meanLabel);
    layout->addRow("缩放因子:", m_scaleLabel);
    layout->addRow("", m_swapRBCheckBox);
    
    // 预处理说明
    QLabel* preprocessInfo = new QLabel();
    preprocessInfo->setText(
        "预处理参数说明：\\n\\n"
        "• 输入尺寸：MobileNet SSD 要求固定的 300×300 像素输入\\n"
        "• 均值减法：减去 RGB 三个通道的均值 127.5\\n"
        "• 缩放因子：将像素值从 [0,255] 归一化到 [-1,1] 范围\\n"
        "• 通道转换：OpenCV 默认 BGR，模型期望 RGB 格式\\n\\n"
        "这些参数针对 MobileNet SSD 模型优化，无需修改。"
    );
    preprocessInfo->setWordWrap(true);
    preprocessInfo->setStyleSheet("QLabel { background-color: #e8f4fd; padding: 10px; border-radius: 5px; }");
    layout->addRow(preprocessInfo);
    
    m_tabWidget->addTab(m_preprocessTab, "预处理参数");
}

void MobileNetSSDConfigDialog::setupPostprocessingParams()
{
    m_postprocessTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_postprocessTab);
    
    // 置信度阈值
    QHBoxLayout* confLayout = new QHBoxLayout();
    m_confidenceSpinBox = new QDoubleSpinBox();
    m_confidenceSpinBox->setRange(0.0, 1.0);
    m_confidenceSpinBox->setSingleStep(0.05);
    m_confidenceSpinBox->setDecimals(2);
    m_confidenceSpinBox->setValue(0.5);
    
    m_confidenceSlider = new QSlider(Qt::Horizontal);
    m_confidenceSlider->setRange(0, 100);
    m_confidenceSlider->setValue(50);
    
    confLayout->addWidget(m_confidenceSpinBox);
    confLayout->addWidget(m_confidenceSlider);
    
    // NMS阈值
    QHBoxLayout* nmsLayout = new QHBoxLayout();
    m_nmsSpinBox = new QDoubleSpinBox();
    m_nmsSpinBox->setRange(0.0, 1.0);
    m_nmsSpinBox->setSingleStep(0.05);
    m_nmsSpinBox->setDecimals(2);
    m_nmsSpinBox->setValue(0.4);
    
    m_nmsSlider = new QSlider(Qt::Horizontal);
    m_nmsSlider->setRange(0, 100);
    m_nmsSlider->setValue(40);
    
    nmsLayout->addWidget(m_nmsSpinBox);
    nmsLayout->addWidget(m_nmsSlider);
    
    // 启用NMS
    m_useNMSCheckBox = new QCheckBox("启用非极大值抑制 (NMS)");
    m_useNMSCheckBox->setChecked(true);
    
    // 最大检测数量
    m_maxDetectionsSpinBox = new QSpinBox();
    m_maxDetectionsSpinBox->setRange(1, 1000);
    m_maxDetectionsSpinBox->setValue(100);
    
    layout->addRow("置信度阈值:", confLayout);
    layout->addRow("NMS 阈值:", nmsLayout);
    layout->addRow("", m_useNMSCheckBox);
    layout->addRow("最大检测数:", m_maxDetectionsSpinBox);
    
    // 连接信号，同步滑块和数值框
    connect(m_confidenceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MobileNetSSDConfigDialog::onConfidenceChanged);
    connect(m_confidenceSlider, &QSlider::valueChanged, 
            this, [this](int value) { m_confidenceSpinBox->setValue(value / 100.0); });
    
    connect(m_nmsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MobileNetSSDConfigDialog::onNMSChanged);
    connect(m_nmsSlider, &QSlider::valueChanged,
            this, [this](int value) { m_nmsSpinBox->setValue(value / 100.0); });
    
    m_tabWidget->addTab(m_postprocessTab, "结果过滤");
}

void MobileNetSSDConfigDialog::setupPerformanceParams()
{
    m_performanceTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_performanceTab);
    
    // 计算后端选择
    m_backendCombo = new QComboBox();
    m_backendCombo->addItem("OpenCV (默认)", static_cast<int>(cv::dnn::DNN_BACKEND_OPENCV));
    m_backendCombo->addItem("Intel Inference Engine", static_cast<int>(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE));
    m_backendCombo->addItem("CUDA", static_cast<int>(cv::dnn::DNN_BACKEND_CUDA));
    
    // 目标设备选择
    m_targetCombo = new QComboBox();
    m_targetCombo->addItem("CPU", static_cast<int>(cv::dnn::DNN_TARGET_CPU));
    m_targetCombo->addItem("GPU (OpenCL)", static_cast<int>(cv::dnn::DNN_TARGET_OPENCL));
    m_targetCombo->addItem("GPU (CUDA)", static_cast<int>(cv::dnn::DNN_TARGET_CUDA));
    
    // 网络优化
    m_enableOptimizationCheckBox = new QCheckBox("启用网络优化 (推荐)");
    m_enableOptimizationCheckBox->setChecked(true);
    
    // 性能信息标签
    m_performanceInfoLabel = new QLabel();
    m_performanceInfoLabel->setWordWrap(true);
    
    layout->addRow("计算后端:", m_backendCombo);
    layout->addRow("目标设备:", m_targetCombo);
    layout->addRow("", m_enableOptimizationCheckBox);
    layout->addRow("性能说明:", m_performanceInfoLabel);
    
    // 连接信号
    connect(m_backendCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MobileNetSSDConfigDialog::onBackendChanged);
    
    // 更新性能信息
    updatePerformanceInfo();
    
    m_tabWidget->addTab(m_performanceTab, "性能优化");
}

void MobileNetSSDConfigDialog::setupSceneParams()
{
    m_sceneTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(m_sceneTab);
    
    // 人员检测模式
    m_personOnlyCheckBox = new QCheckBox("仅检测人员 (Person Detection Mode)");
    m_personOnlyCheckBox->setToolTip("启用后只检测人员，忽略其他类别");
    
    layout->addWidget(m_personOnlyCheckBox);
    
    // 类别选择
    QLabel* classLabel = new QLabel("启用的检测类别:");
    classLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(classLabel);
    
    // 类别操作按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_selectAllButton = new QPushButton("全选");
    m_selectNoneButton = new QPushButton("清空");
    m_selectCommonButton = new QPushButton("常用类别");
    
    buttonLayout->addWidget(m_selectAllButton);
    buttonLayout->addWidget(m_selectNoneButton);
    buttonLayout->addWidget(m_selectCommonButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    
    // 类别列表
    m_classListWidget = new QListWidget();
    layout->addWidget(m_classListWidget);
    
    // 连接信号
    connect(m_personOnlyCheckBox, &QCheckBox::toggled, this, &MobileNetSSDConfigDialog::onPersonOnlyToggled);
    connect(m_selectAllButton, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < m_classListWidget->count(); ++i) {
            m_classListWidget->item(i)->setCheckState(Qt::Checked);
        }
    });
    connect(m_selectNoneButton, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < m_classListWidget->count(); ++i) {
            m_classListWidget->item(i)->setCheckState(Qt::Unchecked);
        }
    });
    connect(m_selectCommonButton, &QPushButton::clicked, this, [this]() {
        QStringList commonClasses = {"person", "car", "motorbike", "bus", "truck", "bicycle"};
        for (int i = 0; i < m_classListWidget->count(); ++i) {
            QListWidgetItem* item = m_classListWidget->item(i);
            bool isCommon = commonClasses.contains(item->text());
            item->setCheckState(isCommon ? Qt::Checked : Qt::Unchecked);
        }
    });
    
    updateClassList();
    
    m_tabWidget->addTab(m_sceneTab, "场景适配");
}

void MobileNetSSDConfigDialog::setupDisplayParams()
{
    m_displayTab = new QWidget();
    QFormLayout* layout = new QFormLayout(m_displayTab);
    
    // 显示标签
    m_showLabelsCheckBox = new QCheckBox("显示类别标签");
    m_showLabelsCheckBox->setChecked(true);
    
    // 显示置信度
    m_showConfidenceCheckBox = new QCheckBox("显示置信度百分比");
    m_showConfidenceCheckBox->setChecked(true);
    
    // 字体大小
    m_fontSizeSpinBox = new QSpinBox();
    m_fontSizeSpinBox->setRange(8, 24);
    m_fontSizeSpinBox->setValue(12);
    m_fontSizeSpinBox->setSuffix(" px");
    
    layout->addRow("", m_showLabelsCheckBox);
    layout->addRow("", m_showConfidenceCheckBox);
    layout->addRow("字体大小:", m_fontSizeSpinBox);
    
    m_tabWidget->addTab(m_displayTab, "显示设置");
}

void MobileNetSSDConfigDialog::setupButtons()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_testButton = new QPushButton("测试模型");
    m_resetButton = new QPushButton("重置默认");
    m_cancelButton = new QPushButton("取消");
    m_okButton = new QPushButton("确定");
    
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_testButton);
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_testButton, &QPushButton::clicked, this, &MobileNetSSDConfigDialog::onTestModel);
    connect(m_resetButton, &QPushButton::clicked, this, &MobileNetSSDConfigDialog::onResetToDefaults);
    connect(m_cancelButton, &QPushButton::clicked, this, &MobileNetSSDConfigDialog::onReject);
    connect(m_okButton, &QPushButton::clicked, this, &MobileNetSSDConfigDialog::onAccept);
}

void MobileNetSSDConfigDialog::updateClassList()
{
    m_classListWidget->clear();
    
    for (const QString& className : m_cocoClasses) {
        QListWidgetItem* item = new QListWidgetItem(className);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        m_classListWidget->addItem(item);
    }
}

void MobileNetSSDConfigDialog::updatePerformanceInfo()
{
    QString info;
    int backendIndex = m_backendCombo->currentIndex();
    
    switch (backendIndex) {
        case 0: // OpenCV
            info = "OpenCV 后端：适用于所有平台，CPU 计算，稳定性好。";
            break;
        case 1: // Inference Engine
            info = "Intel Inference Engine：适用于 Intel CPU/GPU，性能优化，需要安装 OpenVINO。";
            break;
        case 2: // CUDA
            info = "CUDA 后端：适用于 NVIDIA GPU，高性能计算，需要安装 CUDA 和 cuDNN。";
            break;
    }
    
    m_performanceInfoLabel->setText(info);
}

QStringList MobileNetSSDConfigDialog::getCOCOClassNames()
{
    return QStringList{
        "person", "bicycle", "car", "motorbike", "aeroplane", "bus", "train", "truck",
        "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
        "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
        "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard",
        "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
        "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza",
        "donut", "cake", "chair", "sofa", "pottedplant", "bed", "diningtable", "toilet",
        "tvmonitor", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave",
        "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors",
        "teddy bear", "hair drier", "toothbrush"
    };
}

// 槽函数实现
void MobileNetSSDConfigDialog::onBrowseModelFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "选择 MobileNet SSD 模型文件", 
        QString(), "TensorFlow 模型 (*.pb);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        m_modelPathEdit->setText(fileName);
        
        QFileInfo fileInfo(fileName);
        if (fileInfo.exists()) {
            m_modelStatusLabel->setText(QString("模型文件: %1 (%.1f MB)")
                                      .arg(fileInfo.fileName())
                                      .arg(fileInfo.size() / (1024.0 * 1024.0)));
            m_modelStatusLabel->setStyleSheet("QLabel { color: green; }");
        }
    }
}

void MobileNetSSDConfigDialog::onBrowseConfigFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "选择 MobileNet SSD 配置文件", 
        QString(), "配置文件 (*.pbtxt);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        m_configPathEdit->setText(fileName);
    }
}

void MobileNetSSDConfigDialog::onUseBuiltinModelToggled(bool enabled)
{
    // 启用/禁用自定义文件选择控件
    m_modelPathEdit->setEnabled(!enabled);
    m_configPathEdit->setEnabled(!enabled);
    m_browseModelButton->setEnabled(!enabled);
    m_browseConfigButton->setEnabled(!enabled);
    
    if (enabled) {
        // 使用内置模型
        m_modelStatusLabel->setText("将使用内置 MobileNet SSD 模型");
        m_modelStatusLabel->setStyleSheet("QLabel { color: green; font-style: italic; }");
        
        // 清空自定义路径
        m_modelPathEdit->clear();
        m_configPathEdit->clear();
    } else {
        // 使用自定义模型
        m_modelStatusLabel->setText("请选择自定义模型文件");
        m_modelStatusLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");
    }
}

void MobileNetSSDConfigDialog::onBackendChanged()
{
    updatePerformanceInfo();
}

void MobileNetSSDConfigDialog::onPersonOnlyToggled(bool enabled)
{
    m_classListWidget->setEnabled(!enabled);
    m_selectAllButton->setEnabled(!enabled);
    m_selectNoneButton->setEnabled(!enabled);
    m_selectCommonButton->setEnabled(!enabled);
    
    if (enabled) {
        // 人员检测模式，只启用person类别
        for (int i = 0; i < m_classListWidget->count(); ++i) {
            QListWidgetItem* item = m_classListWidget->item(i);
            item->setCheckState(item->text() == "person" ? Qt::Checked : Qt::Unchecked);
        }
    }
}

void MobileNetSSDConfigDialog::onClassSelectionChanged()
{
    // 处理类别选择变化，这里可以添加实时预览或其他逻辑
    // 目前不需要特殊处理，类别选择会在getConfiguration()中读取
}

void MobileNetSSDConfigDialog::onConfidenceChanged(double value)
{
    m_confidenceSlider->setValue(static_cast<int>(value * 100));
}

void MobileNetSSDConfigDialog::onNMSChanged(double value)
{
    m_nmsSlider->setValue(static_cast<int>(value * 100));
}

void MobileNetSSDConfigDialog::onResetToDefaults()
{
    setConfiguration(MobileNetSSDConfig());
}

void MobileNetSSDConfigDialog::onTestModel()
{
    if (!validateConfiguration()) {
        return;
    }
    
    QMessageBox::information(this, "测试模型", "模型测试功能正在开发中...");
}

void MobileNetSSDConfigDialog::onAccept()
{
    if (validateConfiguration()) {
        accept();
    }
}

void MobileNetSSDConfigDialog::onReject()
{
    reject();
}

bool MobileNetSSDConfigDialog::validateConfiguration()
{
    if (m_useBuiltinModelCheckBox->isChecked()) {
        // 使用内置模型，验证资源文件
        QString modelPath = ResourceExtractor::getDefaultModelPath();
        QString configPath = ResourceExtractor::getDefaultConfigPath();
        
        if (modelPath.isEmpty() || configPath.isEmpty()) {
            QMessageBox::critical(this, "资源错误", "无法提取内置 MobileNet SSD 模型文件！\n\n请检查应用程序资源文件是否完整。");
            return false;
        }
        
        if (!QFileInfo::exists(modelPath) || !QFileInfo::exists(configPath)) {
            QMessageBox::critical(this, "资源错误", "内置模型文件提取失败！");
            return false;
        }
        
        return true;
    } else {
        // 使用自定义模型，验证用户选择的文件
        if (m_modelPathEdit->text().isEmpty()) {
            QMessageBox::warning(this, "配置错误", "请选择模型文件 (.pb)！");
            m_tabWidget->setCurrentIndex(0);  // 切换到文件选择标签页
            return false;
        }
        
        if (m_configPathEdit->text().isEmpty()) {
            QMessageBox::warning(this, "配置错误", "请选择配置文件 (.pbtxt)！");
            m_tabWidget->setCurrentIndex(0);
            return false;
        }
        
        // 检查文件是否存在
        if (!QFileInfo::exists(m_modelPathEdit->text())) {
            QMessageBox::warning(this, "文件错误", "模型文件不存在！");
            return false;
        }
        
        if (!QFileInfo::exists(m_configPathEdit->text())) {
            QMessageBox::warning(this, "文件错误", "配置文件不存在！");
            return false;
        }
        
        return true;
    }
}

MobileNetSSDConfigDialog::MobileNetSSDConfig MobileNetSSDConfigDialog::getConfiguration() const
{
    MobileNetSSDConfig config = m_config;
    
    // 更新内置模型选项和文件路径
    config.useBuiltinModel = m_useBuiltinModelCheckBox->isChecked();
    if (config.useBuiltinModel) {
        // 使用内置模型路径
        config.modelPath = ResourceExtractor::getDefaultModelPath();
        config.configPath = ResourceExtractor::getDefaultConfigPath();
    } else {
        // 使用自定义路径
        config.modelPath = m_modelPathEdit->text();
        config.configPath = m_configPathEdit->text();
    }
    
    // 更新后处理参数
    config.confidenceThreshold = m_confidenceSpinBox->value();
    config.nmsThreshold = m_nmsSpinBox->value();
    config.useNMS = m_useNMSCheckBox->isChecked();
    config.maxDetections = m_maxDetectionsSpinBox->value();
    
    // 更新性能参数
    config.backend = static_cast<cv::dnn::Backend>(
        m_backendCombo->currentData().toInt());
    config.target = static_cast<cv::dnn::Target>(
        m_targetCombo->currentData().toInt());
    config.enableOptimization = m_enableOptimizationCheckBox->isChecked();
    
    // 更新场景参数
    config.personOnly = m_personOnlyCheckBox->isChecked();
    config.enabledClasses.clear();
    for (int i = 0; i < m_classListWidget->count(); ++i) {
        QListWidgetItem* item = m_classListWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            config.enabledClasses.append(item->text());
        }
    }
    
    // 更新显示参数
    config.showLabels = m_showLabelsCheckBox->isChecked();
    config.showConfidence = m_showConfidenceCheckBox->isChecked();
    config.fontSize = m_fontSizeSpinBox->value();
    
    return config;
}

void MobileNetSSDConfigDialog::setConfiguration(const MobileNetSSDConfig& config)
{
    m_config = config;
    
    // 设置内置模型选项
    m_useBuiltinModelCheckBox->setChecked(config.useBuiltinModel);
    
    // 设置文件路径（仅当使用自定义模型时）
    if (!config.useBuiltinModel) {
        m_modelPathEdit->setText(config.modelPath);
        m_configPathEdit->setText(config.configPath);
    }
    
    // 设置后处理参数
    m_confidenceSpinBox->setValue(config.confidenceThreshold);
    m_nmsSpinBox->setValue(config.nmsThreshold);
    m_useNMSCheckBox->setChecked(config.useNMS);
    m_maxDetectionsSpinBox->setValue(config.maxDetections);
    
    // 设置性能参数
    for (int i = 0; i < m_backendCombo->count(); ++i) {
        if (m_backendCombo->itemData(i).toInt() == static_cast<int>(config.backend)) {
            m_backendCombo->setCurrentIndex(i);
            break;
        }
    }
    
    for (int i = 0; i < m_targetCombo->count(); ++i) {
        if (m_targetCombo->itemData(i).toInt() == static_cast<int>(config.target)) {
            m_targetCombo->setCurrentIndex(i);
            break;
        }
    }
    m_enableOptimizationCheckBox->setChecked(config.enableOptimization);
    
    // 设置场景参数
    m_personOnlyCheckBox->setChecked(config.personOnly);
    
    // 设置类别选择
    for (int i = 0; i < m_classListWidget->count(); ++i) {
        QListWidgetItem* item = m_classListWidget->item(i);
        bool enabled = config.enabledClasses.contains(item->text());
        item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    }
    
    // 设置显示参数
    m_showLabelsCheckBox->setChecked(config.showLabels);
    m_showConfidenceCheckBox->setChecked(config.showConfidence);
    m_fontSizeSpinBox->setValue(config.fontSize);
    
    updatePerformanceInfo();
}