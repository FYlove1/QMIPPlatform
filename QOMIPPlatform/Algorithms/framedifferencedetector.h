#pragma once
#include "algorithm.h"

class FrameDifferenceDetector : public Algorithm {
public:
    FrameDifferenceDetector();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    cv::Mat m_previousFrame;
    int m_threshold;
    int m_dilateSize;
    bool m_showMotionOnly;
};