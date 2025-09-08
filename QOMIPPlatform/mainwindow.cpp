#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "CommonUtils.h"
#include "detachedwindow.h"
#include "CustomerAlg/mobilenetssdprocessor.h"
#include "resourceextractor.h"
#include "exportprogressdialog.h"
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QMimeData>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <memory>
#include <QFileInfo>
#include <QSettings>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QThread>
#include <QApplication>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    currentWidetCount = 2;

    ui->playButton->setCheckable(true);
    
    // 启用拖放
    setAcceptDrops(true);

    // 初始化界面
    initAll();
    
    // 创建摄像头管理器
    m_cameraManager = new CameraManager(this);
    
    // 初始化摄像头UI
    initCameraUI();
    
    // 创建线程和Reader对象
    m_readerThread = new QThread(this);
    m_reader = new Reader();
    
    // 设置Reader需要处理的窗口数量
    m_reader->setViewCount(currentWidetCount);
    
    // 将Reader移动到线程中
    m_reader->moveToThread(m_readerThread);
    
    // 连接信号和槽
    connect(m_reader, &Reader::frameReady, 
            this, &MainWindow::on_Reader_FrameReady);
    connect(m_reader, &Reader::processingFinished,
            this, &MainWindow::onProcessingFinished);
    
    // 连接摄像头管理器信号
    connect(m_cameraManager, &CameraManager::camerasUpdated,
            this, &MainWindow::onCamerasUpdated);
    
    // 启动线程
    m_readerThread->start();

    // 连接UI动作
    connect(ui->actionOpen_Dir_for_Video, &QAction::triggered, this, [=](){
        ui->V_ListWidget->clear();
        for(const QString& file : CU::pickMediaFilesFromDir("选择媒体文件", this)){
            ui->V_ListWidget->addItem(file);
        }
    });
    
    // 连接视频导出菜单项
    connect(ui->actionExport_All_Playing_Video, &QAction::triggered, this, &MainWindow::exportAllVideos);
    connect(ui->actionExprot_Current_Video, &QAction::triggered, this, &MainWindow::exportCurrentVideo);

    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::on_playButton_clicked);
    
    // 应用启动时扫描摄像头
    QTimer::singleShot(500, this, [this]() {
        refreshCameras();
    });
    
    // 初始化MobileNet SSD处理器
    m_ssdProcessor = std::make_unique<MobileNetSSDProcessor>();
    m_useCustomAlgorithm = false;
    
    // 加载MobileNet SSD配置
    loadMobileNetSSDConfig();
}

MainWindow::~MainWindow()
{
    // 确保线程安全退出
    if (m_readerThread->isRunning()) {
        // 通过线程安全的方式停止处理
        QMetaObject::invokeMethod(m_reader, "stop", Qt::QueuedConnection);
        
        // 等待线程终止
        m_readerThread->quit();
        m_readerThread->wait();
        
        // 删除Reader对象
        m_reader->deleteLater();
    }
    
    // 清理MobileNet SSD处理器
    if (m_ssdProcessor) {
        m_ssdProcessor->unloadModel();
        m_ssdProcessor.reset();
    }
    
    // 保存MobileNet SSD配置
    saveMobileNetSSDConfig();
    
    // 清理资源提取的临时文件
    ResourceExtractor::cleanupTempFiles();
    
    delete ui;
}

void MainWindow::initAll()
{
    // 创建当前窗口个数的窗口，并添加到tab中
    for(int i = 0; i < currentWidetCount; i++){
        BasicViewWidget* widget = new BasicViewWidget(this);
        widget->setObjectName(QString("Video%1").arg(i+1));
        m_vectorWidget.push_back(widget);
        ui->videoWidget->addTab(widget, QString("Video%1").arg(i+1));
    }
    qDebug()<<"[MainWindow]: initAll Down. CurrentCount = " << currentWidetCount;
}

