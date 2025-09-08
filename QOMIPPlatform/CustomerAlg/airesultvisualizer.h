#ifndef AIRESULTVISUALIZER_H
#define AIRESULTVISUALIZER_H

#include "opencv2/opencv.hpp"
#include <vector>
#include <string>

class AIResultVisualizer
{
public:
    // 检测框结构
    struct DetectionBox {
        cv::Rect rect;              // 边界框
        std::string className;      // 类别名称
        float confidence;           // 置信度
        int classId;               // 类别ID
        
        DetectionBox() : confidence(0.0f), classId(-1) {}
        DetectionBox(const cv::Rect& r, const std::string& name, float conf, int id = -1)
            : rect(r), className(name), confidence(conf), classId(id) {}
    };
    
    // 关键点结构
    struct KeyPoint {
        cv::Point2f point;          // 关键点坐标
        std::string name;           // 关键点名称
        float confidence;           // 置信度
        bool visible;               // 是否可见
        
        KeyPoint() : confidence(0.0f), visible(true) {}
        KeyPoint(const cv::Point2f& pt, const std::string& n = "", float conf = 1.0f, bool vis = true)
            : point(pt), name(n), confidence(conf), visible(vis) {}
    };
    
    // 关键点组（用于人体姿态、人脸等）
    struct KeyPointGroup {
        std::vector<KeyPoint> keyPoints;
        std::string groupName;      // 组名（如"person", "face"等）
        cv::Rect boundingBox;       // 包围框（可选）
        
        KeyPointGroup(const std::string& name = "") : groupName(name) {}
    };
    
    // 分类结果结构
    struct Classification {
        std::string className;      // 类别名称
        float confidence;           // 置信度
        int classId;               // 类别ID
        
        Classification() : confidence(0.0f), classId(-1) {}
        Classification(const std::string& name, float conf, int id = -1)
            : className(name), confidence(conf), classId(id) {}
    };
    
    // 综合结果结构
    struct AIResult {
        std::vector<DetectionBox> detections;       // 检测框
        std::vector<KeyPointGroup> keyPointGroups; // 关键点组
        std::vector<Classification> classifications; // 分类结果
        std::string modelType;                      // 模型类型（用于显示）
        
        void clear() {
            detections.clear();
            keyPointGroups.clear();
            classifications.clear();
            modelType.clear();
        }
    };

public:
    AIResultVisualizer();
    ~AIResultVisualizer();
    
    // 主要可视化函数
    cv::Mat visualizeResults(const cv::Mat& inputImage, const AIResult& results);
    
    // 单独的绘制函数
    void drawDetectionBoxes(cv::Mat& image, const std::vector<DetectionBox>& detections);
    void drawKeyPoints(cv::Mat& image, const std::vector<KeyPointGroup>& keyPointGroups);
    void drawClassifications(cv::Mat& image, const std::vector<Classification>& classifications);
    void drawInfoPanel(cv::Mat& image, const AIResult& results);
    
    // 配置函数
    void setBoxColors(const std::vector<cv::Scalar>& colors);
    void setKeyPointColors(const std::vector<cv::Scalar>& colors);
    void setFontScale(double scale);
    void setLineThickness(int thickness);
    void setShowConfidence(bool show);
    void setConfidenceThreshold(float threshold);

private:
    // 绘制参数
    std::vector<cv::Scalar> m_boxColors;        // 检测框颜色
    std::vector<cv::Scalar> m_keyPointColors;   // 关键点颜色
    double m_fontScale;                         // 字体缩放
    int m_lineThickness;                        // 线条粗细
    bool m_showConfidence;                      // 是否显示置信度
    float m_confidenceThreshold;                // 置信度阈值
    
    // 辅助函数
    cv::Scalar getBoxColor(int index);
    cv::Scalar getKeyPointColor(int index);
    std::string formatConfidence(float confidence);
    cv::Size calculateTextSize(const std::string& text);
    void drawText(cv::Mat& image, const std::string& text, const cv::Point& position, 
                  const cv::Scalar& color, bool withBackground = true);
    
    // 默认骨架连接（用于人体姿态）
    void drawPoseSkeleton(cv::Mat& image, const std::vector<KeyPoint>& keyPoints);
    
    // 常用颜色
    void initDefaultColors();
};

#endif // AIRESULTVISUALIZER_H