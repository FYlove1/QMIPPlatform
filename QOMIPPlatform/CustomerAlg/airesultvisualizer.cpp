#include "airesultvisualizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

AIResultVisualizer::AIResultVisualizer()
    : m_fontScale(0.6)
    , m_lineThickness(2)
    , m_showConfidence(true)
    , m_confidenceThreshold(0.0f)
{
    initDefaultColors();
}

AIResultVisualizer::~AIResultVisualizer()
{
}

void AIResultVisualizer::initDefaultColors()
{
    // 默认检测框颜色 (BGR格式)
    m_boxColors = {
        cv::Scalar(0, 255, 0),      // 绿色
        cv::Scalar(0, 0, 255),      // 红色
        cv::Scalar(255, 0, 0),      // 蓝色
        cv::Scalar(0, 255, 255),    // 黄色
        cv::Scalar(255, 0, 255),    // 紫色
        cv::Scalar(255, 255, 0),    // 青色
        cv::Scalar(128, 0, 128),    // 紫罗兰
        cv::Scalar(255, 165, 0),    // 橙色
        cv::Scalar(0, 128, 255),    // 橙红色
        cv::Scalar(128, 128, 0)     // 橄榄色
    };
    
    // 默认关键点颜色
    m_keyPointColors = {
        cv::Scalar(0, 255, 0),      // 绿色
        cv::Scalar(0, 0, 255),      // 红色  
        cv::Scalar(255, 0, 0),      // 蓝色
        cv::Scalar(0, 255, 255),    // 黄色
        cv::Scalar(255, 0, 255),    // 紫色
        cv::Scalar(255, 255, 0),    // 青色
        cv::Scalar(255, 128, 0),    // 橙色
        cv::Scalar(128, 255, 0),    // 黄绿色
        cv::Scalar(0, 128, 255),    // 天蓝色
        cv::Scalar(128, 0, 255)     // 紫红色
    };
}

cv::Mat AIResultVisualizer::visualizeResults(const cv::Mat& inputImage, const AIResult& results)
{
    if (inputImage.empty()) {
        return cv::Mat();
    }
    
    // 复制输入图像
    cv::Mat outputImage = inputImage.clone();
    
    // 依次绘制各种结果
    drawDetectionBoxes(outputImage, results.detections);
    drawKeyPoints(outputImage, results.keyPointGroups);
    drawClassifications(outputImage, results.classifications);
    drawInfoPanel(outputImage, results);
    
    return outputImage;
}