void MainWindow::initCameraUI()
{
    // 获取Camera标签页
    QWidget* cameraTab = ui->camera_tab;
    
    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout(cameraTab);
    
    // 添加标题标签
    QLabel* titleLabel = new QLabel("可用摄像头列表", cameraTab);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);
    
    // 创建摄像头列表控件
    m_cameraListWidget = new QListWidget(cameraTab);
    m_cameraListWidget->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #ccc;"
        "   border-radius: 4px;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 8px;"
        "   margin: 2px;"
        "   border-radius: 3px;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #e3f2fd;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #2196F3;"
        "   color: white;"
        "}"
    );
    
    // 连接列表项点击事件
    connect(m_cameraListWidget, &QListWidget::itemClicked,
            this, &MainWindow::onCameraListItemClicked);
    
    // 连接双击事件，双击直接开始播放
    connect(m_cameraListWidget, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
                onCameraListItemClicked(item);
                on_playButton_clicked();
            });
    
    layout->addWidget(m_cameraListWidget);
    
    // 添加刷新按钮
    QPushButton* refreshButton = new QPushButton("刷新摄像头列表", cameraTab);
    refreshButton->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 15px;"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"
        "}"
    );
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshCameras);
    
    layout->addWidget(refreshButton);
    
    // 添加提示标签
    QLabel* hintLabel = new QLabel("提示：选择摄像头后点击播放按钮开始预览", cameraTab);
    hintLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px;");
    layout->addWidget(hintLabel);
    
    // 添加弹性空间
    layout->addStretch();
}

void MainWindow::refreshCameras()
{
    // 清空列表
    m_cameraListWidget->clear();
    
    // 添加扫描中提示
    m_cameraListWidget->addItem("正在扫描摄像头...");
    
    // 异步扫描摄像头
    QTimer::singleShot(100, this, [this]() {
        m_cameraManager->scanCameras();
    });
}

void MainWindow::onCamerasUpdated(const QList<CameraManager::CameraInfo>& cameras)
{
    // 清空列表
    m_cameraListWidget->clear();
    
    if (cameras.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem("未检测到摄像头设备");
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        item->setTextAlignment(Qt::AlignCenter);
        m_cameraListWidget->addItem(item);
    } else {
        // 添加摄像头到列表
        for (const CameraManager::CameraInfo& camera : cameras) {
            QString displayText = QString("%1 - %2")
                .arg(camera.name)
                .arg(camera.description);
            
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, camera.index);  // 存储摄像头索引
            
            // 根据可用性设置图标或颜色
            if (camera.isAvailable) {
                item->setIcon(QIcon(":/icons/camera_on.png"));  // 如果有图标资源
                item->setToolTip("摄像头可用");
            } else {
                item->setForeground(Qt::gray);
                item->setToolTip("摄像头可能不可用");
            }
            
            m_cameraListWidget->addItem(item);
        }
        
        // 默认选中第一个
        if (m_cameraListWidget->count() > 0) {
            m_cameraListWidget->setCurrentRow(0);
        }
    }
}

void MainWindow::onCameraListItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    
    // 获取摄像头索引
    int cameraIndex = item->data(Qt::UserRole).toInt();
    
    // 设置Reader的摄像头源
    m_reader->setCameraSource(cameraIndex);
    
    // 更新状态栏或显示提示
    statusBar()->showMessage(QString("已选择摄像头: %1").arg(item->text()), 3000);
    
    qDebug() << "Selected camera index:" << cameraIndex;
}

