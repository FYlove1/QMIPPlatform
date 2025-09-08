#include "algorithmimportdialog.h"
#include <QGridLayout>

AlgorithmImportDialog::AlgorithmImportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("导入AI算法模型");
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(500, 400);
    
    setupUI();
    loadPresetParameters();
}

void AlgorithmImportDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    setupModelPathSection();
    setupBlobParametersSection();
    setupButtons();
    
    setLayout(m_mainLayout);
}

void AlgorithmImportDialog::setupModelPathSection()
{
    QGroupBox *modelGroup = new QGroupBox("模型文件设置", this);
    QFormLayout *formLayout = new QFormLayout(modelGroup);
    
    // 模型类型选择
    m_modelTypeCombo = new QComboBox(this);
    m_modelTypeCombo->addItems({
        "ONNX模型 (*.onnx)",
        "TensorFlow模型 (*.pb)",
        "Caffe模型 (*.caffemodel)",
        "PyTorch模型 (*.pt)",
        "Darknet模型 (*.weights)",
        "其他格式"
    });
    formLayout->addRow("模型类型:", m_modelTypeCombo);
    
    // 模型路径选择
    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_modelPathEdit = new QLineEdit(this);
    m_modelPathEdit->setPlaceholderText("请选择模型文件路径...");
    m_browseButton = new QPushButton("浏览", this);
    m_browseButton->setFixedWidth(80);
    
    pathLayout->addWidget(m_modelPathEdit);
    pathLayout->addWidget(m_browseButton);
    formLayout->addRow("模型路径:", pathLayout);
    
    // 连接信号
    connect(m_browseButton, &QPushButton::clicked, this, &AlgorithmImportDialog::onBrowseModel);
    connect(m_modelTypeCombo, &QComboBox::currentTextChanged, this, &AlgorithmImportDialog::onModelTypeChanged);
    
    m_mainLayout->addWidget(modelGroup);
}

void AlgorithmImportDialog::setupBlobParametersSection()
{
    QGroupBox *blobGroup = new QGroupBox("blobFromImage预处理参数", this);
    QGridLayout *gridLayout = new QGridLayout(blobGroup);
    
    // 输入尺寸
    QLabel *sizeLabel = new QLabel("输入尺寸:", this);
    m_widthSpinBox = new QSpinBox(this);
    m_widthSpinBox->setRange(1, 2048);
    m_widthSpinBox->setValue(416);
    m_widthSpinBox->setSuffix(" px");
    
    m_heightSpinBox = new QSpinBox(this);
    m_heightSpinBox->setRange(1, 2048);
    m_heightSpinBox->setValue(416);
    m_heightSpinBox->setSuffix(" px");
    
    QLabel *xLabel = new QLabel("×", this);
    xLabel->setAlignment(Qt::AlignCenter);
    
    gridLayout->addWidget(sizeLabel, 0, 0);
    gridLayout->addWidget(m_widthSpinBox, 0, 1);
    gridLayout->addWidget(xLabel, 0, 2);
    gridLayout->addWidget(m_heightSpinBox, 0, 3);
    
    // 缩放因子
    QLabel *scaleLabel = new QLabel("缩放因子:", this);
    m_scaleFactorSpinBox = new QDoubleSpinBox(this);
    m_scaleFactorSpinBox->setRange(0.0001, 10.0);
    m_scaleFactorSpinBox->setValue(1.0/255.0);
    m_scaleFactorSpinBox->setDecimals(6);
    m_scaleFactorSpinBox->setSingleStep(0.001);
    
    gridLayout->addWidget(scaleLabel, 1, 0);
    gridLayout->addWidget(m_scaleFactorSpinBox, 1, 1, 1, 3);
    
    // 均值减法 (Mean Subtraction)
    QLabel *meanLabel = new QLabel("均值减法 (R,G,B):", this);
    m_meanRSpinBox = new QDoubleSpinBox(this);
    m_meanRSpinBox->setRange(0.0, 255.0);
    m_meanRSpinBox->setValue(0.0);
    m_meanRSpinBox->setDecimals(2);
    
    m_meanGSpinBox = new QDoubleSpinBox(this);
    m_meanGSpinBox->setRange(0.0, 255.0);
    m_meanGSpinBox->setValue(0.0);
    m_meanGSpinBox->setDecimals(2);
    
    m_meanBSpinBox = new QDoubleSpinBox(this);
    m_meanBSpinBox->setRange(0.0, 255.0);
    m_meanBSpinBox->setValue(0.0);
    m_meanBSpinBox->setDecimals(2);
    
    gridLayout->addWidget(meanLabel, 2, 0);
    gridLayout->addWidget(m_meanRSpinBox, 2, 1);
    gridLayout->addWidget(m_meanGSpinBox, 2, 2);
    gridLayout->addWidget(m_meanBSpinBox, 2, 3);
    
    // BGR->RGB交换
    m_swapRBCheckBox = new QCheckBox("交换红蓝通道 (BGR->RGB)", this);
    m_swapRBCheckBox->setChecked(true);
    gridLayout->addWidget(m_swapRBCheckBox, 3, 0, 1, 4);
    
    // 添加说明标签
    QLabel *hintLabel = new QLabel(
        "提示：这些参数用于OpenCV的blobFromImage()函数\n"
        "• 输入尺寸：模型期望的输入图像大小\n"
        "• 缩放因子：通常为1/255.0用于归一化像素值\n"
        "• 均值减法：用于图像预处理的均值值\n"
        "• 通道交换：OpenCV默认BGR，多数模型期望RGB", 
        this
    );
    hintLabel->setStyleSheet("QLabel { color: gray; font-size: 11px; }");
    hintLabel->setWordWrap(true);
    gridLayout->addWidget(hintLabel, 4, 0, 1, 4);
    
    m_mainLayout->addWidget(blobGroup);
}

