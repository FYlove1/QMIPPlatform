#pragma once
#include "algorithm.h"

class SobelEdgeDetector : public Algorithm {
public:
    SobelEdgeDetector();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    int m_kernelSize;
    double m_scale;
    double m_delta;
    int m_direction;
};