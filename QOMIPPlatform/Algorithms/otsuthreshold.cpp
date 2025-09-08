#include "otsuthreshold.h"

OtsuThreshold::OtsuThreshold() : m_invert(false) {
}

cv::Mat OtsuThreshold::process(const cv::Mat& input) {
    cv::Mat gray, output;
    
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input;
    }
    
    int thresholdType = m_invert ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
    cv::threshold(gray, output, 0, 255, thresholdType | cv::THRESH_OTSU);
    
    if (input.channels() == 3) {
        cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);
    }
    
    return output;
}

void OtsuThreshold::setParameters(const QVariantMap& params) {
    if (params.contains("invert")) {
        m_invert = params["invert"].toBool();
    }
}

QVariantMap OtsuThreshold::getParameters() const {
    QVariantMap params;
    params["invert"] = m_invert;
    return params;
}

QString OtsuThreshold::getName() const {
    return "Otsu二值化";
}

QString OtsuThreshold::getDescription() const {
    return "使用Otsu方法自动计算最佳阈值进行二值化。\n"
           "参数需求：\n"
           "- invert (布尔): 是否反转二值化结果，默认值 false";
}

int OtsuThreshold::getId() const {
    return 6;
}

QList<ParameterMeta> OtsuThreshold::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta invertMeta;
    invertMeta.name = "invert";
    invertMeta.displayName = "反转";
    invertMeta.description = "是否反转二值化结果";
    invertMeta.type = ParamType::Bool;
    invertMeta.defaultValue = false;
    metaList.append(invertMeta);
    
    return metaList;
}

Algorithm* OtsuThreshold::clone() const {
    OtsuThreshold* copy = new OtsuThreshold();
    copy->m_invert = this->m_invert;
    return copy;
}