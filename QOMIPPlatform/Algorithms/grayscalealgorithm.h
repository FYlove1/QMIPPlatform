#pragma once
#include "algorithm.h"

class GrayscaleAlgorithm : public Algorithm {
public:
    GrayscaleAlgorithm();
    
    // Algorithm接口实现
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
};