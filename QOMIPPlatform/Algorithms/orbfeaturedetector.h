#pragma once
#include "algorithm.h"
#include <opencv2/features2d.hpp>

class ORBFeatureDetector : public Algorithm {
public:
    ORBFeatureDetector();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    int m_nFeatures;
    float m_scaleFactor;
    int m_nLevels;
    int m_edgeThreshold;
    int m_drawMode;  // 0: points, 1: circles, 2: rich
    bool m_showDescriptors;
    
    cv::Ptr<cv::ORB> m_orb;
    void updateORB();
};