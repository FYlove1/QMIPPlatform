#include "sobeledgedetector.h"

SobelEdgeDetector::SobelEdgeDetector() 
    : m_kernelSize(3), m_scale(1.0), m_delta(0.0), m_direction(2) {
}

cv::Mat SobelEdgeDetector::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat gray, gradX, gradY, output;
    
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input;
    }
    
    if (m_direction == 0 || m_direction == 2) {
        cv::Sobel(gray, gradX, CV_16S, 1, 0, m_kernelSize, m_scale, m_delta);
        cv::convertScaleAbs(gradX, gradX);
    }
    
    if (m_direction == 1 || m_direction == 2) {
        cv::Sobel(gray, gradY, CV_16S, 0, 1, m_kernelSize, m_scale, m_delta);
        cv::convertScaleAbs(gradY, gradY);
    }
    
    if (m_direction == 0) {
        output = gradX;
    } else if (m_direction == 1) {
        output = gradY;
    } else {
        cv::addWeighted(gradX, 0.5, gradY, 0.5, 0, output);
    }
    
    if (input.channels() == 3) {
        cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);
    }
    
    return output;
}

void SobelEdgeDetector::setParameters(const QVariantMap& params) {
    if (params.contains("kernelSize")) {
        m_kernelSize = params["kernelSize"].toInt();
        if (m_kernelSize % 2 == 0) {
            m_kernelSize += 1;
        }
        if (m_kernelSize < 1) m_kernelSize = 1;
        if (m_kernelSize > 31) m_kernelSize = 31;
    }
    if (params.contains("scale")) {
        m_scale = params["scale"].toDouble();
    }
    if (params.contains("delta")) {
        m_delta = params["delta"].toDouble();
    }
    if (params.contains("direction")) {
        m_direction = params["direction"].toInt();
    }
}

QVariantMap SobelEdgeDetector::getParameters() const {
    QVariantMap params;
    params["kernelSize"] = m_kernelSize;
    params["scale"] = m_scale;
    params["delta"] = m_delta;
    params["direction"] = m_direction;
    return params;
}

QString SobelEdgeDetector::getName() const {
    return "Sobel边缘";
}

QString SobelEdgeDetector::getDescription() const {
    return "使用Sobel算子检测图像边缘。\n"
           "参数需求：\n"
           "- kernelSize (整数): Sobel核的大小，必须为奇数，范围 1-31，默认值 3\n"
           "- scale (浮点数): 可选的缩放因子，范围 0.1-10.0，默认值 1.0\n"
           "- delta (浮点数): 可选的增量值，范围 -255.0 到 255.0，默认值 0.0\n"
           "- direction (枚举): 梯度方向，0-X方向，1-Y方向，2-XY组合，默认值 2";
}

int SobelEdgeDetector::getId() const {
    return 8;
}

QList<ParameterMeta> SobelEdgeDetector::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta kernelSizeMeta;
    kernelSizeMeta.name = "kernelSize";
    kernelSizeMeta.displayName = "核大小";
    kernelSizeMeta.description = "Sobel核的大小";
    kernelSizeMeta.type = ParamType::Int;
    kernelSizeMeta.defaultValue = 3;
    kernelSizeMeta.minValue = 1;
    kernelSizeMeta.maxValue = 31;
    metaList.append(kernelSizeMeta);
    
    ParameterMeta scaleMeta;
    scaleMeta.name = "scale";
    scaleMeta.displayName = "缩放因子";
    scaleMeta.description = "可选的缩放因子";
    scaleMeta.type = ParamType::Double;
    scaleMeta.defaultValue = 1.0;
    scaleMeta.minValue = 0.1;
    scaleMeta.maxValue = 10.0;
    metaList.append(scaleMeta);
    
    ParameterMeta deltaMeta;
    deltaMeta.name = "delta";
    deltaMeta.displayName = "增量值";
    deltaMeta.description = "可选的增量值";
    deltaMeta.type = ParamType::Double;
    deltaMeta.defaultValue = 0.0;
    deltaMeta.minValue = -255.0;
    deltaMeta.maxValue = 255.0;
    metaList.append(deltaMeta);
    
    ParameterMeta directionMeta;
    directionMeta.name = "direction";
    directionMeta.displayName = "方向";
    directionMeta.description = "梯度方向";
    directionMeta.type = ParamType::Enum;
    directionMeta.defaultValue = 2;
    directionMeta.enumOptions = QStringList() << "X方向" << "Y方向" << "XY组合";
    metaList.append(directionMeta);
    
    return metaList;
}

Algorithm* SobelEdgeDetector::clone() const {
    SobelEdgeDetector* copy = new SobelEdgeDetector();
    copy->m_kernelSize = this->m_kernelSize;
    copy->m_scale = this->m_scale;
    copy->m_delta = this->m_delta;
    copy->m_direction = this->m_direction;
    return copy;
}