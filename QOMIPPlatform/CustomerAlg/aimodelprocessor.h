#ifndef AIMODELPROCESSOR_H
#define AIMODELPROCESSOR_H

#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"
#include "algorithmimportdialog.h"
#include "airesultvisualizer.h"
#include <vector>
#include <string>
#include <memory>
#include <QString>

class AIModelProcessor
{
public:
    // 模型类型枚举
    enum ModelType {
        MODEL_UNKNOWN = 0,
        MODEL_YOLO_DETECTION,       // YOLO目标检测
        MODEL_SSD_DETECTION,        // SSD目标检测
        MODEL_CLASSIFICATION,       // 图像分类
        MODEL_POSE_ESTIMATION,      // 人体姿态估计
        MODEL_FACE_DETECTION,       // 人脸检测
        MODEL_FACE_LANDMARKS,       // 人脸关键点
        MODEL_SEGMENTATION,         // 语义分割
        MODEL_GENERIC              // 通用模型
    };

    // 后处理参数
    struct PostProcessParams {
        float confidenceThreshold = 0.5f;    // 置信度阈值
        float nmsThreshold = 0.4f;           // NMS阈值
        bool useNMS = true;                  // 是否使用NMS
        std::vector<std::string> classNames; // 类别名称列表
        
        PostProcessParams() = default;
    };

public:
    AIModelProcessor();
    ~AIModelProcessor();
    
    // 模型管理
    bool loadModel(const AlgorithmImportDialog::ModelConfig& config);
    bool loadModel(const std::string& modelPath, const std::string& configPath = "");
    void unloadModel();
    bool isModelLoaded() const { return m_modelLoaded; }
    
    // 主要处理函数
    cv::Mat processFrame(const cv::Mat& inputFrame);
    AIResultVisualizer::AIResult getLastResults() const { return m_lastResults; }
    
    // 配置函数
    void setPostProcessParams(const PostProcessParams& params);
    void setModelType(ModelType type);
    void setClassNames(const std::vector<std::string>& classNames);
    void setConfidenceThreshold(float threshold);
    void setNMSThreshold(float threshold);
    
    // 获取模型信息
    std::string getModelPath() const { return m_modelPath; }
    ModelType getModelType() const { return m_modelType; }
    cv::Size getInputSize() const { return cv::Size(m_blobParams.width, m_blobParams.height); }
    
    // 静态辅助函数
    static ModelType detectModelType(const std::string& modelPath);
    static std::vector<std::string> getDefaultClassNames(ModelType modelType);

private:
    // 核心处理函数
    cv::Mat preprocessImage(const cv::Mat& image);
    std::vector<cv::Mat> runInference(const cv::Mat& blob);
    AIResultVisualizer::AIResult postProcessResults(const std::vector<cv::Mat>& outputs, 
                                                   const cv::Size& originalSize);
    
    // 特定模型的后处理函数
    AIResultVisualizer::AIResult postProcessYOLO(const std::vector<cv::Mat>& outputs, 
                                                const cv::Size& originalSize);
    AIResultVisualizer::AIResult postProcessSSD(const std::vector<cv::Mat>& outputs, 
                                               const cv::Size& originalSize);
    AIResultVisualizer::AIResult postProcessClassification(const std::vector<cv::Mat>& outputs);
    AIResultVisualizer::AIResult postProcessPoseEstimation(const std::vector<cv::Mat>& outputs, 
                                                          const cv::Size& originalSize);
    AIResultVisualizer::AIResult postProcessFaceDetection(const std::vector<cv::Mat>& outputs, 
                                                         const cv::Size& originalSize);
    
    // 辅助函数
    void applyNMS(std::vector<cv::Rect>& boxes, std::vector<float>& confidences, 
                  std::vector<int>& classIds, std::vector<int>& indices);
    cv::Rect scaleRect(const cv::Rect& rect, const cv::Size& originalSize, const cv::Size& inputSize);
    cv::Point2f scalePoint(const cv::Point2f& point, const cv::Size& originalSize, const cv::Size& inputSize);

private:
    // DNN相关
    cv::dnn::Net m_net;                           // OpenCV DNN网络
    bool m_modelLoaded;                           // 模型是否已加载
    std::string m_modelPath;                      // 模型文件路径
    std::string m_configPath;                     // 配置文件路径（可选）
    
    // 预处理参数 
    AlgorithmImportDialog::BlobParameters m_blobParams;
    
    // 模型和后处理参数
    ModelType m_modelType;                        // 模型类型
    PostProcessParams m_postProcessParams;       // 后处理参数
    
    // 结果相关
    AIResultVisualizer::AIResult m_lastResults;  // 最后一次的结果
    std::unique_ptr<AIResultVisualizer> m_visualizer; // 可视化器
    
    // 性能统计
    double m_preprocessTime;                      // 预处理时间
    double m_inferenceTime;                       // 推理时间  
    double m_postprocessTime;                     // 后处理时间
};

#endif // AIMODELPROCESSOR_H