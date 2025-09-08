#include "blurfilter.h"

BlurFilter::BlurFilter() : m_kernelSize(15) {
}

cv::Mat BlurFilter::process(const cv::Mat& input) {
    cv::Mat result;
    
    // 确保内核大小是奇数
    int kernelSize = (m_kernelSize % 2 == 0) ? m_kernelSize + 1 : m_kernelSize;
    
    // 应用高斯模糊
    cv::GaussianBlur(input, result, cv::Size(kernelSize, kernelSize), 0);
    
    return result;
}

void BlurFilter::setParameters(const QVariantMap& params) {
    if (params.contains("kernelSize")) {
        // 强制转换为整数
        bool ok;
        int kernelSize = params["kernelSize"].toInt(&ok);
        
        if (!ok) {
            // 转换失败，使用默认值
            m_kernelSize = 15;
            return;
        }
        
        // 范围检查
        if (kernelSize < 3) {
            kernelSize = 3;
        } else if (kernelSize > 99) {
            kernelSize = 99;
        }
        
        // 确保是奇数
        if (kernelSize % 2 == 0) {
            kernelSize += 1;
            // 再次检查上限
            if (kernelSize > 99) {
                kernelSize = 99;
            }
        }
        
        m_kernelSize = kernelSize;
    }
}

QVariantMap BlurFilter::getParameters() const {
    QVariantMap params;
    params["kernelSize"] = m_kernelSize;
    return params;
}

QString BlurFilter::getName() const {
    return "高斯模糊";
}

QString BlurFilter::getDescription() const {
    return "使用高斯算法对图像进行平滑处理。\n"
           "参数需求：\n"
           "- kernelSize (整数): 高斯模糊的内核大小，必须为奇数，范围 3-99，默认值 15";
}

int BlurFilter::getId() const {
    return 2; // ALGO_BLUR
}

QList<ParameterMeta> BlurFilter::getParametersMeta() const {
    QList<ParameterMeta> meta;
    
    ParameterMeta kernelSizeMeta;
    kernelSizeMeta.name = "kernelSize";
    kernelSizeMeta.displayName = "内核大小";
    kernelSizeMeta.description = "高斯模糊的内核大小，必须为奇数";
    kernelSizeMeta.type = ParamType::Int;
    kernelSizeMeta.defaultValue = 15;
    kernelSizeMeta.minValue = 3;
    kernelSizeMeta.maxValue = 99;
    
    meta.append(kernelSizeMeta);
    return meta;
}

Algorithm* BlurFilter::clone() const {
    return new BlurFilter(*this);
}