#include "hogpedestriandetector.h"

HOGPedestrianDetector::HOGPedestrianDetector() 
    : m_hitThreshold(0.0), m_scaleFactor(1.05), m_minNeighbors(2), m_showConfidence(true) {
    m_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
}

cv::Mat HOGPedestrianDetector::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat output = input.clone();
    
    // HOG检测需要足够大的图像
    cv::Mat resized;
    double scale = 1.0;
    if (input.cols < 64 || input.rows < 128) {
        scale = std::max(64.0 / input.cols, 128.0 / input.rows);
        cv::resize(input, resized, cv::Size(), scale, scale);
    } else {
        resized = input;
    }
    
    // 检测行人
    std::vector<cv::Rect> found;
    std::vector<double> weights;
    
    m_hog.detectMultiScale(resized, found, weights, 
        m_hitThreshold, 
        cv::Size(8, 8),  // winStride
        cv::Size(32, 32), // padding
        m_scaleFactor,
        m_minNeighbors);
    
    // 绘制检测结果
    int detectionCount = 0;
    for (size_t i = 0; i < found.size(); i++) {
        cv::Rect r = found[i];
        
        // 如果图像被缩放，需要调整矩形坐标
        if (scale != 1.0) {
            r.x /= scale;
            r.y /= scale;
            r.width /= scale;
            r.height /= scale;
        }
        
        // 绘制边界框
        cv::Scalar color = input.channels() == 3 ? cv::Scalar(0, 255, 0) : cv::Scalar(255);
        cv::rectangle(output, r, color, 2);
        
        // 添加标签
        char label[100];
        if (m_showConfidence && i < weights.size()) {
            sprintf(label, "Person %.2f", weights[i]);
        } else {
            sprintf(label, "Person %d", detectionCount + 1);
        }
        
        int baseline;
        cv::Size textSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        
        // 绘制标签背景
        if (input.channels() == 3) {
            cv::rectangle(output, 
                cv::Point(r.x, r.y - textSize.height - 4),
                cv::Point(r.x + textSize.width, r.y),
                cv::Scalar(0, 255, 0), -1);
            
            cv::putText(output, label, 
                cv::Point(r.x, r.y - 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        } else {
            cv::putText(output, label, 
                cv::Point(r.x, r.y - 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255), 1);
        }
        
        detectionCount++;
    }
    
    // 显示检测统计
    char stats[100];
    sprintf(stats, "Detected: %d pedestrians", detectionCount);
    cv::Scalar statsColor = input.channels() == 3 ? cv::Scalar(255, 255, 0) : cv::Scalar(255);
    cv::putText(output, stats, cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, statsColor, 2);
    
    return output;
}

void HOGPedestrianDetector::setParameters(const QVariantMap& params) {
    if (params.contains("hitThreshold")) {
        m_hitThreshold = params["hitThreshold"].toDouble();
        if (m_hitThreshold < 0) m_hitThreshold = 0;
        if (m_hitThreshold > 10) m_hitThreshold = 10;
    }
    if (params.contains("scaleFactor")) {
        m_scaleFactor = params["scaleFactor"].toDouble();
        if (m_scaleFactor < 1.01) m_scaleFactor = 1.01;
        if (m_scaleFactor > 2.0) m_scaleFactor = 2.0;
    }
    if (params.contains("minNeighbors")) {
        m_minNeighbors = params["minNeighbors"].toInt();
        if (m_minNeighbors < 0) m_minNeighbors = 0;
        if (m_minNeighbors > 10) m_minNeighbors = 10;
    }
    if (params.contains("showConfidence")) {
        m_showConfidence = params["showConfidence"].toBool();
    }
}

QVariantMap HOGPedestrianDetector::getParameters() const {
    QVariantMap params;
    params["hitThreshold"] = m_hitThreshold;
    params["scaleFactor"] = m_scaleFactor;
    params["minNeighbors"] = m_minNeighbors;
    params["showConfidence"] = m_showConfidence;
    return params;
}

QString HOGPedestrianDetector::getName() const {
    return "HOG行人检测";
}

QString HOGPedestrianDetector::getDescription() const {
    return "使用HOG特征和SVM分类器检测行人。\n"
           "返回检测到的行人边界框列表。\n"
           "参数说明：\n"
           "- hitThreshold: 检测阈值 (0-10)\n"
           "- scaleFactor: 图像金字塔缩放因子 (1.01-2.0)\n"
           "- minNeighbors: 最小邻居数 (0-10)\n"
           "- showConfidence: 显示置信度分数";
}

int HOGPedestrianDetector::getId() const {
    return 14;
}

QList<ParameterMeta> HOGPedestrianDetector::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta thresholdMeta;
    thresholdMeta.name = "hitThreshold";
    thresholdMeta.displayName = "检测阈值";
    thresholdMeta.description = "HOG检测阈值，越小越敏感";
    thresholdMeta.type = ParamType::Double;
    thresholdMeta.defaultValue = 0.0;
    thresholdMeta.minValue = 0.0;
    thresholdMeta.maxValue = 10.0;
    metaList.append(thresholdMeta);
    
    ParameterMeta scaleMeta;
    scaleMeta.name = "scaleFactor";
    scaleMeta.displayName = "缩放因子";
    scaleMeta.description = "图像金字塔缩放因子";
    scaleMeta.type = ParamType::Double;
    scaleMeta.defaultValue = 1.05;
    scaleMeta.minValue = 1.01;
    scaleMeta.maxValue = 2.0;
    metaList.append(scaleMeta);
    
    ParameterMeta neighborsMeta;
    neighborsMeta.name = "minNeighbors";
    neighborsMeta.displayName = "最小邻居数";
    neighborsMeta.description = "合并检测窗口的最小邻居数";
    neighborsMeta.type = ParamType::Int;
    neighborsMeta.defaultValue = 2;
    neighborsMeta.minValue = 0;
    neighborsMeta.maxValue = 10;
    metaList.append(neighborsMeta);
    
    ParameterMeta confidenceMeta;
    confidenceMeta.name = "showConfidence";
    confidenceMeta.displayName = "显示置信度";
    confidenceMeta.description = "在标签中显示置信度分数";
    confidenceMeta.type = ParamType::Bool;
    confidenceMeta.defaultValue = true;
    metaList.append(confidenceMeta);
    
    return metaList;
}

Algorithm* HOGPedestrianDetector::clone() const {
    HOGPedestrianDetector* copy = new HOGPedestrianDetector();
    copy->m_hitThreshold = this->m_hitThreshold;
    copy->m_scaleFactor = this->m_scaleFactor;
    copy->m_minNeighbors = this->m_minNeighbors;
    copy->m_showConfidence = this->m_showConfidence;
    return copy;
}