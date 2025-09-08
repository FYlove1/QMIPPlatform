#pragma once

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @class Reader
 * @brief 视频读取工作类，负责从视频文件或摄像头读取帧并定时发送
 * 
 * 设计为与QThread配合使用，通过moveToThread移动到线程中执行
 */
class Reader : public QObject {
    Q_OBJECT
public:
    /**
     * @brief 输入源类型枚举
     */
    enum SourceType {
        SOURCE_NONE,    ///< 无输入源
        SOURCE_FILE,    ///< 文件输入
        SOURCE_CAMERA   ///< 摄像头输入
    };
    
    /**
     * @brief 构造函数
     * @param parent 父QObject对象
     */
    explicit Reader(QObject *parent = nullptr);
    ~Reader();
    
    

public slots:
    /**
     * @brief 开始/继续播放视频
     */
    void play();
    
    /**
     * @brief 暂停视频播放
     */
    void pause();
    
    /**
     * @brief 停止视频播放并清理资源
     */
    void stop();
    
    /**
     * @brief 读取并处理一帧视频
     */
    void processFrame();

    /**
     * @brief 设置视频源文件
     * @param file 视频文件路径
     */
    void setSource(const QString &file);
    
    /**
     * @brief 设置摄像头源
     * @param cameraIndex 摄像头索引
     */
    void setCameraSource(int cameraIndex);
    
    /**
     * @brief 设置需要处理的视频窗口数量
     * @param count 窗口数量
     */
    void setViewCount(int count);
    
    /**
     * @brief 设置视频播放的帧率
     * @param fps 每秒帧数，默认为30
     */
    void setFrameRate(int fps = 30);
    
    /**
     * @brief 获取当前输入源类型
     */
    SourceType getSourceType() const { return m_sourceType; }
    
    // 获取当前源文件路径
    QString getCurrentSourcePath() const { return m_path; }
    
    // 获取当前摄像头索引
    int getCurrentCameraIndex() const { return m_cameraIndex; }
    
signals:
    /**
     * @brief 当新帧准备好时发出此信号
     * @param mats 帧组，包含原始帧和多种处理效果的帧
     */
    void frameReady(const cv::Mat& frame);
    
    
    /**
     * @brief 视频处理已完成（结束或出错）
     * @param message 完成信息
     */
    void processingFinished(const QString &message);

private:
    QString m_path;             ///< 视频文件路径
    int m_cameraIndex = -1;     ///< 摄像头索引
    SourceType m_sourceType = SOURCE_NONE;  ///< 当前输入源类型
    QMutex m_mutex;             ///< 互斥锁，用于线程安全
    bool m_running = true;      ///< 运行标志
    bool m_play = false;        ///< 播放状态标志
    int r_videoNumber = 1;      ///< 需要输出的矩阵个数
    cv::VideoCapture m_cap;     ///< OpenCV视频捕获对象
    int m_frameInterval = 33;   ///< 帧间隔(毫秒)，默认33ms约30fps
    QTimer *m_timer;            ///< 定时器，用于控制帧读取频率
    
    /**
     * @brief 打开输入源（文件或摄像头）
     */
    bool openSource();
};