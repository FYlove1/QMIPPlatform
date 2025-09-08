#pragma once
#include "algorithm.h"
#include <opencv2/video/background_segm.hpp>

class MOG2BackgroundSubtractor : public Algorithm {
public:
    MOG2BackgroundSubtractor();
    ~MOG2BackgroundSubtractor();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    cv::Ptr<cv::BackgroundSubtractorMOG2> m_pMOG2;
    int m_history;
    double m_varThreshold;
    bool m_detectShadows;
    bool m_showForegroundOnly;
    double m_learningRate;
};