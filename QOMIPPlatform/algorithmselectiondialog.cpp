#include "algorithmselectiondialog.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QHeaderView>
#include <QFormLayout>

AlgorithmSelectionDialog::AlgorithmSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , m_selectedModelIndex(-1)
{
    setWindowTitle("算法选择与管理");
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(800, 600);
    
    // 初始化设置
    m_settings = new QSettings("QOMIPPlatform", "AlgorithmConfig", this);
    
    setupUI();
    loadSettings();
    updateModelList();
    updateUIState();
}

AlgorithmSelectionDialog::~AlgorithmSelectionDialog()
{
    saveSettings();
}

void AlgorithmSelectionDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    setupModeSelection();
    setupModelManagement();
    setupModelDetails();
    setupButtons();
    
    m_mainLayout->addWidget(m_splitter);
    
    setLayout(m_mainLayout);
}

void AlgorithmSelectionDialog::setupModeSelection()
{
    QWidget* leftWidget = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    
    // 模式选择组
    m_modeGroup = new QGroupBox("算法模式选择", leftWidget);
    QVBoxLayout* modeLayout = new QVBoxLayout(m_modeGroup);
    
    m_normalModeRadio = new QRadioButton("普通算法模式", m_modeGroup);
    m_normalModeRadio->setToolTip("使用传统的图像处理算法");
    m_normalModeRadio->setChecked(true);
    
    m_aiModeRadio = new QRadioButton("AI大模型模式", m_modeGroup);
    m_aiModeRadio->setToolTip("使用导入的AI深度学习模型");
    
    modeLayout->addWidget(m_normalModeRadio);
    modeLayout->addWidget(m_aiModeRadio);
    
    // 连接信号
    connect(m_normalModeRadio, &QRadioButton::toggled, this, &AlgorithmSelectionDialog::onModeChanged);
    connect(m_aiModeRadio, &QRadioButton::toggled, this, &AlgorithmSelectionDialog::onModeChanged);
    
    leftLayout->addWidget(m_modeGroup);
    
    setupModelManagement();
    leftLayout->addWidget(m_modelGroup);
    
    m_splitter->addWidget(leftWidget);
}

void AlgorithmSelectionDialog::setupModelManagement()
{
    m_modelGroup = new QGroupBox("AI模型管理", this);
    QVBoxLayout* modelLayout = new QVBoxLayout(m_modelGroup);
    
    // 模型列表
    m_modelList = new QListWidget(m_modelGroup);
    m_modelList->setAlternatingRowColors(true);
    m_modelList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addModelButton = new QPushButton("添加模型", m_modelGroup);
    m_removeModelButton = new QPushButton("删除模型", m_modelGroup);
    m_editModelButton = new QPushButton("编辑模型", m_modelGroup);
    
    m_addModelButton->setIcon(QIcon(":/icons/add.png"));
    m_removeModelButton->setIcon(QIcon(":/icons/remove.png"));
    m_editModelButton->setIcon(QIcon(":/icons/edit.png"));
    
    buttonLayout->addWidget(m_addModelButton);
    buttonLayout->addWidget(m_removeModelButton);
    buttonLayout->addWidget(m_editModelButton);
    buttonLayout->addStretch();
    
    modelLayout->addWidget(m_modelList);
    modelLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(m_modelList, &QListWidget::itemSelectionChanged, 
            this, &AlgorithmSelectionDialog::onModelSelectionChanged);
    connect(m_modelList, &QListWidget::itemDoubleClicked, 
            this, &AlgorithmSelectionDialog::onModelDoubleClicked);
    connect(m_addModelButton, &QPushButton::clicked, 
            this, &AlgorithmSelectionDialog::onAddModelClicked);
    connect(m_removeModelButton, &QPushButton::clicked, 
            this, &AlgorithmSelectionDialog::onRemoveModelClicked);
    connect(m_editModelButton, &QPushButton::clicked, 
            this, &AlgorithmSelectionDialog::onEditModelClicked);
}

