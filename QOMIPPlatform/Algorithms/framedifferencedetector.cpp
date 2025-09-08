#include "framedifferencedetector.h"

FrameDifferenceDetector::FrameDifferenceDetector() 
    : m_threshold(30), m_dilateSize(3), m_showMotionOnly(false) {
}

cv::Mat FrameDifferenceDetector::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // 第一帧初始化
    if (m_previousFrame.empty()) {
        m_previousFrame = gray.clone();
        return input;  // 第一帧返回原图
    }
    
    // 计算帧差
    cv::Mat diff;
    cv::absdiff(m_previousFrame, gray, diff);
    
    // 二值化
    cv::Mat mask;
    cv::threshold(diff, mask, m_threshold, 255, cv::THRESH_BINARY);
    
    // 形态学处理去除噪声
    if (m_dilateSize > 0) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, 
            cv::Size(m_dilateSize*2+1, m_dilateSize*2+1));
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
    }
    
    // 更新前一帧
    m_previousFrame = gray.clone();
    
    cv::Mat output;
    if (m_showMotionOnly) {
        // 只返回掩膜（转换为3通道以便显示）
        if (input.channels() == 3) {
            cv::cvtColor(mask, output, cv::COLOR_GRAY2BGR);
        } else {
            output = mask;
        }
    } else {
        // 在原图上高亮显示运动区域
        output = input.clone();
        if (input.channels() == 3) {
            // 运动区域用绿色轮廓标记
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::drawContours(output, contours, -1, cv::Scalar(0, 255, 0), 2);
            
            // 添加运动状态文字
            if (!contours.empty()) {
                cv::putText(output, "Motion Detected", cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
            }
        } else {
            // 灰度图直接叠加
            cv::add(output, mask * 0.5, output);
        }
    }
    
    return output;
}

void FrameDifferenceDetector::setParameters(const QVariantMap& params) {
    if (params.contains("threshold")) {
        m_threshold = params["threshold"].toInt();
        if (m_threshold < 1) m_threshold = 1;
        if (m_threshold > 255) m_threshold = 255;
    }
    if (params.contains("dilateSize")) {
        m_dilateSize = params["dilateSize"].toInt();
        if (m_dilateSize < 0) m_dilateSize = 0;
        if (m_dilateSize > 10) m_dilateSize = 10;
    }
    if (params.contains("showMotionOnly")) {
        m_showMotionOnly = params["showMotionOnly"].toBool();
    }
    if (params.contains("reset") && params["reset"].toBool()) {
        m_previousFrame = cv::Mat();  // 重置前一帧
    }
}

QVariantMap FrameDifferenceDetector::getParameters() const {
    QVariantMap params;
    params["threshold"] = m_threshold;
    params["dilateSize"] = m_dilateSize;
    params["showMotionOnly"] = m_showMotionOnly;
    params["reset"] = false;
    return params;
}

QString FrameDifferenceDetector::getName() const {
    return "帧差运动检测";
}

QString FrameDifferenceDetector::getDescription() const {
    return "使用帧差法检测运动区域。\n"
           "返回二值掩膜，白色表示运动区域，黑色表示静止。\n"
           "参数说明：\n"
           "- threshold: 差异阈值 (1-255)\n"
           "- dilateSize: 膨胀大小，用于连接运动区域 (0-10)\n"
           "- showMotionOnly: 是否只显示掩膜\n"
           "- reset: 重置前一帧缓存";
}

int FrameDifferenceDetector::getId() const {
    return 11;
}

QList<ParameterMeta> FrameDifferenceDetector::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta thresholdMeta;
    thresholdMeta.name = "threshold";
    thresholdMeta.displayName = "差异阈值";
    thresholdMeta.description = "帧差二值化阈值";
    thresholdMeta.type = ParamType::Int;
    thresholdMeta.defaultValue = 30;
    thresholdMeta.minValue = 1;
    thresholdMeta.maxValue = 255;
    metaList.append(thresholdMeta);
    
    ParameterMeta dilateMeta;
    dilateMeta.name = "dilateSize";
    dilateMeta.displayName = "膨胀大小";
    dilateMeta.description = "形态学膨胀核大小";
    dilateMeta.type = ParamType::Int;
    dilateMeta.defaultValue = 3;
    dilateMeta.minValue = 0;
    dilateMeta.maxValue = 10;
    metaList.append(dilateMeta);
    
    ParameterMeta motionOnlyMeta;
    motionOnlyMeta.name = "showMotionOnly";
    motionOnlyMeta.displayName = "仅显示运动掩膜";
    motionOnlyMeta.description = "只显示二值掩膜";
    motionOnlyMeta.type = ParamType::Bool;
    motionOnlyMeta.defaultValue = false;
    metaList.append(motionOnlyMeta);
    
    ParameterMeta resetMeta;
    resetMeta.name = "reset";
    resetMeta.displayName = "重置";
    resetMeta.description = "重置前一帧缓存";
    resetMeta.type = ParamType::Bool;
    resetMeta.defaultValue = false;
    metaList.append(resetMeta);
    
    return metaList;
}

Algorithm* FrameDifferenceDetector::clone() const {
    FrameDifferenceDetector* copy = new FrameDifferenceDetector();
    copy->m_threshold = this->m_threshold;
    copy->m_dilateSize = this->m_dilateSize;
    copy->m_showMotionOnly = this->m_showMotionOnly;
    copy->m_previousFrame = this->m_previousFrame.clone();
    return copy;
}