#include "hsvcolorextraction.h"

HSVColorExtraction::HSVColorExtraction()
    : m_hMin(0), m_hMax(15),  // 默认提取红色范围
      m_sMin(100), m_sMax(255),
      m_vMin(100), m_vMax(255),
      m_showMask(false) {
}

cv::Mat HSVColorExtraction::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat hsv, mask, output;
    
    // 转换为HSV颜色空间
    cv::cvtColor(input, hsv, cv::COLOR_BGR2HSV);
    
    // 检查是否需要处理H通道环绕情况（例如红色在H通道两端）
    if (m_hMin <= m_hMax) {
        // 正常情况，使用单个掩码
        cv::Scalar lowerBound(m_hMin, m_sMin, m_vMin);
        cv::Scalar upperBound(m_hMax, m_sMax, m_vMax);
        cv::inRange(hsv, lowerBound, upperBound, mask);
    } else {
        // H通道环绕情况（例如红色跨越180/0边界）
        cv::Mat mask1, mask2;
        cv::Scalar lowerBound1(0, m_sMin, m_vMin);
        cv::Scalar upperBound1(m_hMax, m_sMax, m_vMax);
        cv::Scalar lowerBound2(m_hMin, m_sMin, m_vMin);
        cv::Scalar upperBound2(180, m_sMax, m_vMax);
        
        cv::inRange(hsv, lowerBound1, upperBound1, mask1);
        cv::inRange(hsv, lowerBound2, upperBound2, mask2);
        cv::bitwise_or(mask1, mask2, mask);
    }
    
    if (m_showMask) {
        // 显示二值掩码
        cv::cvtColor(mask, output, cv::COLOR_GRAY2BGR);
    } else {
        // 应用掩码提取颜色
        output = cv::Mat::zeros(input.size(), input.type());
        input.copyTo(output, mask);
    }
    
    return output;
}

void HSVColorExtraction::setParameters(const QVariantMap& params) {
    // 设置HSV范围
    m_hMin = params.value("hMin", m_hMin).toInt();
    m_hMax = params.value("hMax", m_hMax).toInt();
    m_sMin = params.value("sMin", m_sMin).toInt();
    m_sMax = params.value("sMax", m_sMax).toInt();
    m_vMin = params.value("vMin", m_vMin).toInt();
    m_vMax = params.value("vMax", m_vMax).toInt();
    
    // 控制是否显示掩码
    m_showMask = params.value("showMask", m_showMask).toBool();
    
    // 确保值在有效范围内
    m_hMin = qBound(0, m_hMin, 180);
    m_hMax = qBound(0, m_hMax, 180);
    m_sMin = qBound(0, m_sMin, 255);
    m_sMax = qBound(0, m_sMax, 255);
    m_vMin = qBound(0, m_vMin, 255);
    m_vMax = qBound(0, m_vMax, 255);
}

QVariantMap HSVColorExtraction::getParameters() const {
    QVariantMap params;
    params["hMin"] = m_hMin;
    params["hMax"] = m_hMax;
    params["sMin"] = m_sMin;
    params["sMax"] = m_sMax;
    params["vMin"] = m_vMin;
    params["vMax"] = m_vMax;
    params["showMask"] = m_showMask;
    return params;
}

QString HSVColorExtraction::getName() const {
    return "HSV颜色提取";
}

QString HSVColorExtraction::getDescription() const {
    return "在HSV颜色空间中提取指定范围内的颜色。\n"
           "参数需求：\n"
           "- hMin (整数): 色相最小值，范围 0-180，默认值 0\n"
           "- hMax (整数): 色相最大值，范围 0-180，默认值 15\n"
           "- sMin (整数): 饱和度最小值，范围 0-255，默认值 100\n"
           "- sMax (整数): 饱和度最大值，范围 0-255，默认值 255\n"
           "- vMin (整数): 亮度最小值，范围 0-255，默认值 100\n"
           "- vMax (整数): 亮度最大值，范围 0-255，默认值 255\n"
           "- showMask (布尔): 是否显示二值掩码而不是提取的颜色，默认值 false";
}

