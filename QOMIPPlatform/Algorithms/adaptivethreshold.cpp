#include "adaptivethreshold.h"

AdaptiveThreshold::AdaptiveThreshold() 
    : m_blockSize(11), m_C(2), m_method(0), m_invert(false) {
}

cv::Mat AdaptiveThreshold::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat gray, output;
    
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input;
    }
    
    int adaptiveMethod = (m_method == 0) ? cv::ADAPTIVE_THRESH_MEAN_C : cv::ADAPTIVE_THRESH_GAUSSIAN_C;
    int thresholdType = m_invert ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
    
    cv::adaptiveThreshold(gray, output, 255, adaptiveMethod, thresholdType, m_blockSize, m_C);
    
    if (input.channels() == 3) {
        cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);
    }
    
    return output;
}

void AdaptiveThreshold::setParameters(const QVariantMap& params) {
    if (params.contains("blockSize")) {
        m_blockSize = params["blockSize"].toInt();
        if (m_blockSize % 2 == 0) {
            m_blockSize += 1;
        }
        if (m_blockSize < 3) m_blockSize = 3;
        if (m_blockSize > 99) m_blockSize = 99;
    }
    if (params.contains("C")) {
        m_C = params["C"].toDouble();
    }
    if (params.contains("method")) {
        m_method = params["method"].toInt();
    }
    if (params.contains("invert")) {
        m_invert = params["invert"].toBool();
    }
}

QVariantMap AdaptiveThreshold::getParameters() const {
    QVariantMap params;
    params["blockSize"] = m_blockSize;
    params["C"] = m_C;
    params["method"] = m_method;
    params["invert"] = m_invert;
    return params;
}

QString AdaptiveThreshold::getName() const {
    return "自适应二值化";
}

QString AdaptiveThreshold::getDescription() const {
    return "根据图像局部区域自适应计算阈值进行二值化。\n"
           "参数需求：\n"
           "- blockSize (整数): 计算阈值的邻域大小，必须为奇数，范围 3-99，默认值 11\n"
           "- C (浮点数): 从平均值或加权平均值中减去的常数，范围 -50.0 到 50.0，默认值 2.0\n"
           "- method (枚举): 自适应方法，0-均值，1-高斯加权，默认值 0\n"
           "- invert (布尔): 是否反转二值化结果，默认值 false";
}

int AdaptiveThreshold::getId() const {
    return 7;
}

QList<ParameterMeta> AdaptiveThreshold::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta blockSizeMeta;
    blockSizeMeta.name = "blockSize";
    blockSizeMeta.displayName = "块大小";
    blockSizeMeta.description = "计算阈值的邻域大小（必须为奇数）";
    blockSizeMeta.type = ParamType::Int;
    blockSizeMeta.defaultValue = 11;
    blockSizeMeta.minValue = 3;
    blockSizeMeta.maxValue = 99;
    metaList.append(blockSizeMeta);
    
    ParameterMeta cMeta;
    cMeta.name = "C";
    cMeta.displayName = "常数C";
    cMeta.description = "从平均值或加权平均值中减去的常数";
    cMeta.type = ParamType::Double;
    cMeta.defaultValue = 2.0;
    cMeta.minValue = -50.0;
    cMeta.maxValue = 50.0;
    metaList.append(cMeta);
    
    ParameterMeta methodMeta;
    methodMeta.name = "method";
    methodMeta.displayName = "方法";
    methodMeta.description = "自适应方法";
    methodMeta.type = ParamType::Enum;
    methodMeta.defaultValue = 0;
    methodMeta.enumOptions = QStringList() << "均值" << "高斯加权";
    metaList.append(methodMeta);
    
    ParameterMeta invertMeta;
    invertMeta.name = "invert";
    invertMeta.displayName = "反转";
    invertMeta.description = "是否反转二值化结果";
    invertMeta.type = ParamType::Bool;
    invertMeta.defaultValue = false;
    metaList.append(invertMeta);
    
    return metaList;
}

Algorithm* AdaptiveThreshold::clone() const {
    AdaptiveThreshold* copy = new AdaptiveThreshold();
    copy->m_blockSize = this->m_blockSize;
    copy->m_C = this->m_C;
    copy->m_method = this->m_method;
    copy->m_invert = this->m_invert;
    return copy;
}