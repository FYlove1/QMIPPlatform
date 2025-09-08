#ifndef ALGORITHMIMPORTDIALOG_H
#define ALGORITHMIMPORTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>

class AlgorithmImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AlgorithmImportDialog(QWidget *parent = nullptr);

    // 获取用户配置的参数
    struct BlobParameters {
        int width;
        int height;
        double scaleFactor;
        double meanR;
        double meanG;
        double meanB;
        bool swapRB;
    };

    struct ModelConfig {
        QString modelPath;
        QString modelType;
        BlobParameters blobParams;
    };

    ModelConfig getModelConfig() const;

private slots:
    void onBrowseModel();
    void onAccept();
    void onReject();
    void onModelTypeChanged(const QString &type);

private:
    void setupUI();
    void setupModelPathSection();
    void setupBlobParametersSection();
    void setupButtons();
    void loadPresetParameters();

private:
    // 模型路径相关
    QLineEdit *m_modelPathEdit;
    QPushButton *m_browseButton;
    QComboBox *m_modelTypeCombo;
    
    // blobFromImage参数相关
    QSpinBox *m_widthSpinBox;
    QSpinBox *m_heightSpinBox;
    QDoubleSpinBox *m_scaleFactorSpinBox;
    QDoubleSpinBox *m_meanRSpinBox;
    QDoubleSpinBox *m_meanGSpinBox;
    QDoubleSpinBox *m_meanBSpinBox;
    QCheckBox *m_swapRBCheckBox;
    
    // 按钮
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    
    // 布局
    QVBoxLayout *m_mainLayout;
};

#endif // ALGORITHMIMPORTDIALOG_H