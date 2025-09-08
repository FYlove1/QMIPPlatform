#include "medianblur.h"

MedianBlur::MedianBlur() : m_kernelSize(5) {
}

cv::Mat MedianBlur::process(const cv::Mat& input) {
    cv::Mat output;
    cv::medianBlur(input, output, m_kernelSize);
    return output;
}

void MedianBlur::setParameters(const QVariantMap& params) {
    if (params.contains("kernelSize")) {
        m_kernelSize = params["kernelSize"].toInt();
        if (m_kernelSize % 2 == 0) {
            m_kernelSize += 1;
        }
        if (m_kernelSize < 3) m_kernelSize = 3;
        if (m_kernelSize > 31) m_kernelSize = 31;
    }
}

QVariantMap MedianBlur::getParameters() const {
    QVariantMap params;
    params["kernelSize"] = m_kernelSize;
    return params;
}

QString MedianBlur::getName() const {
    return "中值模糊";
}

QString MedianBlur::getDescription() const {
    return "使用中值滤波器对图像进行模糊处理，有效去除椒盐噪声。\n"
           "参数需求：\n"
           "- kernelSize (整数): 中值滤波器的核大小，必须为奇数，范围 3-31，默认值 5";
}

int MedianBlur::getId() const {
    return 5;
}

QList<ParameterMeta> MedianBlur::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta kernelSizeMeta;
    kernelSizeMeta.name = "kernelSize";
    kernelSizeMeta.displayName = "核大小";
    kernelSizeMeta.description = "中值滤波器的核大小（必须为奇数）";
    kernelSizeMeta.type = ParamType::Int;
    kernelSizeMeta.defaultValue = 5;
    kernelSizeMeta.minValue = 3;
    kernelSizeMeta.maxValue = 31;
    metaList.append(kernelSizeMeta);
    
    return metaList;
}

Algorithm* MedianBlur::clone() const {
    MedianBlur* copy = new MedianBlur();
    copy->m_kernelSize = this->m_kernelSize;
    return copy;
}