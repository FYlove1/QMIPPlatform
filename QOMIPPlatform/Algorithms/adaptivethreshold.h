#pragma once
#include "algorithm.h"

class AdaptiveThreshold : public Algorithm {
public:
    AdaptiveThreshold();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    int m_blockSize;
    double m_C;
    int m_method;
    bool m_invert;
};