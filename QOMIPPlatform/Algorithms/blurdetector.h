#pragma once
#include "algorithm.h"

class BlurDetector : public Algorithm {
public:
    BlurDetector();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    double m_threshold;
    bool m_showHeatmap;
    int m_blockSize;
    
    double calculateLaplacianVariance(const cv::Mat& src);
};