int HSVColorExtraction::getId() const {
    return 10; // 确保此ID与其他算法不冲突
}

QList<ParameterMeta> HSVColorExtraction::getParametersMeta() const {
    QList<ParameterMeta> meta;
    
    // 色相范围参数
    ParameterMeta hMinMeta;
    hMinMeta.name = "hMin";
    hMinMeta.displayName = "H最小值";
    hMinMeta.description = "色相最小值 (0-180)";
    hMinMeta.type = ParamType::Int;
    hMinMeta.defaultValue = 0;
    hMinMeta.minValue = 0;
    hMinMeta.maxValue = 180;
    meta.append(hMinMeta);
    
    ParameterMeta hMaxMeta;
    hMaxMeta.name = "hMax";
    hMaxMeta.displayName = "H最大值";
    hMaxMeta.description = "色相最大值 (0-180)";
    hMaxMeta.type = ParamType::Int;
    hMaxMeta.defaultValue = 15;
    hMaxMeta.minValue = 0;
    hMaxMeta.maxValue = 180;
    meta.append(hMaxMeta);
    
    // 饱和度范围参数
    ParameterMeta sMinMeta;
    sMinMeta.name = "sMin";
    sMinMeta.displayName = "S最小值";
    sMinMeta.description = "饱和度最小值 (0-255)";
    sMinMeta.type = ParamType::Int;
    sMinMeta.defaultValue = 100;
    sMinMeta.minValue = 0;
    sMinMeta.maxValue = 255;
    meta.append(sMinMeta);
    
    ParameterMeta sMaxMeta;
    sMaxMeta.name = "sMax";
    sMaxMeta.displayName = "S最大值";
    sMaxMeta.description = "饱和度最大值 (0-255)";
    sMaxMeta.type = ParamType::Int;
    sMaxMeta.defaultValue = 255;
    sMaxMeta.minValue = 0;
    sMaxMeta.maxValue = 255;
    meta.append(sMaxMeta);
    
    // 亮度范围参数
    ParameterMeta vMinMeta;
    vMinMeta.name = "vMin";
    vMinMeta.displayName = "V最小值";
    vMinMeta.description = "亮度最小值 (0-255)";
    vMinMeta.type = ParamType::Int;
    vMinMeta.defaultValue = 100;
    vMinMeta.minValue = 0;
    vMinMeta.maxValue = 255;
    meta.append(vMinMeta);
    
    ParameterMeta vMaxMeta;
    vMaxMeta.name = "vMax";
    vMaxMeta.displayName = "V最大值";
    vMaxMeta.description = "亮度最大值 (0-255)";
    vMaxMeta.type = ParamType::Int;
    vMaxMeta.defaultValue = 255;
    vMaxMeta.minValue = 0;
    vMaxMeta.maxValue = 255;
    meta.append(vMaxMeta);
    
    // 掩码显示选项
    ParameterMeta showMaskMeta;
    showMaskMeta.name = "showMask";
    showMaskMeta.displayName = "显示掩码";
    showMaskMeta.description = "是否显示二值掩码而不是提取的颜色";
    showMaskMeta.type = ParamType::Bool;
    showMaskMeta.defaultValue = false;
    meta.append(showMaskMeta);
    
    return meta;
}

Algorithm* HSVColorExtraction::clone() const {
    HSVColorExtraction* copy = new HSVColorExtraction();
    copy->m_hMin = this->m_hMin;
    copy->m_hMax = this->m_hMax;
    copy->m_sMin = this->m_sMin;
    copy->m_sMax = this->m_sMax;
    copy->m_vMin = this->m_vMin;
    copy->m_vMax = this->m_vMax;
    copy->m_showMask = this->m_showMask;
    return copy;
}