void MainWindow::on_playButton_clicked()
{
    bool sourceChanged = false;
    
    // 检查是否有选中的视频文件
    QListWidgetItem* currentItem = ui->V_ListWidget->currentItem();
    if (currentItem) {
        QString filePath = currentItem->text();
        // 检查是否与当前源不同
        if (m_reader->getSourceType() != Reader::SOURCE_FILE || 
            m_reader->getCurrentSourcePath() != filePath) {
            m_reader->setSource(filePath);
            sourceChanged = true;
        }
    } 
    // 检查是否有选中的摄像头
    else {
        QListWidgetItem* cameraItem = m_cameraListWidget->currentItem();
        if (cameraItem) {
            int cameraIndex = cameraItem->data(Qt::UserRole).toInt();
            // 检查是否与当前源不同
            if (m_reader->getSourceType() != Reader::SOURCE_CAMERA || 
                m_reader->getCurrentCameraIndex() != cameraIndex) {
                m_reader->setCameraSource(cameraIndex);
                sourceChanged = true;
            }
        } else if (m_reader->getSourceType() == Reader::SOURCE_NONE) {
            // 没有任何源可用
            QMessageBox::warning(this, "警告", "请先选择视频文件或摄像头");
            return;
        }
    }

    // 如果源发生变化，重置播放状态
    if (sourceChanged) {
        ui->playButton->setChecked(true);
    }
    
    // 根据按钮状态执行播放或暂停
    if (ui->playButton->isChecked()) {
        // 播放
        ui->playButton->setText("暂停");
        ui->playButton->setStyleSheet("QPushButton { background-color: #f44336; }");
        
        // 通过线程安全的方式调用play方法
        QMetaObject::invokeMethod(m_reader, "play", Qt::QueuedConnection);
        
        qDebug() << "[MainWindow]: 开始播放";
    } else {
        // 暂停
        ui->playButton->setText("播放");
        ui->playButton->setStyleSheet("");
        
        // 通过线程安全的方式调用pause方法
        QMetaObject::invokeMethod(m_reader, "pause", Qt::QueuedConnection);
        
        qDebug() << "[MainWindow]: 暂停播放";
    }
}

void MainWindow::on_Reader_FrameReady(const cv::Mat &frame)
{
    // 将帧分发给所有视图窗口进行处理
    for(int i = 0; i < m_vectorWidget.size(); i++) {
        if(m_vectorWidget[i]) {
            // 获取当前选中的widget索引
            int currentTabIndex = ui->videoWidget->currentIndex();
            bool isCurrentWidget = (i == currentTabIndex);
            
            cv::Mat processedFrame = frame;
            
            // 如果是当前选中的widget且启用了MobileNet SSD模式，使用SSD处理
            if (isCurrentWidget && 
                m_useCustomAlgorithm && 
                m_ssdProcessor->isModelLoaded()) {
                
                try {
                    processedFrame = m_ssdProcessor->processFrame(frame);
                } catch (const std::exception& e) {
                    qDebug() << "[MainWindow] MobileNet SSD处理异常:" << e.what();
                    // 发生异常时使用原始帧
                    processedFrame = frame;
                }
            }
            
            // 将处理后的帧发送给widget
            m_vectorWidget[i]->processFrame(processedFrame);
        }
    }
}

void MainWindow::onProcessingFinished(const QString &message)
{
    // 恢复播放按钮状态
    ui->playButton->setChecked(false);
    ui->playButton->setText("播放");
    ui->playButton->setStyleSheet("");
    
    // 显示完成消息
    statusBar()->showMessage(message, 5000);
    qDebug() << "[MainWindow]:" << message;
}

void MainWindow::on_actionDelete_Current_Widget_triggered()
{
    // 获取当前选中的标签页索引
    int currentIndex = ui->videoWidget->currentIndex();
    
    // 检查是否有可删除的标签页
    if (currentIndex == -1 || ui->videoWidget->count() <= 1) {
        QMessageBox::information(this, tr("提示"), tr("至少需要保留一个视频窗口"));
        return;
    }
    
    // 获取要删除的widget
    BasicViewWidget* widgetToDelete = qobject_cast<BasicViewWidget*>(ui->videoWidget->widget(currentIndex));
    
    // 从标签页中移除
    ui->videoWidget->removeTab(currentIndex);
    
    // 从向量中移除
    int vectorIndex = m_vectorWidget.indexOf(widgetToDelete);
    if (vectorIndex != -1) {
        m_vectorWidget.remove(vectorIndex);
    }
    
    // 删除widget
    if (widgetToDelete) {
        widgetToDelete->deleteLater();
    }
    
    // 更新计数
    currentWidetCount--;
    
    // 更新Reader的视图数量
    m_reader->setViewCount(currentWidetCount);
    
    qDebug() << "[MainWindow]: 删除视频窗口，当前数量：" << currentWidetCount;
}

