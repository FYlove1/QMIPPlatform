#pragma once

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QVector>
#include <QPair>
#include <QString>
#include <QVariantMap>
#include <opencv2/opencv.hpp>
#include "algorithmlistmodel.h"
#include "Algorithms/algorithm.h" // 添加这行确保Algorithm类可用
/**
 * @class FrameProcessor
 * @brief 视频帧处理器，支持多算法处理队列
 */
class FrameProcessor : public QObject {
    Q_OBJECT
public:
    explicit FrameProcessor(QObject *parent = nullptr);
    ~FrameProcessor();
    AlgorithmListModel* algorithmModel() const { return m_algorithmModel; }
    
    // 添加算法
    void addAlgorithm(int algorithmId, const QVariantMap &params = QVariantMap());
    
    // 移除算法
    bool removeAlgorithm(int index);
    
    // 清空算法列表
    void clearAlgorithms();
    
    // 更新算法参数
    bool updateAlgorithmParams(int index, const QVariantMap &params);
    
    // 获取算法信息列表（线程安全）
    QVector<QPair<QString, int>> getAlgorithmInfo() const;
    
    // 获取算法数量
    int getAlgorithmCount() const;
    
    // 获取特定算法信息
    int getAlgorithmId(int index) const;
    QString getAlgorithmName(int index) const;
    QVariantMap getAlgorithmParams(int index) const;
    
    // 将帧添加到处理队列
    void enqueueFrame(const cv::Mat& frame);
    
    // 启动/停止处理
    void startProcessing();
    void stopProcessing();
    // 终止处理线程并清空队列
    void terminateProcessing();

signals:
    // 处理完成后发出信号
    void frameProcessed(const cv::Mat& result);
    
    // 处理错误
    void processingError(const QString& errorMessage);

private slots:
    // 处理队列中的帧
    void processFrames();

private:
    QThread m_thread;                    // 处理线程
    QQueue<cv::Mat> m_frameQueue;        // 帧队列
    mutable QMutex m_mutex;              // 互斥锁 (mutable使其可在const方法中使用)
    QWaitCondition m_condition;          // 条件变量
    bool m_running;                      // 运行标志
    
    AlgorithmListModel* m_algorithmModel; // 算法列表模型
};
