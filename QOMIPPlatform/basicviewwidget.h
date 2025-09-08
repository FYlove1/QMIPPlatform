#ifndef BASICVIEWWIDGET_H
#define BASICVIEWWIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <opencv2/opencv.hpp>
#include "frameprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class BasicViewWidget; }
QT_END_NAMESPACE

class BasicViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BasicViewWidget(QWidget *parent = nullptr);
    ~BasicViewWidget();

    /* 把 Mat 显示到场景里；如果场景里已经有图则替换 */
    void setImage(const cv::Mat &img);
    
    /* 设置算法类型 */
    //void setAlgorithmType(int type);
    
    /* 处理新帧 */
    void processFrame(const cv::Mat& frame);
    FrameProcessor*     m_processor;     // 帧处理器
private slots:
    /* 处理完成后更新显示 */
    void onFrameProcessed(const cv::Mat& result);


protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    Ui::BasicViewWidget *ui;

    QGraphicsScene      m_scene;
    QGraphicsPixmapItem m_pixItem;
    double              m_scale = 1.0;   // 当前缩放比例
    
    

    /* 事件处理 */
    void wheelEvent(QWheelEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void showAlgorithmManager();
};

#endif // BASICVIEWWIDGET_H