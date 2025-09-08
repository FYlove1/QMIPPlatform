#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "opencv2/opencv.hpp"
#include <QTime>
#include <QGraphicsScene>
#include <QPixmap>
#include <QVector>
#include <QThread>
#include <QMap>
#include <QListWidget>
#include "Reader.h"
#include "basicviewwidget.h"
#include "cameramanager.h"
#include "CustomerAlg/mobilenetssdconfigdialog.h"
#include "CustomerAlg/mobilenetssdprocessor.h"
#include "algorithmselectiondialog.h"

class DetachedWindow;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void on_playButton_clicked();
    void on_Reader_FrameReady(const cv::Mat &frame);
    void onProcessingFinished(const QString &message);
    void on_actionDelete_Current_Widget_triggered();
    void on_actionAdd_triggered();
    void on_actionSeparation_Current_Widget_triggered();
    void onDetachedWindowClosing(DetachedWindow *window);
    void onReturnToMainWindow(DetachedWindow *window);
    void onCameraListItemClicked(QListWidgetItem *item);
    void onCamerasUpdated(const QList<CameraManager::CameraInfo>& cameras);
    void refreshCameras();
    void on_actionImport_Algorithm_Ai_triggered();
    void on_actionCurrent_Algorithm_triggered();

private:
    Ui::MainWindow *ui;
    QThread *m_readerThread;    // 视频读取线程
    Reader *m_reader;           // 视频读取工作对象
    QVector<BasicViewWidget*> m_vectorWidget;  // 视图窗口列表
    int currentWidetCount;      // 当前视图窗口数量
    QMap<BasicViewWidget*, DetachedWindow*> m_detachedWindows;  // 分离窗口映射
    CameraManager *m_cameraManager;  // 摄像头管理器
    QListWidget *m_cameraListWidget;  // 摄像头列表控件
    
    // 算法配置相关
    MobileNetSSDConfigDialog::MobileNetSSDConfig m_mobilenetConfig;  // MobileNet SSD配置
    std::unique_ptr<MobileNetSSDProcessor> m_ssdProcessor;           // MobileNet SSD处理器
    bool m_useCustomAlgorithm;                                       // 是否使用自定义算法
    
    void initAll();                    // 初始化界面和组件
    void initCameraUI();               // 初始化摄像头UI
    void loadMobileNetSSDConfig();     // 加载MobileNet SSD配置
    void saveMobileNetSSDConfig();     // 保存MobileNet SSD配置
    
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
};
#endif // MAINWINDOW_H