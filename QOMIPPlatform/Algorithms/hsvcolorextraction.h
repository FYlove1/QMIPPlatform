#pragma once
#include "algorithm.h"

class HSVColorExtraction : public Algorithm {
public:
    HSVColorExtraction();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    int m_hMin, m_hMax;
    int m_sMin, m_sMax;
    int m_vMin, m_vMax;
    bool m_showMask;
};