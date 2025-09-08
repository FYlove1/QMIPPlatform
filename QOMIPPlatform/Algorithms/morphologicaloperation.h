#pragma once
#include "algorithm.h"

class MorphologicalOperation : public Algorithm {
public:
    MorphologicalOperation();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    int m_operation;
    int m_kernelSize;
    int m_kernelShape;
    int m_iterations;
};