#include "originalalgorithm.h"

OriginalAlgorithm::OriginalAlgorithm() {
}

cv::Mat OriginalAlgorithm::process(const cv::Mat& input) {
    // 原始图像处理，直接返回输入
    return input.clone();
}

void OriginalAlgorithm::setParameters(const QVariantMap& params) {
    // 无参数需要设置
    Q_UNUSED(params);
}

QVariantMap OriginalAlgorithm::getParameters() const {
    // 无参数
    return QVariantMap();
}

QString OriginalAlgorithm::getName() const {
    return "原始图像";
}

QString OriginalAlgorithm::getDescription() const {
    return "不对图像进行任何处理，保持原始状态";
}

int OriginalAlgorithm::getId() const {
    return 0; // ALGO_ORIGINAL
}

QList<ParameterMeta> OriginalAlgorithm::getParametersMeta() const {
    // 原始算法没有参数
    return QList<ParameterMeta>();
}

Algorithm* OriginalAlgorithm::clone() const {
    return new OriginalAlgorithm(*this);
}