void AlgorithmSelectionDialog::setupModelDetails()
{
    QWidget* rightWidget = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    
    // 模型详情组
    m_detailsGroup = new QGroupBox("模型详情", rightWidget);
    QFormLayout* detailsLayout = new QFormLayout(m_detailsGroup);
    
    m_modelNameLabel = new QLabel("未选择", m_detailsGroup);
    m_modelPathLabel = new QLabel("无", m_detailsGroup);
    m_modelPathLabel->setWordWrap(true);
    m_modelTypeLabel = new QLabel("无", m_detailsGroup);
    m_importTimeLabel = new QLabel("无", m_detailsGroup);
    
    detailsLayout->addRow("模型名称:", m_modelNameLabel);
    detailsLayout->addRow("模型路径:", m_modelPathLabel);
    detailsLayout->addRow("模型类型:", m_modelTypeLabel);
    detailsLayout->addRow("导入时间:", m_importTimeLabel);
    
    // 描述编辑
    m_descriptionEdit = new QTextEdit(m_detailsGroup);
    m_descriptionEdit->setMaximumHeight(80);
    m_descriptionEdit->setPlaceholderText("模型描述信息...");
    detailsLayout->addRow("描述:", m_descriptionEdit);
    
    rightLayout->addWidget(m_detailsGroup);
    
    // 参数调整组
    m_parametersGroup = new QGroupBox("推理参数", rightWidget);
    QFormLayout* paramLayout = new QFormLayout(m_parametersGroup);
    
    m_confidenceSpinBox = new QDoubleSpinBox(m_parametersGroup);
    m_confidenceSpinBox->setRange(0.0, 1.0);
    m_confidenceSpinBox->setSingleStep(0.05);
    m_confidenceSpinBox->setValue(0.5);
    m_confidenceSpinBox->setDecimals(2);
    
    m_nmsSpinBox = new QDoubleSpinBox(m_parametersGroup);
    m_nmsSpinBox->setRange(0.0, 1.0);
    m_nmsSpinBox->setSingleStep(0.05);
    m_nmsSpinBox->setValue(0.4);
    m_nmsSpinBox->setDecimals(2);
    
    m_useNMSCheckBox = new QCheckBox("启用非极大值抑制", m_parametersGroup);
    m_useNMSCheckBox->setChecked(true);
    
    paramLayout->addRow("置信度阈值:", m_confidenceSpinBox);
    paramLayout->addRow("NMS阈值:", m_nmsSpinBox);
    paramLayout->addRow("", m_useNMSCheckBox);
    
    // 连接参数变化信号
    connect(m_confidenceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AlgorithmSelectionDialog::onParameterChanged);
    connect(m_nmsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AlgorithmSelectionDialog::onParameterChanged);
    connect(m_useNMSCheckBox, &QCheckBox::toggled,
            this, &AlgorithmSelectionDialog::onParameterChanged);
    connect(m_descriptionEdit, &QTextEdit::textChanged,
            this, &AlgorithmSelectionDialog::onParameterChanged);
    
    rightLayout->addWidget(m_parametersGroup);
    rightLayout->addStretch();
    
    m_splitter->addWidget(rightWidget);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
}

void AlgorithmSelectionDialog::setupButtons()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("取消", this);
    m_okButton = new QPushButton("确定", this);
    m_okButton->setDefault(true);
    
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);
    
    connect(m_cancelButton, &QPushButton::clicked, this, &AlgorithmSelectionDialog::onReject);
    connect(m_okButton, &QPushButton::clicked, this, &AlgorithmSelectionDialog::onAccept);
    
    m_mainLayout->addLayout(buttonLayout);
}

void AlgorithmSelectionDialog::loadSettings()
{
    // 加载算法模式
    int mode = m_settings->value("algorithm_mode", static_cast<int>(MODE_NORMAL)).toInt();
    m_currentConfig.mode = static_cast<AlgorithmMode>(mode);
    
    if (m_currentConfig.mode == MODE_NORMAL) {
        m_normalModeRadio->setChecked(true);
    } else {
        m_aiModeRadio->setChecked(true);
    }
    
    // 加载选中的模型
    m_currentConfig.selectedModelName = m_settings->value("selected_model", "").toString();
    
    // 加载保存的模型列表
    int modelCount = m_settings->beginReadArray("saved_models");
    m_savedModels.clear();
    
    for (int i = 0; i < modelCount; ++i) {
        m_settings->setArrayIndex(i);
        QVariantMap modelData = m_settings->value("model_data").toMap();
        SavedAIModel model = modelFromVariant(modelData);
        m_savedModels.push_back(model);
    }
    m_settings->endArray();
}