void MainWindow::on_actionAdd_triggered()
{
    // 增加新的视频窗口
    BasicViewWidget* newWidget = new BasicViewWidget(this);
    newWidget->setObjectName(QString("Video%1").arg(currentWidetCount + 1));
    
    // 添加到向量
    m_vectorWidget.push_back(newWidget);
    
    // 添加到标签页
    ui->videoWidget->addTab(newWidget, QString("Video%1").arg(currentWidetCount + 1));
    
    // 更新计数
    currentWidetCount++;
    
    // 切换到新添加的标签页
    ui->videoWidget->setCurrentIndex(ui->videoWidget->count() - 1);
    
    // 更新Reader的视图数量
    m_reader->setViewCount(currentWidetCount);
}

void MainWindow::on_actionSeparation_Current_Widget_triggered()
{
    // 获取当前选中的标签页索引
    int currentIndex = ui->videoWidget->currentIndex();
    
    // 检查是否有可分离的窗口
    if (currentIndex == -1 || ui->videoWidget->count() == 0) {
        QMessageBox::information(this, tr("提示"), tr("没有可以分离的窗口"));
        return;
    }
    
    // 获取要分离的widget
    BasicViewWidget* widgetToDetach = qobject_cast<BasicViewWidget*>(ui->videoWidget->widget(currentIndex));
    if (!widgetToDetach) {
        return;
    }
    
    // 检查是否已经分离
    if (m_detachedWindows.contains(widgetToDetach)) {
        QMessageBox::information(this, tr("提示"), tr("该窗口已经分离"));
        return;
    }
    
    // 获取标签页标题
    QString tabTitle = ui->videoWidget->tabText(currentIndex);
    
    // 从tab widget中移除widget（但不删除）
    ui->videoWidget->removeTab(currentIndex);
    
    // 创建分离窗口
    DetachedWindow* detachedWindow = new DetachedWindow(widgetToDetach, tabTitle, currentIndex, this);
    m_detachedWindows[widgetToDetach] = detachedWindow;
    
    // 连接信号
    connect(detachedWindow, &DetachedWindow::windowClosing, 
            this, &MainWindow::onDetachedWindowClosing);
    connect(detachedWindow, &DetachedWindow::returnToMainWindow,
            this, &MainWindow::onReturnToMainWindow);
    
    // 显示分离窗口
    detachedWindow->show();
    
    // 切换到下一个或上一个标签页
    int newIndex = -1;
    if (ui->videoWidget->count() > 0) {
        if (currentIndex < ui->videoWidget->count()) {
            newIndex = currentIndex;  // 选择原位置的下一个
        } else if (currentIndex > 0) {
            newIndex = currentIndex - 1;  // 选择上一个
        } else {
            newIndex = 0;  // 选择第一个
        }
        ui->videoWidget->setCurrentIndex(newIndex);
    }
}

void MainWindow::onDetachedWindowClosing(DetachedWindow *window)
{
    if (!window) return;
    
    BasicViewWidget* widget = window->getWidget();
    
    // 从映射中移除
    m_detachedWindows.remove(widget);
    
    // 注意：widget会随着DetachedWindow的销毁而销毁
    // 需要从m_vectorWidget中移除引用
    int index = m_vectorWidget.indexOf(widget);
    if (index != -1) {
        m_vectorWidget.remove(index);
        currentWidetCount--;
        m_reader->setViewCount(currentWidetCount);
    }
}

