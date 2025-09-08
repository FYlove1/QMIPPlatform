#include "basicviewwidget.h"
#include "ui_basicviewwidget.h"
#include "CommonUtils.h"
#include "algorithmdialog.h"
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QMenu> // 添加此头文件
#include <QContextMenuEvent> // 可能也需要添加

BasicViewWidget::BasicViewWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BasicViewWidget)
    , m_processor(new FrameProcessor())
{
    ui->setupUi(this);

    /* 把场景挂到 view */
    ui->graphicsView->setScene(&m_scene);

    /* 拖拽 / 滚轮 由 view 自己完成 */
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    /* 把 pixmapItem 放进场景 */
    m_scene.addItem(&m_pixItem);
    
    /* 连接处理器信号 */
    connect(m_processor, &FrameProcessor::frameProcessed,
            this, &BasicViewWidget::onFrameProcessed);
    
    /* 启动处理器 */
    m_processor->startProcessing();
}


BasicViewWidget::~BasicViewWidget() 
{ 
    // 确保完全终止线程
    if (m_processor) {
        m_processor->terminateProcessing();
        delete m_processor;
        m_processor = nullptr;
    }
    delete ui; 
}

void BasicViewWidget::setImage(const cv::Mat &img)
{
    if (img.empty()) return;
    m_pixItem.setPixmap(CU::matToPixmap(img,false));
    ui->graphicsView->fitInView(&m_pixItem, Qt::KeepAspectRatio);
    //m_scale = 1.0;
}

// void BasicViewWidget::setAlgorithmType(int type)
// {
//     m_processor->setAlgorithmType(type);
// }

void BasicViewWidget::processFrame(const cv::Mat& frame)
{
    if (!frame.empty()) {
        m_processor->enqueueFrame(frame);
    }
}

void BasicViewWidget::onFrameProcessed(const cv::Mat& result)
{
    // 存储当前处理后的帧用于导出
    result.copyTo(m_currentFrame);
    setImage(result);
}

/* 滚轮缩放 */
void BasicViewWidget::wheelEvent(QWheelEvent *e)
{
    const double factor = 1.15;
    if (e->angleDelta().y() > 0)
        m_scale *= factor;
    else
        m_scale /= factor;
    ui->graphicsView->resetTransform();
    ui->graphicsView->scale(m_scale, m_scale);
}

/* 双击还原 */
void BasicViewWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    ui->graphicsView->fitInView(&m_pixItem, Qt::KeepAspectRatio);
    m_scale = 1.0;
}

void BasicViewWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    
    // 添加算法管理菜单项
    QAction *algorithmAction = menu.addAction("算法管理");
    connect(algorithmAction, &QAction::triggered, this, &BasicViewWidget::showAlgorithmManager);
    
    // 可以添加其他菜单项...
    menu.addSeparator();
    QAction *resetAction = menu.addAction("重置视图");
    connect(resetAction, &QAction::triggered, [this]() {
        // 重置视图的代码
    });
    
    menu.exec(event->globalPos());
}

void BasicViewWidget::showAlgorithmManager()
{
    // 创建并显示算法管理对话框
    AlgorithmDialog dialog(m_processor, this);
    dialog.setWindowTitle(QString("算法管理 - %1").arg(objectName()));
    dialog.exec();
}

cv::Mat BasicViewWidget::getCurrentProcessedFrame() const
{
    return m_currentFrame;
}

QString BasicViewWidget::getWidgetName() const
{
    QString name = objectName();
    return name.isEmpty() ? "UnnamedWidget" : name;
}

QVector<Algorithm*> BasicViewWidget::getAlgorithms() const
{
    if (m_processor && m_processor->algorithmModel()) {
        return m_processor->algorithmModel()->getAllAlgorithms();
    }
    return QVector<Algorithm*>();
}