void AlgorithmImportDialog::setupButtons()
{
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("取消", this);
    m_okButton = new QPushButton("确定", this);
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);
    
    connect(m_cancelButton, &QPushButton::clicked, this, &AlgorithmImportDialog::onReject);
    connect(m_okButton, &QPushButton::clicked, this, &AlgorithmImportDialog::onAccept);
    
    m_mainLayout->addLayout(buttonLayout);
}

void AlgorithmImportDialog::loadPresetParameters()
{
    // 根据模型类型设置预设参数
    onModelTypeChanged(m_modelTypeCombo->currentText());
}

void AlgorithmImportDialog::onBrowseModel()
{
    QString currentType = m_modelTypeCombo->currentText();
    QString filter;
    
    if (currentType.contains("ONNX")) {
        filter = "ONNX模型 (*.onnx)";
    } else if (currentType.contains("TensorFlow")) {
        filter = "TensorFlow模型 (*.pb)";
    } else if (currentType.contains("Caffe")) {
        filter = "Caffe模型 (*.caffemodel)";
    } else if (currentType.contains("PyTorch")) {
        filter = "PyTorch模型 (*.pt *.pth)";
    } else if (currentType.contains("Darknet")) {
        filter = "Darknet模型 (*.weights)";
    } else {
        filter = "所有文件 (*.*)";
    }
    
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择AI模型文件",
        QString(),
        filter + ";;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        m_modelPathEdit->setText(fileName);
    }
}

void AlgorithmImportDialog::onModelTypeChanged(const QString &type)
{
    // 根据模型类型设置常用的预设参数
    if (type.contains("YOLO") || type.contains("Darknet")) {
        // YOLO系列常用参数
        m_widthSpinBox->setValue(416);
        m_heightSpinBox->setValue(416);
        m_scaleFactorSpinBox->setValue(1.0/255.0);
        m_meanRSpinBox->setValue(0.0);
        m_meanGSpinBox->setValue(0.0);
        m_meanBSpinBox->setValue(0.0);
        m_swapRBCheckBox->setChecked(true);
    } else if (type.contains("SSD")) {
        // SSD系列常用参数
        m_widthSpinBox->setValue(300);
        m_heightSpinBox->setValue(300);
        m_scaleFactorSpinBox->setValue(0.017);
        m_meanRSpinBox->setValue(103.94);
        m_meanGSpinBox->setValue(116.78);
        m_meanBSpinBox->setValue(123.68);
        m_swapRBCheckBox->setChecked(false);
    } else if (type.contains("MobileNet")) {
        // MobileNet常用参数
        m_widthSpinBox->setValue(224);
        m_heightSpinBox->setValue(224);
        m_scaleFactorSpinBox->setValue(0.017);
        m_meanRSpinBox->setValue(103.94);
        m_meanGSpinBox->setValue(116.78);
        m_meanBSpinBox->setValue(123.68);
        m_swapRBCheckBox->setChecked(false);
    } else {
        // 默认参数
        m_widthSpinBox->setValue(416);
        m_heightSpinBox->setValue(416);
        m_scaleFactorSpinBox->setValue(1.0/255.0);
        m_meanRSpinBox->setValue(0.0);
        m_meanGSpinBox->setValue(0.0);
        m_meanBSpinBox->setValue(0.0);
        m_swapRBCheckBox->setChecked(true);
    }
}

void AlgorithmImportDialog::onAccept()
{
    // 验证输入
    if (m_modelPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择模型文件路径！");
        return;
    }
    
    // 检查文件是否存在
    if (!QFile::exists(m_modelPathEdit->text())) {
        QMessageBox::warning(this, "警告", "选择的模型文件不存在！");
        return;
    }
    
    accept();
}

void AlgorithmImportDialog::onReject()
{
    reject();
}

AlgorithmImportDialog::ModelConfig AlgorithmImportDialog::getModelConfig() const
{
    ModelConfig config;
    config.modelPath = m_modelPathEdit->text();
    config.modelType = m_modelTypeCombo->currentText();
    
    config.blobParams.width = m_widthSpinBox->value();
    config.blobParams.height = m_heightSpinBox->value();
    config.blobParams.scaleFactor = m_scaleFactorSpinBox->value();
    config.blobParams.meanR = m_meanRSpinBox->value();
    config.blobParams.meanG = m_meanGSpinBox->value();
    config.blobParams.meanB = m_meanBSpinBox->value();
    config.blobParams.swapRB = m_swapRBCheckBox->isChecked();
    
    return config;
}