void MainWindow::onReturnToMainWindow(DetachedWindow *window)
{
    if (!window) return;
    
    BasicViewWidget* widget = window->getWidget();
    QString title = window->getTitle();
    int originalIndex = window->getOriginalIndex();
    
    // 从分离窗口中取出widget
    widget->setParent(nullptr);
    
    // 计算插入位置
    int insertIndex = qMin(originalIndex, ui->videoWidget->count());
    
    // 重新添加到tab widget
    ui->videoWidget->insertTab(insertIndex, widget, title);
    ui->videoWidget->setCurrentIndex(insertIndex);
    
    // 从映射中移除
    m_detachedWindows.remove(widget);
    
    // 关闭分离窗口
    window->close();
    window->deleteLater();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查是否是从分离窗口拖动过来的
    if (event->mimeData()->hasFormat("application/x-detachedwindow")) {
        // 显示视觉反馈
        ui->videoWidget->setStyleSheet("QTabWidget { border: 2px solid #3498db; background-color: #ecf0f1; }");
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-detachedwindow")) {
        // 清除视觉反馈
        ui->videoWidget->setStyleSheet("");
        
        // 从MIME数据中获取分离窗口的指针
        QByteArray data = event->mimeData()->data("application/x-detachedwindow");
        DetachedWindow* detachedWindow = (DetachedWindow*)(data.toLongLong());
        
        // 验证指针是否有效
        if (detachedWindow && m_detachedWindows.values().contains(detachedWindow)) {
            onReturnToMainWindow(detachedWindow);
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    // 清除视觉反馈
    ui->videoWidget->setStyleSheet("");
    QMainWindow::dragLeaveEvent(event);
}

void MainWindow::on_actionImport_Algorithm_Ai_triggered()
{
    MobileNetSSDConfigDialog dialog(this);
    dialog.setConfiguration(m_mobilenetConfig);
    
    if (dialog.exec() == QDialog::Accepted) {
        MobileNetSSDConfigDialog::MobileNetSSDConfig config = dialog.getConfiguration();
        
        // 保存配置
        m_mobilenetConfig = config;
        
        // 尝试加载模型
        if (m_ssdProcessor->loadModel(config)) {
            m_useCustomAlgorithm = true;
            
            // 显示加载成功信息
            QString message = QString("MobileNet SSD 模型配置成功！\n\n"
                                    "模型文件: %1\n"
                                    "配置文件: %2\n"
                                    "置信度阈值: %3\n"
                                    "NMS阈值: %4\n"
                                    "启用类别数: %5")
                              .arg(QFileInfo(config.modelPath).fileName())
                              .arg(QFileInfo(config.configPath).fileName()) 
                              .arg(config.confidenceThreshold, 0, 'f', 2)
                              .arg(config.nmsThreshold, 0, 'f', 2)
                              .arg(config.enabledClasses.size());
            
            QMessageBox::information(this, "配置成功", message);
            
            // 更新状态栏
            statusBar()->showMessage("算法模式: MobileNet SSD 目标检测", 3000);
            
            qDebug() << "[MainWindow] MobileNet SSD模型加载成功:" << config.modelPath;
            
        } else {
            QMessageBox::critical(this, "加载失败", 
                                 "无法加载MobileNet SSD模型！\n\n请检查：\n"
                                 "1. 模型文件路径是否正确\n"
                                 "2. 配置文件路径是否正确\n" 
                                 "3. OpenCV是否支持TensorFlow模型");
            m_useCustomAlgorithm = false;
        }
        
        // 保存配置到设置
        saveMobileNetSSDConfig();
    }
}

void MainWindow::on_actionCurrent_Algorithm_triggered()
{
    // 创建简单的算法切换对话框
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("算法模式切换");
    msgBox.setIcon(QMessageBox::Question);
    
    if (m_useCustomAlgorithm && m_ssdProcessor->isModelLoaded()) {
        msgBox.setText("当前正在使用 MobileNet SSD 目标检测算法。");
        msgBox.setInformativeText("选择要执行的操作：");
        
        QPushButton* configButton = msgBox.addButton("配置参数", QMessageBox::ActionRole);
        QPushButton* disableButton = msgBox.addButton("禁用AI算法", QMessageBox::DestructiveRole);
        QPushButton* cancelButton = msgBox.addButton("取消", QMessageBox::RejectRole);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == configButton) {
            // 打开配置界面
            on_actionImport_Algorithm_Ai_triggered();
        } else if (msgBox.clickedButton() == disableButton) {
            // 禁用AI算法
            m_useCustomAlgorithm = false;
            m_ssdProcessor->unloadModel();
            statusBar()->showMessage("算法模式: 普通算法", 3000);
            saveMobileNetSSDConfig();
            
            QMessageBox::information(this, "算法切换", "已切换到普通算法模式。");
        }
    } else {
        msgBox.setText("当前正在使用普通图像处理算法。");
        msgBox.setInformativeText("是否要配置并启用 MobileNet SSD 目标检测算法？");
        
        QPushButton* enableButton = msgBox.addButton("配置AI算法", QMessageBox::AcceptRole);
        QPushButton* cancelButton = msgBox.addButton("取消", QMessageBox::RejectRole);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == enableButton) {
            // 打开配置界面
            on_actionImport_Algorithm_Ai_triggered();
        }
    }
}

