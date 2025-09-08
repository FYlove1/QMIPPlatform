#include "thresholdfilter.h"

ThresholdFilter::ThresholdFilter() : m_threshold(128), m_maxVal(255) {
}

cv::Mat ThresholdFilter::process(const cv::Mat& input) {
    cv::Mat gray, result;
    
    // 转换为灰度图
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // 应用二值化
    cv::threshold(gray, result, m_threshold, m_maxVal, cv::THRESH_BINARY);
    
    // 将结果转换回与输入相同的通道数
    if (input.channels() == 3) {
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }
    
    return result;
}

void ThresholdFilter::setParameters(const QVariantMap& params) {
    if (params.contains("threshold")) {
        bool ok;
        int value = params["threshold"].toInt(&ok);
        if (ok) {
            // 范围检查
            m_threshold = qBound(0, value, 255);
        }
        // 如果转换失败，保持当前值
    }
    
    if (params.contains("maxVal")) {
        bool ok;
        int value = params["maxVal"].toInt(&ok);
        if (ok) {
            // 范围检查
            m_maxVal = qBound(0, value, 255);
        }
        // 如果转换失败，保持当前值
    }
}

QVariantMap ThresholdFilter::getParameters() const {
    QVariantMap params;
    params["threshold"] = m_threshold;
    params["maxVal"] = m_maxVal;
    return params;
}

QString ThresholdFilter::getName() const {
    return "二值化";
}

QString ThresholdFilter::getDescription() const {
    return "将图像转换为黑白二值图像。\n"
           "参数需求：\n"
           "- threshold (整数): 二值化的阈值，范围 0-255，默认值 128\n"
           "- maxVal (整数): 二值化的最大值，范围 0-255，默认值 255";
}

int ThresholdFilter::getId() const {
    return 4; // ALGO_THRESHOLD
}

QList<ParameterMeta> ThresholdFilter::getParametersMeta() const {
    QList<ParameterMeta> meta;
    
    ParameterMeta thresholdMeta;
    thresholdMeta.name = "threshold";
    thresholdMeta.displayName = "阈值";
    thresholdMeta.description = "二值化的阈值";
    thresholdMeta.type = ParamType::Int;
    thresholdMeta.defaultValue = 128;
    thresholdMeta.minValue = 0;
    thresholdMeta.maxValue = 255;
    
    ParameterMeta maxValMeta;
    maxValMeta.name = "maxVal";
    maxValMeta.displayName = "最大值";
    maxValMeta.description = "二值化的最大值";
    maxValMeta.type = ParamType::Int;
    maxValMeta.defaultValue = 255;
    maxValMeta.minValue = 0;
    maxValMeta.maxValue = 255;
    
    meta.append(thresholdMeta);
    meta.append(maxValMeta);
    return meta;
}

Algorithm* ThresholdFilter::clone() const {
    return new ThresholdFilter(*this);
}