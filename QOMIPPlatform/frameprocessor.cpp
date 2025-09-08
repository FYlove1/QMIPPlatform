#include "frameprocessor.h"
#include "CommonUtils.h"
#include <QDebug>

FrameProcessor::FrameProcessor(QObject *parent)
    : QObject(parent), m_running(false), m_algorithmModel(new AlgorithmListModel(this))
{
    // 将处理器移到专用线程
    moveToThread(&m_thread);
    
    // 连接线程启动信号到处理槽
    connect(&m_thread, &QThread::started, this, &FrameProcessor::processFrames);
}

FrameProcessor::~FrameProcessor()
{
    terminateProcessing(); 
}

// 添加算法
void FrameProcessor::addAlgorithm(int algorithmId, const QVariantMap &params)
{
    if (m_algorithmModel) {
        m_algorithmModel->addAlgorithm(algorithmId, QString(), params);
    }
}

// 删除算法
bool FrameProcessor::removeAlgorithm(int index)
{
    if (m_algorithmModel) {
        return m_algorithmModel->removeAlgorithm(index);
    }
    return false;
}

// 更新算法参数
bool FrameProcessor::updateAlgorithmParams(int index, const QVariantMap &params)
{
    if (m_algorithmModel) {
        return m_algorithmModel->updateAlgorithmParameters(index, params);
    }
    return false;
}

void FrameProcessor::enqueueFrame(const cv::Mat& frame)
{
    if (!m_running) return;
    
    
    
    // 如果队列太长，可能丢弃旧帧以避免内存问题
    if (m_frameQueue.size() > 5) {
        m_frameQueue.dequeue();
    }
    
    m_frameQueue.enqueue(frame.clone());
    m_condition.wakeOne();
}

void FrameProcessor::startProcessing()
{
    
    m_running = true;
    
    // 如果线程未运行，启动它
    if (!m_thread.isRunning()) {
        // 确保信号连接正确
        disconnect(&m_thread, &QThread::started, this, &FrameProcessor::processFrames);
        connect(&m_thread, &QThread::started, this, &FrameProcessor::processFrames);
        m_thread.start();
    } else {
        // 线程已运行，唤醒它继续处理
        m_condition.wakeAll();
    }
}

void FrameProcessor::stopProcessing()
{
  
    m_running = false;
    m_condition.wakeAll();  // 唤醒线程，让它检查m_running标志
    
    // 清空队列
    m_frameQueue.clear();
}

void FrameProcessor::terminateProcessing()
{
    {
        QMutexLocker locker(&m_mutex);
        m_running = false;
        m_frameQueue.clear();
        m_condition.wakeAll();
    }
    
    // 请求线程中断
    m_thread.requestInterruption();
    
    // 退出事件循环并等待线程结束
    if (m_thread.isRunning()) {
        m_thread.quit();
        if (!m_thread.wait(3000)) {
            qWarning() << "FrameProcessor 线程无法在3秒内终止，强制终止";
            m_thread.terminate();
            m_thread.wait(); // 即使在terminate后也需要wait
        }
    }
    
    // 不需要再次调用quit()
}

void FrameProcessor::processFrames()
{
    while (!m_thread.isInterruptionRequested()) {
        cv::Mat frame;
        //QVector<QPair<int, QVariantMap>> algorithms;
        
        {
            // 如果没有运行或队列为空，则等待
            while (!m_running || m_frameQueue.isEmpty()) {
                // 如果线程被请求中断，则退出
                if (m_thread.isInterruptionRequested()) {
                    return;
                }
                
                // // 使用超时等待，以便定期检查中断状态
                // if (!m_condition.wait(&m_mutex, 500)) {
                //     // 超时后再次检查中断请求
                //     if (m_thread.isInterruptionRequested()) {
                //         return;  // 立即返回，结束线程
                //     }
                //     continue;
                // }
                if (m_thread.isInterruptionRequested()) {
                    return;  // 立即返回，结束线程
                }
                
                // 再次检查运行状态
                if (!m_running && m_frameQueue.isEmpty()) {
                    continue;  // 如果停止运行且没有帧，则继续等待
                }
            }
            
            // 取出队首帧
            frame = m_frameQueue.dequeue();
        }
        
        // 获取当前算法的克隆列表
        QVector<Algorithm*> algorithms = m_algorithmModel->getAllAlgorithms();
        
        try {
            // 获取当前算法列表
            QVector<Algorithm*> algorithms = m_algorithmModel->getAllAlgorithms();
            
            // 初始结果为输入帧
            cv::Mat result = frame.clone();
            
            // 依次应用每个算法
            for (Algorithm* algorithm : algorithms) {
                if (algorithm) {
                    result = algorithm->process(result);
                }
            }
            
            // 发送处理结果
            emit frameProcessed(result);
            
            // 清理克隆的算法实例
            qDeleteAll(algorithms);
        }
        catch (const cv::Exception& e) {
            qWarning() << "OpenCV错误:" << e.what();
        }
        catch (const std::exception& e) {
            qWarning() << "标准异常:" << e.what();
        }
    }
}

// 获取算法数量
int FrameProcessor::getAlgorithmCount() const
{
    if (m_algorithmModel) {
        return m_algorithmModel->rowCount();
    }
    return 0;
}

// 获取算法ID
int FrameProcessor::getAlgorithmId(int index) const
{
    if (m_algorithmModel) {
        return m_algorithmModel->getAlgorithmId(index);
    }
    return -1;
}

// 获取算法名称
QString FrameProcessor::getAlgorithmName(int index) const
{
    if (m_algorithmModel) {
        return m_algorithmModel->getAlgorithmName(index);
    }
    return QString();
}

// 获取算法参数
QVariantMap FrameProcessor::getAlgorithmParams(int index) const
{
    if (m_algorithmModel) {
        return m_algorithmModel->getAlgorithmParams(index);
    }
    return QVariantMap();
}