void MainWindow::loadMobileNetSSDConfig()
{
    QSettings settings("QOMIPPlatform", "MobileNetSSD");
    
    // 加载是否启用自定义算法
    m_useCustomAlgorithm = settings.value("use_custom_algorithm", false).toBool();
    
    // 加载MobileNet SSD配置
    m_mobilenetConfig.modelPath = settings.value("model_path", "").toString();
    m_mobilenetConfig.configPath = settings.value("config_path", "").toString();
    
    // 加载后处理参数
    m_mobilenetConfig.confidenceThreshold = settings.value("confidence_threshold", 0.5).toFloat();
    m_mobilenetConfig.nmsThreshold = settings.value("nms_threshold", 0.4).toFloat();
    m_mobilenetConfig.useNMS = settings.value("use_nms", true).toBool();
    m_mobilenetConfig.maxDetections = settings.value("max_detections", 100).toInt();
    
    // 加载性能参数
    m_mobilenetConfig.backend = static_cast<cv::dnn::Backend>(
        settings.value("backend", static_cast<int>(cv::dnn::DNN_BACKEND_OPENCV)).toInt());
    m_mobilenetConfig.target = static_cast<cv::dnn::Target>(
        settings.value("target", static_cast<int>(cv::dnn::DNN_TARGET_CPU)).toInt());
    m_mobilenetConfig.enableOptimization = settings.value("enable_optimization", true).toBool();
    
    // 加载场景参数
    m_mobilenetConfig.personOnly = settings.value("person_only", false).toBool();
    m_mobilenetConfig.enabledClasses = settings.value("enabled_classes", 
        m_mobilenetConfig.enabledClasses).toStringList();
    
    // 加载显示参数
    m_mobilenetConfig.showLabels = settings.value("show_labels", true).toBool();
    m_mobilenetConfig.showConfidence = settings.value("show_confidence", true).toBool();
    m_mobilenetConfig.fontSize = settings.value("font_size", 12).toInt();
    
    // 如果启用了自定义算法且有有效的模型路径，尝试加载模型
    if (m_useCustomAlgorithm && 
        !m_mobilenetConfig.modelPath.isEmpty() && 
        !m_mobilenetConfig.configPath.isEmpty()) {
        
        if (QFileInfo::exists(m_mobilenetConfig.modelPath) && 
            QFileInfo::exists(m_mobilenetConfig.configPath)) {
            
            if (m_ssdProcessor->loadModel(m_mobilenetConfig)) {
                qDebug() << "[MainWindow] 启动时成功加载MobileNet SSD模型";
            } else {
                qDebug() << "[MainWindow] 启动时加载MobileNet SSD模型失败，禁用自定义算法";
                m_useCustomAlgorithm = false;
            }
        } else {
            qDebug() << "[MainWindow] MobileNet SSD模型文件不存在，禁用自定义算法";
            m_useCustomAlgorithm = false;
        }
    }
}

