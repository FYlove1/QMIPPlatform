#include "cannyedgedetector.h"

CannyEdgeDetector::CannyEdgeDetector() : m_threshold1(50), m_threshold2(150) {
}

cv::Mat CannyEdgeDetector::process(const cv::Mat& input) {
    cv::Mat gray, edges, result;
    
    // 转换为灰度图
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // 应用Canny边缘检测
    cv::Canny(gray, edges, m_threshold1, m_threshold2);
    
    // 将结果转换回与输入相同的通道数
    if (input.channels() == 3) {
        cv::cvtColor(edges, result, cv::COLOR_GRAY2BGR);
    } else {
        result = edges;
    }
    
    return result;
}

void CannyEdgeDetector::setParameters(const QVariantMap& params) {
    int threshold1 = m_threshold1;
    int threshold2 = m_threshold2;
    
    if (params.contains("threshold1")) {
        bool ok;
        int value = params["threshold1"].toInt(&ok);
        if (ok) {
            // 范围检查
            threshold1 = qBound(0, value, 255);
        }
    }
    
    if (params.contains("threshold2")) {
        bool ok;
        int value = params["threshold2"].toInt(&ok);
        if (ok) {
            // 范围检查
            threshold2 = qBound(0, value, 255);
        }
    }
    
    // 确保threshold2 >= threshold1
    if (threshold2 < threshold1) {
        std::swap(threshold1, threshold2);
    }
    
    m_threshold1 = threshold1;
    m_threshold2 = threshold2;
}

QVariantMap CannyEdgeDetector::getParameters() const {
    QVariantMap params;
    params["threshold1"] = m_threshold1;
    params["threshold2"] = m_threshold2;
    return params;
}

QString CannyEdgeDetector::getName() const {
    return "边缘检测";
}

QString CannyEdgeDetector::getDescription() const {
    return "使用Canny算法检测图像边缘。\n"
           "参数需求：\n"
           "- threshold1 (整数): 低阈值，范围 0-255，默认值 50\n"
           "- threshold2 (整数): 高阈值，范围 0-255，默认值 150，必须大于低阈值";
}

int CannyEdgeDetector::getId() const {
    return 3; // ALGO_CANNY
}

QList<ParameterMeta> CannyEdgeDetector::getParametersMeta() const {
    QList<ParameterMeta> meta;
    
    ParameterMeta threshold1Meta;
    threshold1Meta.name = "threshold1";
    threshold1Meta.displayName = "低阈值";
    threshold1Meta.description = "Canny边缘检测的低阈值";
    threshold1Meta.type = ParamType::Int;
    threshold1Meta.defaultValue = 50;
    threshold1Meta.minValue = 0;
    threshold1Meta.maxValue = 255;
    
    ParameterMeta threshold2Meta;
    threshold2Meta.name = "threshold2";
    threshold2Meta.displayName = "高阈值";
    threshold2Meta.description = "Canny边缘检测的高阈值";
    threshold2Meta.type = ParamType::Int;
    threshold2Meta.defaultValue = 150;
    threshold2Meta.minValue = 0;
    threshold2Meta.maxValue = 255;
    
    meta.append(threshold1Meta);
    meta.append(threshold2Meta);
    return meta;
}

Algorithm* CannyEdgeDetector::clone() const {
    return new CannyEdgeDetector(*this);
}