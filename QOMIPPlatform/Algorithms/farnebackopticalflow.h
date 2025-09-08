#pragma once
#include "algorithm.h"
#include <opencv2/video/tracking.hpp>

class FarnebackOpticalFlow : public Algorithm {
public:
    FarnebackOpticalFlow();
    
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
    double m_pyrScale;
    int m_levels;
    int m_winSize;
    int m_iterations;
    int m_polyN;
    double m_polySigma;
    int m_visualMode;  // 0: color wheel, 1: arrows, 2: magnitude
    int m_arrowSpacing;
    
    cv::Mat visualizeFlow(const cv::Mat& flow, const cv::Mat& original);
    cv::Mat flowToColor(const cv::Mat& flow);
    void drawOptFlowMap(const cv::Mat& flow, cv::Mat& dst, int step, const cv::Scalar& color);
};