void MainWindow::saveMobileNetSSDConfig()
{
    QSettings settings("QOMIPPlatform", "MobileNetSSD");
    
    // 保存是否启用自定义算法
    settings.setValue("use_custom_algorithm", m_useCustomAlgorithm);
    
    // 保存MobileNet SSD配置
    settings.setValue("model_path", m_mobilenetConfig.modelPath);
    settings.setValue("config_path", m_mobilenetConfig.configPath);
    
    // 保存后处理参数
    settings.setValue("confidence_threshold", m_mobilenetConfig.confidenceThreshold);
    settings.setValue("nms_threshold", m_mobilenetConfig.nmsThreshold);
    settings.setValue("use_nms", m_mobilenetConfig.useNMS);
    settings.setValue("max_detections", m_mobilenetConfig.maxDetections);
    
    // 保存性能参数
    settings.setValue("backend", static_cast<int>(m_mobilenetConfig.backend));
    settings.setValue("target", static_cast<int>(m_mobilenetConfig.target));
    settings.setValue("enable_optimization", m_mobilenetConfig.enableOptimization);
    
    // 保存场景参数
    settings.setValue("person_only", m_mobilenetConfig.personOnly);
    settings.setValue("enabled_classes", m_mobilenetConfig.enabledClasses);
    
    // 保存显示参数
    settings.setValue("show_labels", m_mobilenetConfig.showLabels);
    settings.setValue("show_confidence", m_mobilenetConfig.showConfidence);
    settings.setValue("font_size", m_mobilenetConfig.fontSize);
    
    settings.sync();
}

