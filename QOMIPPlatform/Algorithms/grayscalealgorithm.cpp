#include "grayscalealgorithm.h"

GrayscaleAlgorithm::GrayscaleAlgorithm() {
}

cv::Mat GrayscaleAlgorithm::process(const cv::Mat& input) {
    cv::Mat result;
    
    // 如果已经是灰度图，直接返回
    if (input.channels() == 1) {
        return input.clone();
    }
    
    // 转换为灰度图
    cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
    
    // 转回3通道，保持与输入相同的通道数
    if (input.channels() == 3) {
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }
    
    return result;
}

void GrayscaleAlgorithm::setParameters(const QVariantMap& params) {
    // 灰度处理没有参数
    Q_UNUSED(params);
}

QVariantMap GrayscaleAlgorithm::getParameters() const {
    // 灰度处理没有参数
    return QVariantMap();
}

QString GrayscaleAlgorithm::getName() const {
    return "灰度处理";
}

QString GrayscaleAlgorithm::getDescription() const {
    return "将图像转换为灰度模式";
}

int GrayscaleAlgorithm::getId() const {
    return 1; // ALGO_GRAYSCALE
}

QList<ParameterMeta> GrayscaleAlgorithm::getParametersMeta() const {
    // 灰度算法没有参数
    return QList<ParameterMeta>();
}

Algorithm* GrayscaleAlgorithm::clone() const {
    return new GrayscaleAlgorithm(*this);
}