#ifndef ALGORITHMSELECTIONDIALOG_H
#define ALGORITHMSELECTIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "CustomerAlg/algorithmimportdialog.h"
#include "CustomerAlg/aimodelprocessor.h"

class AlgorithmSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    // 算法模式枚举
    enum AlgorithmMode {
        MODE_NORMAL = 0,    // 普通算法
        MODE_AI_MODEL       // AI大模型
    };

    // 保存的AI模型配置
    struct SavedAIModel {
        QString name;                                    // 模型显示名称
        QString modelPath;                              // 模型文件路径
        QString modelType;                              // 模型类型
        AlgorithmImportDialog::BlobParameters blobParams; // blob参数
        AIModelProcessor::PostProcessParams postParams;   // 后处理参数
        QString description;                            // 描述信息
        QDateTime importTime;                          // 导入时间
        
        SavedAIModel() = default;
    };

    // 当前算法配置
    struct AlgorithmConfig {
        AlgorithmMode mode;
        QString selectedModelName;
        SavedAIModel selectedModel;
        
        AlgorithmConfig() : mode(MODE_NORMAL) {}
    };

public:
    explicit AlgorithmSelectionDialog(QWidget *parent = nullptr);
    ~AlgorithmSelectionDialog();

    // 获取当前配置
    AlgorithmConfig getCurrentConfig() const;
    
    // 设置配置
    void setCurrentConfig(const AlgorithmConfig& config);
    
    // 添加新模型到列表
    void addAIModel(const AlgorithmImportDialog::ModelConfig& modelConfig);
    
    // 获取所有已保存的模型
    std::vector<SavedAIModel> getSavedModels() const;

private slots:
    void onModeChanged();
    void onModelSelectionChanged();
    void onAddModelClicked();
    void onRemoveModelClicked();
    void onEditModelClicked();
    void onModelDoubleClicked(QListWidgetItem* item);
    void onAccept();
    void onReject();
    void onParameterChanged();

private:
    void setupUI();
    void setupModeSelection();
    void setupModelManagement();
    void setupModelDetails();
    void setupButtons();
    
    void loadSettings();
    void saveSettings();
    void updateModelList();
    void updateModelDetails();
    void updateUIState();
    
    // 模型序列化
    QVariantMap modelToVariant(const SavedAIModel& model);
    SavedAIModel modelFromVariant(const QVariantMap& variant);
    
    QString generateModelName(const QString& modelPath, const QString& modelType);

private:
    // UI组件
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 模式选择
    QGroupBox* m_modeGroup;
    QRadioButton* m_normalModeRadio;
    QRadioButton* m_aiModeRadio;
    
    // 模型管理
    QGroupBox* m_modelGroup;
    QListWidget* m_modelList;
    QPushButton* m_addModelButton;
    QPushButton* m_removeModelButton;
    QPushButton* m_editModelButton;
    
    // 模型详情和参数调整
    QGroupBox* m_detailsGroup;
    QLabel* m_modelNameLabel;
    QLabel* m_modelPathLabel;
    QLabel* m_modelTypeLabel;
    QLabel* m_importTimeLabel;
    QTextEdit* m_descriptionEdit;
    
    // 参数调整
    QGroupBox* m_parametersGroup;
    QDoubleSpinBox* m_confidenceSpinBox;
    QDoubleSpinBox* m_nmsSpinBox;
    QCheckBox* m_useNMSCheckBox;
    
    // 按钮
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // 数据
    std::vector<SavedAIModel> m_savedModels;
    AlgorithmConfig m_currentConfig;
    QSettings* m_settings;
    
    // 当前选中的模型索引
    int m_selectedModelIndex;
};

#endif // ALGORITHMSELECTIONDIALOG_H