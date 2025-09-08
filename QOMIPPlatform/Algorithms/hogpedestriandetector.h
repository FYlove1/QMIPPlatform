#pragma once
#include "algorithm.h"
#include <opencv2/objdetect.hpp>

class HOGPedestrianDetector : public Algorithm {
public:
    HOGPedestrianDetector();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    cv::HOGDescriptor m_hog;
    double m_hitThreshold;
    double m_scaleFactor;
    int m_minNeighbors;
    bool m_showConfidence;
};