void AIResultVisualizer::drawDetectionBoxes(cv::Mat& image, const std::vector<DetectionBox>& detections)
{
    std::cout << "DEBUG VISUALIZER: Drawing " << detections.size() << " detection boxes" << std::endl;
    std::cout << "DEBUG VISUALIZER: Confidence threshold: " << m_confidenceThreshold << std::endl;
    
    int index = 1; // 从1开始编号
    
    for (const auto& detection : detections) {
        std::cout << "DEBUG VISUALIZER: Processing detection " << index 
                 << " - Confidence: " << detection.confidence 
                 << ", Threshold: " << m_confidenceThreshold << std::endl;
                 
        if (detection.confidence < m_confidenceThreshold) {
            std::cout << "DEBUG VISUALIZER: Detection " << index << " filtered by confidence" << std::endl;
            continue;
        }
        
        cv::Scalar color = getBoxColor(index - 1);
        
        std::cout << "DEBUG VISUALIZER: Drawing box for detection " << index 
                 << " at (" << detection.rect.x << ", " << detection.rect.y 
                 << ", " << detection.rect.width << ", " << detection.rect.height 
                 << ") with color (" << color[0] << ", " << color[1] << ", " << color[2] << ")" << std::endl;
        
        // 绘制检测框
        cv::rectangle(image, detection.rect, color, m_lineThickness);
        
        // 准备标签文本
        std::string labelText = std::to_string(index);
        
        // 计算标签位置和大小
        cv::Size textSize = calculateTextSize(labelText);
        cv::Point labelPos(detection.rect.x, detection.rect.y - 5);
        
        // 确保标签不超出图像边界
        if (labelPos.y < textSize.height) {
            labelPos.y = detection.rect.y + textSize.height + 5;
        }
        
        // 绘制标签背景
        cv::Rect labelBg(labelPos.x - 2, labelPos.y - textSize.height - 2, 
                         textSize.width + 4, textSize.height + 4);
        cv::rectangle(image, labelBg, color, -1);
        
        // 绘制标签文本
        cv::putText(image, labelText, labelPos, cv::FONT_HERSHEY_SIMPLEX, 
                   m_fontScale, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        
        std::cout << "DEBUG VISUALIZER: Successfully drew detection box " << index << std::endl;
        
        index++;
    }
    
    std::cout << "DEBUG VISUALIZER: Finished drawing detection boxes" << std::endl;
}

void AIResultVisualizer::drawKeyPoints(cv::Mat& image, const std::vector<KeyPointGroup>& keyPointGroups)
{
    for (const auto& group : keyPointGroups) {
        // 绘制关键点
        for (size_t i = 0; i < group.keyPoints.size(); ++i) {
            const auto& kp = group.keyPoints[i];
            if (!kp.visible || kp.confidence < m_confidenceThreshold) {
                continue;
            }
            
            cv::Scalar color = getKeyPointColor(i);
            
            // 绘制关键点圆圈
            cv::circle(image, kp.point, 4, color, -1);
            cv::circle(image, kp.point, 4, cv::Scalar(0, 0, 0), 1);
            
            // 如果有名称，绘制标签
            if (!kp.name.empty()) {
                cv::Point textPos(kp.point.x + 5, kp.point.y - 5);
                drawText(image, kp.name, textPos, color, false);
            }
        }
        
        // 如果是人体姿态，绘制骨架连接
        if (group.groupName == "person" || group.groupName == "pose") {
            drawPoseSkeleton(image, group.keyPoints);
        }
    }
}

void AIResultVisualizer::drawClassifications(cv::Mat& image, const std::vector<Classification>& classifications)
{
    if (classifications.empty()) return;
    
    int yOffset = 30;
    for (const auto& cls : classifications) {
        if (cls.confidence < m_confidenceThreshold) {
            continue;
        }
        
        std::string text = cls.className;
        if (m_showConfidence) {
            text += " (" + formatConfidence(cls.confidence) + ")";
        }
        
        drawText(image, text, cv::Point(10, yOffset), cv::Scalar(255, 255, 255));
        yOffset += 25;
    }
}

void AIResultVisualizer::drawInfoPanel(cv::Mat& image, const AIResult& results)
{
    std::vector<std::string> infoTexts;
    
    // 收集所有检测结果信息
    int index = 1;
    for (const auto& detection : results.detections) {
        if (detection.confidence < m_confidenceThreshold) {
            continue;
        }
        
        std::string info = std::to_string(index) + ". " + detection.className;
        if (m_showConfidence) {
            info += " (" + formatConfidence(detection.confidence) + ")";
        }
        infoTexts.push_back(info);
        index++;
    }
    
    // 如果没有检测结果但有分类结果
    if (infoTexts.empty() && !results.classifications.empty()) {
        for (const auto& cls : results.classifications) {
            if (cls.confidence < m_confidenceThreshold) {
                continue;
            }
            
            std::string info = cls.className;
            if (m_showConfidence) {
                info += " (" + formatConfidence(cls.confidence) + ")";
            }
            infoTexts.push_back(info);
        }
    }
    
    // 添加模型类型信息
    if (!results.modelType.empty()) {
        infoTexts.insert(infoTexts.begin(), "Model: " + results.modelType);
    }
    
    if (infoTexts.empty()) {
        return;
    }
    
    // 计算信息面板大小
    int maxTextWidth = 0;
    int textHeight = 20;
    
    for (const auto& text : infoTexts) {
        cv::Size size = calculateTextSize(text);
        maxTextWidth = std::max(maxTextWidth, size.width);
    }
    
    // 绘制半透明背景
    int panelWidth = maxTextWidth + 20;
    int panelHeight = infoTexts.size() * textHeight + 20;
    
    cv::Rect panelRect(10, 10, panelWidth, panelHeight);
    cv::Mat roi = image(panelRect);
    cv::Mat overlay = roi.clone();
    cv::rectangle(overlay, cv::Rect(0, 0, panelWidth, panelHeight), 
                 cv::Scalar(0, 0, 0), -1);
    cv::addWeighted(roi, 0.7, overlay, 0.3, 0, roi);
    
    // 绘制文本
    int yPos = 30;
    for (const auto& text : infoTexts) {
        cv::putText(image, text, cv::Point(20, yPos), 
                   cv::FONT_HERSHEY_SIMPLEX, m_fontScale, 
                   cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        yPos += textHeight;
    }
}

void AIResultVisualizer::drawPoseSkeleton(cv::Mat& image, const std::vector<KeyPoint>& keyPoints)
{
    // COCO人体姿态的17个关键点连接关系
    std::vector<std::pair<int, int>> skeleton = {
        {0, 1}, {0, 2}, {1, 3}, {2, 4},     // 头部
        {5, 6}, {5, 7}, {7, 9}, {6, 8},     // 手臂
        {8, 10}, {5, 11}, {6, 12}, {11, 12}, // 躯干
        {11, 13}, {13, 15}, {12, 14}, {14, 16} // 腿部
    };
    
    for (const auto& connection : skeleton) {
        int idx1 = connection.first;
        int idx2 = connection.second;
        
        if (idx1 < keyPoints.size() && idx2 < keyPoints.size()) {
            const auto& kp1 = keyPoints[idx1];
            const auto& kp2 = keyPoints[idx2];
            
            if (kp1.visible && kp2.visible && 
                kp1.confidence > m_confidenceThreshold && 
                kp2.confidence > m_confidenceThreshold) {
                
                cv::line(image, kp1.point, kp2.point, 
                        cv::Scalar(0, 255, 255), m_lineThickness - 1);
            }
        }
    }
}

cv::Scalar AIResultVisualizer::getBoxColor(int index)
{
    return m_boxColors[index % m_boxColors.size()];
}

cv::Scalar AIResultVisualizer::getKeyPointColor(int index)
{
    return m_keyPointColors[index % m_keyPointColors.size()];
}

std::string AIResultVisualizer::formatConfidence(float confidence)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << (confidence * 100) << "%";
    return oss.str();
}

cv::Size AIResultVisualizer::calculateTextSize(const std::string& text)
{
    int baseline = 0;
    return cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, m_fontScale, 1, &baseline);
}