void AlgorithmSelectionDialog::saveSettings()
{
    // 保存算法模式
    m_settings->setValue("algorithm_mode", static_cast<int>(m_currentConfig.mode));
    m_settings->setValue("selected_model", m_currentConfig.selectedModelName);
    
    // 保存模型列表
    m_settings->beginWriteArray("saved_models", m_savedModels.size());
    for (size_t i = 0; i < m_savedModels.size(); ++i) {
        m_settings->setArrayIndex(i);
        QVariantMap modelData = modelToVariant(m_savedModels[i]);
        m_settings->setValue("model_data", modelData);
    }
    m_settings->endArray();
    
    m_settings->sync();
}

void AlgorithmSelectionDialog::updateModelList()
{
    m_modelList->clear();
    
    for (const auto& model : m_savedModels) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(model.name);
        item->setToolTip(QString("路径: %1\n类型: %2\n导入时间: %3")
                        .arg(model.modelPath)
                        .arg(model.modelType)
                        .arg(model.importTime.toString()));
        
        // 设置图标（根据模型类型）
        if (model.modelType.contains("YOLO", Qt::CaseInsensitive)) {
            item->setIcon(QIcon(":/icons/yolo.png"));
        } else if (model.modelType.contains("SSD", Qt::CaseInsensitive)) {
            item->setIcon(QIcon(":/icons/ssd.png"));
        } else {
            item->setIcon(QIcon(":/icons/ai_model.png"));
        }
        
        m_modelList->addItem(item);
        
        // 如果是当前选中的模型，设置为选中状态
        if (model.name == m_currentConfig.selectedModelName) {
            item->setSelected(true);
            m_selectedModelIndex = m_modelList->row(item);
        }
    }
}

void AlgorithmSelectionDialog::updateModelDetails()
{
    if (m_selectedModelIndex >= 0 && m_selectedModelIndex < m_savedModels.size()) {
        const SavedAIModel& model = m_savedModels[m_selectedModelIndex];
        
        m_modelNameLabel->setText(model.name);
        m_modelPathLabel->setText(model.modelPath);
        m_modelTypeLabel->setText(model.modelType);
        m_importTimeLabel->setText(model.importTime.toString("yyyy-MM-dd hh:mm:ss"));
        m_descriptionEdit->setText(model.description);
        
        // 更新参数
        m_confidenceSpinBox->setValue(model.postParams.confidenceThreshold);
        m_nmsSpinBox->setValue(model.postParams.nmsThreshold);
        m_useNMSCheckBox->setChecked(model.postParams.useNMS);
        
        // 启用编辑和删除按钮
        m_removeModelButton->setEnabled(true);
        m_editModelButton->setEnabled(true);
        m_parametersGroup->setEnabled(true);
        m_descriptionEdit->setEnabled(true);
    } else {
        m_modelNameLabel->setText("未选择");
        m_modelPathLabel->setText("无");
        m_modelTypeLabel->setText("无");
        m_importTimeLabel->setText("无");
        m_descriptionEdit->clear();
        
        // 禁用相关按钮
        m_removeModelButton->setEnabled(false);
        m_editModelButton->setEnabled(false);
        m_parametersGroup->setEnabled(false);
        m_descriptionEdit->setEnabled(false);
    }
}

void AlgorithmSelectionDialog::updateUIState()
{
    bool aiModeEnabled = m_aiModeRadio->isChecked();
    
    // 根据模式启用/禁用AI相关组件
    m_modelGroup->setEnabled(aiModeEnabled);
    m_detailsGroup->setEnabled(aiModeEnabled);
    
    if (aiModeEnabled && m_savedModels.empty()) {
        // 如果选择AI模式但没有模型，显示提示
        QLabel* hintLabel = new QLabel("请先添加AI模型", m_modelGroup);
        hintLabel->setAlignment(Qt::AlignCenter);
        hintLabel->setStyleSheet("color: gray; font-style: italic;");
    }
}

// 槽函数实现
void AlgorithmSelectionDialog::onModeChanged()
{
    if (m_normalModeRadio->isChecked()) {
        m_currentConfig.mode = MODE_NORMAL;
    } else {
        m_currentConfig.mode = MODE_AI_MODEL;
    }
    updateUIState();
}