// 导出当前视频方法
void MainWindow::exportCurrentVideo()
{
    QString currentSource = getCurrentVideoSource();
    if (currentSource.isEmpty()) {
        QMessageBox::warning(this, "警告", "当前没有视频源或当前源不是视频文件");
        return;
    }
    
    // 选择导出目录
    QString exportDir = QFileDialog::getExistingDirectory(this, "选择导出目录", 
        QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (exportDir.isEmpty()) {
        return; // 用户取消
    }
    
    // 只导出当前选中widget的算法
    QStringList sources;
    sources << currentSource;
    performVideoExport(sources, exportDir, true); // true表示只用当前算法
}

// 导出所有算法的视频方法
void MainWindow::exportAllVideos()
{
    QString currentSource = getCurrentVideoSource();
    if (currentSource.isEmpty()) {
        QMessageBox::warning(this, "警告", "当前没有视频源或当前源不是视频文件");
        return;
    }
    
    // 选择导出目录
    QString exportDir = QFileDialog::getExistingDirectory(this, "选择导出目录", 
        QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (exportDir.isEmpty()) {
        return; // 用户取消
    }
    
    // 使用当前视频源，但导出所有算法
    QStringList sources;
    sources << currentSource;
    performVideoExport(sources, exportDir, false); // false表示使用所有算法
}

// 获取所有视频源
QStringList MainWindow::getVideoSources() const
{
    QStringList videoSources;
    
    // 遍历视频列表中的所有文件
    for (int i = 0; i < ui->V_ListWidget->count(); i++) {
        QListWidgetItem* item = ui->V_ListWidget->item(i);
        if (item) {
            QString filePath = item->text();
            QFileInfo fileInfo(filePath);
            
            // 检查文件是否存在且是视频格式
            if (fileInfo.exists() && isVideoFile(filePath)) {
                videoSources << filePath;
            }
        }
    }
    
    return videoSources;
}

// 获取当前视频源
QString MainWindow::getCurrentVideoSource() const
{
    // 检查Reader的当前源类型
    if (m_reader->getSourceType() != Reader::SOURCE_FILE) {
        return QString(); // 不是文件源
    }
    
    QString currentPath = m_reader->getCurrentSourcePath();
    if (currentPath.isEmpty() || !isVideoFile(currentPath)) {
        return QString();
    }
    
    return currentPath;
}

// 执行视频导出
void MainWindow::performVideoExport(const QStringList &sources, const QString &exportDir, bool currentOnly)
{
    if (sources.isEmpty()) {
        return;
    }
    
    // 如果正在播放，先暂停
    bool wasPlaying = ui->playButton->isChecked();
    if (wasPlaying) {
        ui->playButton->click(); // 暂停播放
    }
    
    // 预先获取所有需要的算法配置，避免在导出过程中重复访问
    QVector<WidgetExportConfig> exportConfigs;
    
    if (currentOnly) {
        // 只导出当前选中widget的算法
        int currentIndex = ui->videoWidget->currentIndex();
        if (currentIndex >= 0 && currentIndex < m_vectorWidget.size()) {
            BasicViewWidget* currentWidget = m_vectorWidget[currentIndex];
            if (currentWidget) {
                QString widgetName = currentWidget->getWidgetName();
                QVector<Algorithm*> algorithms = currentWidget->getAlgorithms();
                
                // 只有当widget有算法时才添加配置
                if (!algorithms.isEmpty()) {
                    WidgetExportConfig config;
                    config.widgetName = widgetName;
                    config.algorithms = algorithms;
                    exportConfigs.append(config);
                }
            }
        }
    } else {
        // 预先获取所有BasicViewWidget的算法配置
        for (BasicViewWidget* widget : m_vectorWidget) {
            if (widget) {
                QString widgetName = widget->getWidgetName();
                QVector<Algorithm*> algorithms = widget->getAlgorithms();
                
                // 只有当widget有算法时才添加配置
                if (!algorithms.isEmpty()) {
                    WidgetExportConfig config;
                    config.widgetName = widgetName;
                    config.algorithms = algorithms;
                    exportConfigs.append(config);
                }
            }
        }
    }
    
    // 创建导出进度对话框
    ExportProgressDialog* progressDialog = new ExportProgressDialog(this);
    progressDialog->show();
    
    // 为每个视频源创建导出任务
    int totalExports = 0;
    bool exportCancelled = false;
    
    connect(progressDialog, &ExportProgressDialog::cancelRequested, [&exportCancelled]() {
        exportCancelled = true;
    });
    
    for (int i = 0; i < sources.size() && !exportCancelled; i++) {
        const QString& sourcePath = sources[i];
        
        // 创建VideoExporter实例
        VideoExporter* exporter = new VideoExporter(this);
        exporter->setSourceVideo(sourcePath);
        
        // 直接批量设置预处理好的配置，避免重复调用getAlgorithms()
        exporter->setWidgetConfigs(exportConfigs);
        
        // 连接进度信号
        connect(exporter, &VideoExporter::exportProgress, 
                [progressDialog](int widgetIndex, int totalWidgets, int frameIndex, int totalFrames, const QString &currentFile) {
                    // 计算整体进度：每个widget占总进度的一部分，当前widget内的帧进度
                    int widgetProgress = (widgetIndex * 100) / totalWidgets;
                    int frameProgress = (frameIndex * 100) / (totalWidgets * totalFrames);
                    int totalProgress = widgetProgress + frameProgress;
                    
                    progressDialog->setValue(totalProgress);
                    progressDialog->setCurrentFile(currentFile);
                });
        // 连接完成和错误信号到清理逻辑
        connect(exporter, &VideoExporter::exportCompleted, [exporter, progressDialog, &totalExports](int count) {
            totalExports += count;
            if (count > 0) {
                progressDialog->setValue(100);
                progressDialog->setExportComplete();
            }
            exporter->deleteLater(); // 在完成后再删除
        });
        connect(exporter, &VideoExporter::exportError, [exporter, progressDialog](const QString& error) {
            QMessageBox::critical(progressDialog, "导出错误", error);
            exporter->deleteLater(); // 在错误后再删除
        });
        connect(exporter, &VideoExporter::exportCancelled, [exporter, &exportCancelled]() {
            exportCancelled = true;
            exporter->deleteLater(); // 在取消后再删除
        });
        
        // 连接取消信号到导出器
        connect(progressDialog, &ExportProgressDialog::cancelRequested, exporter, &VideoExporter::cancelExport);
        
        // 开始导出
        qDebug() << "Starting export for source:" << sourcePath;
        exporter->startExport(exportDir);
    }
    
    if (!exportCancelled && totalExports > 0) {
        QMessageBox::information(progressDialog, "导出完成", 
            QString("已成功导出 %1 个处理后的视频文件到:\n%2").arg(totalExports).arg(exportDir));
    }
    
    progressDialog->accept();
}

// 检查文件是否是视频格式的辅助函数
bool MainWindow::isVideoFile(const QString &filePath) const
{
    QStringList videoExtensions = {"mp4", "avi", "mov", "wmv", "flv", "mkv", "webm", "m4v", "3gp", "mpg", "mpeg"};
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    return videoExtensions.contains(extension);
}
