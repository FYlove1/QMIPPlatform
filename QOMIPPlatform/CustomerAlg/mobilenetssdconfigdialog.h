#ifndef MOBILENETSSDCONFIGDIALOG_H
#define MOBILENETSSDCONFIGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QLabel>
#include <QSlider>
#include <QTabWidget>
#include <QTextEdit>
#include "opencv2/dnn.hpp"

class MobileNetSSDConfigDialog : public QDialog
{
    Q_OBJECT

public:
    // MobileNet SSD配置结构
    struct MobileNetSSDConfig {
        // 模型文件路径
        QString modelPath;          // .pb文件路径
        QString configPath;         // .pbtxt文件路径
        bool useBuiltinModel = true; // 使用内置模型
        
        // 预处理参数（性能优化）
        int inputWidth = 224;            // 适中分辨率，平衡性能和效果
        int inputHeight = 224;           // 比300小，比160大
        bool swapRB = true;              // BGR->RGB转换
        double meanR = 127.5;            // 均值减法
        double meanG = 127.5;
        double meanB = 127.5;
        double scaleFactor = 1.0/127.5;  // 归一化因子
        
        // 结果过滤参数（适配模型特性）
        float confidenceThreshold = 0.03f;   // 降低到适合模型的阈值
        float nmsThreshold = 0.4f;           // 标准NMS阈值
        bool useNMS = true;                  // 启用NMS去重
        
        // 性能优化参数（流畅度优先）
        cv::dnn::Backend backend = cv::dnn::DNN_BACKEND_OPENCV;
        cv::dnn::Target target = cv::dnn::DNN_TARGET_CPU;
        bool enableOptimization = true;     // 启用网络优化
        
        // 场景适配参数（人体检测优化）
        QStringList enabledClasses;         // 启用的类别列表
        bool personOnly = true;              // 仅检测人，提升性能
        int maxDetections = 20;              // 限制检测数量
        
        // 显示参数
        bool showLabels = true;              // 显示标签
        bool showConfidence = true;          // 显示置信度
        int fontSize = 12;                   // 字体大小
        
        MobileNetSSDConfig() {
            // 初始化默认启用的类别（COCO数据集）
            enabledClasses << "person" << "car" << "motorbike" << "bus" << "truck" 
                          << "bicycle" << "dog" << "cat" << "horse" << "sheep" << "cow";
        }
    };

public:
    explicit MobileNetSSDConfigDialog(QWidget *parent = nullptr);
    ~MobileNetSSDConfigDialog();

    // 获取和设置配置
    MobileNetSSDConfig getConfiguration() const;
    void setConfiguration(const MobileNetSSDConfig& config);

    // 验证配置
    bool validateConfiguration();

private slots:
    void onBrowseModelFile();
    void onBrowseConfigFile();
    void onUseBuiltinModelToggled(bool enabled);
    void onBackendChanged();
    void onPersonOnlyToggled(bool enabled);
    void onClassSelectionChanged();
    void onConfidenceChanged(double value);
    void onNMSChanged(double value);
    void onResetToDefaults();
    void onTestModel();
    void onAccept();
    void onReject();

private:
    void setupUI();
    void setupFileSelection();
    void setupPreprocessingParams();
    void setupPostprocessingParams();
    void setupPerformanceParams();
    void setupSceneParams();
    void setupDisplayParams();
    void setupButtons();
    
    void updateClassList();
    void updatePerformanceInfo();
    QStringList getCOCOClassNames();

private:
    // UI组件
    QTabWidget* m_tabWidget;
    QVBoxLayout* m_mainLayout;
    
    // 文件选择标签页
    QWidget* m_fileTab;
    QLineEdit* m_modelPathEdit;
    QLineEdit* m_configPathEdit;
    QPushButton* m_browseModelButton;
    QPushButton* m_browseConfigButton;
    QLabel* m_modelStatusLabel;
    QCheckBox* m_useBuiltinModelCheckBox;
    
    // 预处理参数标签页（固定值显示）
    QWidget* m_preprocessTab;
    QLabel* m_inputSizeLabel;
    QLabel* m_meanLabel;
    QLabel* m_scaleLabel;
    QCheckBox* m_swapRBCheckBox;
    
    // 后处理参数标签页
    QWidget* m_postprocessTab;
    QDoubleSpinBox* m_confidenceSpinBox;
    QSlider* m_confidenceSlider;
    QDoubleSpinBox* m_nmsSpinBox;
    QSlider* m_nmsSlider;
    QCheckBox* m_useNMSCheckBox;
    QSpinBox* m_maxDetectionsSpinBox;
    
    // 性能优化标签页
    QWidget* m_performanceTab;
    QComboBox* m_backendCombo;
    QComboBox* m_targetCombo;
    QCheckBox* m_enableOptimizationCheckBox;
    QLabel* m_performanceInfoLabel;
    
    // 场景适配标签页
    QWidget* m_sceneTab;
    QCheckBox* m_personOnlyCheckBox;
    QListWidget* m_classListWidget;
    QPushButton* m_selectAllButton;
    QPushButton* m_selectNoneButton;
    QPushButton* m_selectCommonButton;
    
    // 显示参数标签页
    QWidget* m_displayTab;
    QCheckBox* m_showLabelsCheckBox;
    QCheckBox* m_showConfidenceCheckBox;
    QSpinBox* m_fontSizeSpinBox;
    
    // 按钮
    QPushButton* m_testButton;
    QPushButton* m_resetButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // 数据
    MobileNetSSDConfig m_config;
    QStringList m_cocoClasses;
};

#endif // MOBILENETSSDCONFIGDIALOG_H