#include "algorithmdialog.h"
#include <QQmlContext>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QtQuick/QQuickItem>
#include <QUrl>
#include "Algorithms/algorithmfactory.h" // 添加算法工厂头文件
#include <QMessageBox> // 用于显示错误消息
#include <QMap> // 用于名称到ID映射
#include "CommonUtils.h"

AlgorithmDialog::AlgorithmDialog(FrameProcessor* processor, QWidget* parent)
    : QDialog(parent), m_processor(processor)
{
    setMinimumSize(550, 400);
    setupUI();
}

void AlgorithmDialog::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // QML视图
    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    
    // 设置QML上下文属性
        QQmlContext* context = m_quickWidget->rootContext();
    context->setContextProperty("algorithmModel", m_processor->algorithmModel());
    
    // 加载QML文件
    //qDebug() << "尝试加载:" << "qrc:/ListView/Qml/AlgorithmModelListView.qml";
    QFileInfo info(QDir::current(), "Qml/AlgorithmModelListView.qml");
    //qDebug() << "文件是否存在:" << info.exists();
    //qDebug() << "完整路径:" << info.absoluteFilePath();
    m_quickWidget->setSource(QUrl(QStringLiteral("qrc:/Qml/AlgorithmModelListView.qml")));
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_addButton = new QPushButton("添加算法", this);
    m_closeButton = new QPushButton("关闭", this);
    
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    
    // 添加到主布局
    mainLayout->addWidget(m_quickWidget);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号和槽
    connect(m_addButton, &QPushButton::clicked, this, &AlgorithmDialog::onAddAlgorithm);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    // 连接QML信号
    QQuickItem* rootObject = m_quickWidget->rootObject(); // 使用正确的类型
    if (rootObject) {
        connect(rootObject, SIGNAL(algorithmDeleted(int)), 
                this, SLOT(onAlgorithmDeleted(int)));
        connect(rootObject, SIGNAL(algorithmParamsChanged(int,QVariant)), 
                this, SLOT(onAlgorithmParamsChanged(int,QVariant)));
    }
}

void AlgorithmDialog::onAddAlgorithm()
{

    //开始添加算法
    qDebug()<<"[AlgorithMidialog] :"<<"now , Add Algorithm";
    // 从工厂获取所有已注册算法信息
    QList<QPair<int, QString>> algorithmInfoList = AlgorithmFactory::instance().getAlgorithmInfoList();
    
    if (algorithmInfoList.isEmpty()) {
        QMessageBox::warning(this, "错误", "没有可用的算法");
        return;
    }
    
    // 构建算法名称列表
    QStringList algorithmNames;
    QMap<QString, int> nameToIdMap; // 用于从名称查找ID
    
    for (const auto& info : algorithmInfoList) {
        algorithmNames << info.second; // 算法名称
        nameToIdMap[info.second] = info.first; // 名称到ID的映射
    }
    
    // 显示算法选择对话框
    bool ok;
    QString selected = QInputDialog::getItem(
        this, "选择算法", "请选择要添加的算法:", 
        algorithmNames, 0, false, &ok);
    
    if (!ok || selected.isEmpty())
        return;
    
    // 查找选择的算法ID
    int algorithmId = nameToIdMap.value(selected, -1);
    if (algorithmId == -1) {
        QMessageBox::warning(this, "错误", "无效的算法选择");
        return;
    }
    
    // 添加算法到处理器
    m_processor->addAlgorithm(algorithmId);
    
    // 通知QML刷新
    QQuickItem* rootObject = m_quickWidget->rootObject();
    if (rootObject) {
        QMetaObject::invokeMethod(rootObject, "refreshModel");
    }
}

void AlgorithmDialog::onAlgorithmDeleted(int index)
{
    if (m_processor) {
        m_processor->removeAlgorithm(index);
        qDebug()<<"[Algorithmidalog]: onAlgorithDeted success";
    }
}

void AlgorithmDialog::onAlgorithmParamsChanged(int index, const QVariant& params)
{
    if (m_processor) {
        // 将QVariant转换为QVariantMap
        QVariantMap paramMap = params.toMap();
        m_processor->updateAlgorithmParams(index, paramMap);
    }
}