void AIResultVisualizer::drawText(cv::Mat& image, const std::string& text, 
                                 const cv::Point& position, const cv::Scalar& color, bool withBackground)
{
    if (withBackground) {
        cv::Size textSize = calculateTextSize(text);
        cv::Rect bgRect(position.x - 2, position.y - textSize.height - 2,
                       textSize.width + 4, textSize.height + 4);
        cv::rectangle(image, bgRect, cv::Scalar(0, 0, 0), -1);
    }
    
    cv::putText(image, text, position, cv::FONT_HERSHEY_SIMPLEX, 
               m_fontScale, color, 1, cv::LINE_AA);
}

// 配置函数实现
void AIResultVisualizer::setBoxColors(const std::vector<cv::Scalar>& colors)
{
    if (!colors.empty()) {
        m_boxColors = colors;
    }
}

void AIResultVisualizer::setKeyPointColors(const std::vector<cv::Scalar>& colors)
{
    if (!colors.empty()) {
        m_keyPointColors = colors;
    }
}

void AIResultVisualizer::setFontScale(double scale)
{
    m_fontScale = std::max(0.3, scale);
}

void AIResultVisualizer::setLineThickness(int thickness)
{
    m_lineThickness = std::max(1, thickness);
}

void AIResultVisualizer::setShowConfidence(bool show)
{
    m_showConfidence = show;
}

void AIResultVisualizer::setConfidenceThreshold(float threshold)
{
    m_confidenceThreshold = std::max(0.0f, std::min(1.0f, threshold));
}