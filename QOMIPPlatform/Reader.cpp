#include "Reader.h"
#include <QDebug>

Reader::Reader(QObject *parent)
    : QObject(parent), m_running(true), m_play(false)
{
    // 创建并配置定时器
    m_timer = new QTimer(this);
    m_timer->setInterval(m_frameInterval);
    
    // 连接定时器的timeout信号到处理帧的槽函数
    connect(m_timer, &QTimer::timeout, this, &Reader::processFrame);
}

Reader::~Reader()
{
    // 确保资源被正确释放
    stop();
}

void Reader::setSource(const QString &file) {
    QMutexLocker lock(&m_mutex);
    m_path = file;
    m_sourceType = SOURCE_FILE;
    m_cameraIndex = -1;  // 清除摄像头索引
    
    // 如果已经打开一个视频，先关闭它
    if (m_cap.isOpened()) {
        m_cap.release();
    }

    qDebug()<<"Current Source is file: "<<file;
}

void Reader::setCameraSource(int cameraIndex) {
    QMutexLocker lock(&m_mutex);
    m_cameraIndex = cameraIndex;
    m_sourceType = SOURCE_CAMERA;
    m_path.clear();  // 清除文件路径
    
    // 如果已经打开一个源，先关闭它
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    
    qDebug() << "Current Source is camera index:" << cameraIndex;
}

void Reader::setViewCount(int count) {
    QMutexLocker lock(&m_mutex);
    r_videoNumber = qMax(1, count); // 至少需要1个视图
}

void Reader::setFrameRate(int fps) {
    QMutexLocker lock(&m_mutex);
    m_frameInterval = fps > 0 ? 1000 / fps : 33; // 将fps转换为毫秒间隔
    
    // 如果定时器正在运行，更新其间隔
    if (m_timer->isActive()) {
        m_timer->setInterval(m_frameInterval);
    }
}

void Reader::play() { 
    QMutexLocker lock(&m_mutex); 
    m_play = true;
    
    // 如果定时器未启动，则启动它
    if (!m_timer->isActive()) {
        m_timer->start();
    }
    qDebug() << "[Reader]:Now , play function is called";
}

void Reader::pause() { 
    QMutexLocker lock(&m_mutex); 
    m_play = false; 
}

void Reader::stop() { 
    QMutexLocker lock(&m_mutex); 
    m_running = false; 
    m_play = false;
    
    // 停止定时器
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    
    m_path.clear();
    m_cameraIndex = -1;
    m_sourceType = SOURCE_NONE;
    
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    
    emit processingFinished("视频处理已停止");
}

bool Reader::openSource() {
    // 此方法假设已经持有锁
    if (m_cap.isOpened()) {
        return true;  // 已经打开
    }
    
    bool success = false;
    
    switch (m_sourceType) {
        case SOURCE_FILE:
            if (!m_path.isEmpty()) {
                success = m_cap.open(m_path.toStdString());
                if (!success) {
                    qWarning() << "无法打开视频文件：" << m_path;
                    emit processingFinished("无法打开视频文件: " + m_path);
                }
            }
            break;
            
        case SOURCE_CAMERA:
            if (m_cameraIndex >= 0) {
                // 尝试使用适合平台的后端打开摄像头
                #ifdef Q_OS_WIN
                    success = m_cap.open(m_cameraIndex, cv::CAP_DSHOW);
                #elif defined(Q_OS_MAC)
                    success = m_cap.open(m_cameraIndex, cv::CAP_AVFOUNDATION);
                #elif defined(Q_OS_LINUX)
                    success = m_cap.open(m_cameraIndex, cv::CAP_V4L2);
                #else
                    success = m_cap.open(m_cameraIndex);
                #endif
                
                if (!success) {
                    // 如果特定后端失败，尝试默认后端
                    success = m_cap.open(m_cameraIndex);
                }
                
                if (!success) {
                    qWarning() << "无法打开摄像头：" << m_cameraIndex;
                    emit processingFinished(QString("无法打开摄像头 %1").arg(m_cameraIndex));
                } else {
                    // 设置摄像头参数（可选）
                    // m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
                    // m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
                    // m_cap.set(cv::CAP_PROP_FPS, 30);
                    qDebug() << "Successfully opened camera" << m_cameraIndex;
                }
            }
            break;
            
        case SOURCE_NONE:
        default:
            qWarning() << "No source set";
            return false;
    }
    
    return success;
}

// 在 Reader.cpp 中修改
void Reader::processFrame() {
    QMutexLocker lock(&m_mutex);
    
    // 检查是否应该继续运行
    if (!m_running) {
        return;
    }
    
    // 如果没有设置为播放状态，不处理
    if (!m_play) {
        return;
    }
    
    // 如果视频捕获未打开，尝试打开
    if (!m_cap.isOpened()) {
        if (!openSource()) {
            return;  // 无法打开源
        }
    }
    
    // 读取一帧
    cv::Mat frame;
    if (m_cap.read(frame)) {
        if (!frame.empty()) {
            // 直接发送原始帧，不再创建帧组
            emit frameReady(frame);
            qDebug() << "已读取并发送一帧";
        }
    } else {
        // 处理结束情况
        if (m_sourceType == SOURCE_FILE) {
            // 视频文件结束
            qDebug() << "视频播放完毕";
            m_play = false;
            
            emit processingFinished("视频播放完毕");
            
            // 重置视频，便于再次播放
            if (m_cap.isOpened()) {
                m_cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            }
        } else if (m_sourceType == SOURCE_CAMERA) {
            // 摄像头读取失败，可能是暂时的问题
            qWarning() << "Failed to read frame from camera";
            // 不停止播放，继续尝试
        }
    }
}