void AlgorithmSelectionDialog::onModelSelectionChanged()
{
    QListWidgetItem* currentItem = m_modelList->currentItem();
    if (currentItem) {
        m_selectedModelIndex = m_modelList->row(currentItem);
        m_currentConfig.selectedModelName = currentItem->text();
        if (m_selectedModelIndex < m_savedModels.size()) {
            m_currentConfig.selectedModel = m_savedModels[m_selectedModelIndex];
        }
    } else {
        m_selectedModelIndex = -1;
        m_currentConfig.selectedModelName.clear();
    }
    updateModelDetails();
}

void AlgorithmSelectionDialog::onAddModelClicked()
{
    AlgorithmImportDialog importDialog(this);
    if (importDialog.exec() == QDialog::Accepted) {
        AlgorithmImportDialog::ModelConfig config = importDialog.getModelConfig();
        addAIModel(config);
        updateModelList();
        
        // 选中新添加的模型
        if (!m_savedModels.empty()) {
            m_modelList->setCurrentRow(m_savedModels.size() - 1);
        }
    }
}

void AlgorithmSelectionDialog::onRemoveModelClicked()
{
    if (m_selectedModelIndex >= 0 && m_selectedModelIndex < m_savedModels.size()) {
        QString modelName = m_savedModels[m_selectedModelIndex].name;
        
        int ret = QMessageBox::question(this, "确认删除", 
                                       QString("确定要删除模型 \"%1\" 吗？").arg(modelName),
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            m_savedModels.erase(m_savedModels.begin() + m_selectedModelIndex);
            updateModelList();
            
            // 清空选择
            if (m_currentConfig.selectedModelName == modelName) {
                m_currentConfig.selectedModelName.clear();
            }
            m_selectedModelIndex = -1;
            updateModelDetails();
        }
    }
}

void AlgorithmSelectionDialog::onEditModelClicked()
{
    if (m_selectedModelIndex >= 0 && m_selectedModelIndex < m_savedModels.size()) {
        // TODO: 实现编辑功能，可以重新打开导入对话框并预填充数据
        QMessageBox::information(this, "提示", "编辑功能正在开发中...");
    }
}

void AlgorithmSelectionDialog::onModelDoubleClicked(QListWidgetItem* item)
{
    // 双击直接选择该模型并关闭对话框
    if (item && m_aiModeRadio->isChecked()) {
        onAccept();
    }
}

void AlgorithmSelectionDialog::onParameterChanged()
{
    // 更新当前选中模型的参数
    if (m_selectedModelIndex >= 0 && m_selectedModelIndex < m_savedModels.size()) {
        SavedAIModel& model = m_savedModels[m_selectedModelIndex];
        model.postParams.confidenceThreshold = m_confidenceSpinBox->value();
        model.postParams.nmsThreshold = m_nmsSpinBox->value();
        model.postParams.useNMS = m_useNMSCheckBox->isChecked();
        model.description = m_descriptionEdit->toPlainText();
        
        m_currentConfig.selectedModel = model;
    }
}

void AlgorithmSelectionDialog::onAccept()
{
    // 验证配置
    if (m_currentConfig.mode == MODE_AI_MODEL) {
        if (m_savedModels.empty()) {
            QMessageBox::warning(this, "警告", "AI模式下必须至少有一个模型！");
            return;
        }
        if (m_currentConfig.selectedModelName.isEmpty()) {
            QMessageBox::warning(this, "警告", "请选择一个AI模型！");
            return;
        }
    }
    
    saveSettings();
    accept();
}

void AlgorithmSelectionDialog::onReject()
{
    reject();
}

// 公共接口实现
AlgorithmSelectionDialog::AlgorithmConfig AlgorithmSelectionDialog::getCurrentConfig() const
{
    return m_currentConfig;
}

void AlgorithmSelectionDialog::setCurrentConfig(const AlgorithmConfig& config)
{
    m_currentConfig = config;
    
    // 更新UI
    if (config.mode == MODE_NORMAL) {
        m_normalModeRadio->setChecked(true);
    } else {
        m_aiModeRadio->setChecked(true);
    }
    
    updateUIState();
    updateModelList();
}

