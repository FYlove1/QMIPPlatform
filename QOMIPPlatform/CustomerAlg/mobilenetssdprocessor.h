#ifndef MOBILENETSSDPROCESSOR_H
#define MOBILENETSSDPROCESSOR_H

#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"
#include "mobilenetssdconfigdialog.h"
#include "airesultvisualizer.h"
#include <vector>
#include <string>
#include <memory>
#include <chrono>

class MobileNetSSDProcessor
{
public:
    // 性能统计结构
    struct PerformanceStats {
        double preprocessTime = 0.0;    // 预处理时间(ms)
        double inferenceTime = 0.0;     // 推理时间(ms)
        double postprocessTime = 0.0;   // 后处理时间(ms)
        double totalTime = 0.0;         // 总时间(ms)
        double fps = 0.0;               // 帧率
        int detectionCount = 0;         // 检测数量
        int frameSkipCount = 0;         // 跳帧次数
        
        void reset() {
            preprocessTime = inferenceTime = postprocessTime = totalTime = fps = 0.0;
            detectionCount = frameSkipCount = 0;
        }
    };

public:
    MobileNetSSDProcessor();
    ~MobileNetSSDProcessor();
    
    // 模型管理
    bool loadModel(const MobileNetSSDConfigDialog::MobileNetSSDConfig& config);
    void unloadModel();
    bool isModelLoaded() const { return m_modelLoaded; }
    
    // 主要处理函数
    cv::Mat processFrame(const cv::Mat& inputFrame);
    
    // 配置管理
    void setConfiguration(const MobileNetSSDConfigDialog::MobileNetSSDConfig& config);
    MobileNetSSDConfigDialog::MobileNetSSDConfig getConfiguration() const { return m_config; }
    
    // 性能统计
    PerformanceStats getPerformanceStats() const { return m_perfStats; }
    void resetPerformanceStats() { m_perfStats.reset(); }
    
    // 结果获取
    AIResultVisualizer::AIResult getLastResults() const { return m_lastResults; }
    
    // 实时参数调整（无需重新加载模型）
    void setConfidenceThreshold(float threshold);
    void setNMSThreshold(float threshold);
    void setEnabledClasses(const QStringList& classes);
    void setPersonOnlyMode(bool enabled);

private:
    // 核心处理函数
    cv::Mat preprocessImage(const cv::Mat& image);
    std::vector<cv::Mat> runInference(const cv::Mat& blob);
    AIResultVisualizer::AIResult postProcessDetections(const std::vector<cv::Mat>& outputs, 
                                                      const cv::Size& originalSize);
    
    // 结果过滤和优化
    void filterDetectionsByClass(std::vector<AIResultVisualizer::DetectionBox>& detections);
    void applyNMS(std::vector<AIResultVisualizer::DetectionBox>& detections);
    void limitMaxDetections(std::vector<AIResultVisualizer::DetectionBox>& detections);
    
    // 辅助函数
    cv::Rect denormalizeRect(float left, float top, float right, float bottom, 
                           const cv::Size& imageSize);
    int getClassIndex(const QString& className);
    bool isClassEnabled(int classIndex);
    
    // 性能监控
    void startTimer() { m_timer = std::chrono::high_resolution_clock::now(); }
    double getElapsedTime() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - m_timer).count();
    }
    
    // COCO类别名称
    void initCOCOClassNames();

private:
    // 模型相关
    cv::dnn::Net m_net;                                        // OpenCV DNN网络
    bool m_modelLoaded;                                        // 模型加载状态
    MobileNetSSDConfigDialog::MobileNetSSDConfig m_config;   // 当前配置
    
    // 结果和可视化
    AIResultVisualizer::AIResult m_lastResults;              // 最后检测结果
    std::unique_ptr<AIResultVisualizer> m_visualizer;        // 结果可视化器
    
    // 性能统计和跳帧控制
    PerformanceStats m_perfStats;                            // 性能统计数据
    std::chrono::high_resolution_clock::time_point m_timer; // 计时器
    int m_frameCounter;                                      // 帧计数器
    int m_frameSkipInterval;                                 // 跳帧间隔（每N帧处理一次）
    
    // COCO数据集类别
    QStringList m_cocoClassNames;                            // COCO类别名称列表
    
    // 缓存的检测结果（用于NMS等操作）
    std::vector<cv::Rect> m_boxes;
    std::vector<float> m_confidences;
    std::vector<int> m_classIds;
    std::vector<int> m_indices;
};

#endif // MOBILENETSSDPROCESSOR_H