#pragma once
#include "algorithm.h"

class ThresholdFilter : public Algorithm {
public:
    ThresholdFilter();
    
    // Algorithm接口实现
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    int m_threshold;
    int m_maxVal;
};