void AlgorithmSelectionDialog::addAIModel(const AlgorithmImportDialog::ModelConfig& modelConfig)
{
    SavedAIModel newModel;
    newModel.name = generateModelName(modelConfig.modelPath, modelConfig.modelType);
    newModel.modelPath = modelConfig.modelPath;
    newModel.modelType = modelConfig.modelType;
    newModel.blobParams = modelConfig.blobParams;
    newModel.importTime = QDateTime::currentDateTime();
    newModel.description = QString("从 %1 导入的 %2 模型").arg(QFileInfo(modelConfig.modelPath).fileName()).arg(modelConfig.modelType);
    
    // 设置默认后处理参数
    newModel.postParams.confidenceThreshold = 0.5f;
    newModel.postParams.nmsThreshold = 0.4f;
    newModel.postParams.useNMS = true;
    
    m_savedModels.push_back(newModel);
}

std::vector<AlgorithmSelectionDialog::SavedAIModel> AlgorithmSelectionDialog::getSavedModels() const
{
    return m_savedModels;
}

// 辅助函数
QString AlgorithmSelectionDialog::generateModelName(const QString& modelPath, const QString& modelType)
{
    QFileInfo fileInfo(modelPath);
    QString baseName = fileInfo.baseName();
    
    // 检查是否已存在相同名称
    int counter = 1;
    QString candidateName = QString("%1_%2").arg(modelType.split(" ").first()).arg(baseName);
    
    while (true) {
        bool nameExists = false;
        for (const auto& model : m_savedModels) {
            if (model.name == candidateName) {
                nameExists = true;
                break;
            }
        }
        
        if (!nameExists) {
            break;
        }
        
        candidateName = QString("%1_%2_%3").arg(modelType.split(" ").first()).arg(baseName).arg(counter);
        counter++;
    }
    
    return candidateName;
}

QVariantMap AlgorithmSelectionDialog::modelToVariant(const SavedAIModel& model)
{
    QVariantMap map;
    map["name"] = model.name;
    map["modelPath"] = model.modelPath;
    map["modelType"] = model.modelType;
    map["description"] = model.description;
    map["importTime"] = model.importTime;
    
    // Blob参数
    QVariantMap blobMap;
    blobMap["width"] = model.blobParams.width;
    blobMap["height"] = model.blobParams.height;
    blobMap["scaleFactor"] = model.blobParams.scaleFactor;
    blobMap["meanR"] = model.blobParams.meanR;
    blobMap["meanG"] = model.blobParams.meanG;
    blobMap["meanB"] = model.blobParams.meanB;
    blobMap["swapRB"] = model.blobParams.swapRB;
    map["blobParams"] = blobMap;
    
    // 后处理参数
    QVariantMap postMap;
    postMap["confidenceThreshold"] = model.postParams.confidenceThreshold;
    postMap["nmsThreshold"] = model.postParams.nmsThreshold;
    postMap["useNMS"] = model.postParams.useNMS;
    map["postParams"] = postMap;
    
    return map;
}

AlgorithmSelectionDialog::SavedAIModel AlgorithmSelectionDialog::modelFromVariant(const QVariantMap& variant)
{
    SavedAIModel model;
    model.name = variant["name"].toString();
    model.modelPath = variant["modelPath"].toString();
    model.modelType = variant["modelType"].toString();
    model.description = variant["description"].toString();
    model.importTime = variant["importTime"].toDateTime();
    
    // Blob参数
    QVariantMap blobMap = variant["blobParams"].toMap();
    model.blobParams.width = blobMap["width"].toInt();
    model.blobParams.height = blobMap["height"].toInt();
    model.blobParams.scaleFactor = blobMap["scaleFactor"].toDouble();
    model.blobParams.meanR = blobMap["meanR"].toDouble();
    model.blobParams.meanG = blobMap["meanG"].toDouble();
    model.blobParams.meanB = blobMap["meanB"].toDouble();
    model.blobParams.swapRB = blobMap["swapRB"].toBool();
    
    // 后处理参数
    QVariantMap postMap = variant["postParams"].toMap();
    model.postParams.confidenceThreshold = postMap["confidenceThreshold"].toFloat();
    model.postParams.nmsThreshold = postMap["nmsThreshold"].toFloat();
    model.postParams.useNMS = postMap["useNMS"].toBool();
    
    return model;
}