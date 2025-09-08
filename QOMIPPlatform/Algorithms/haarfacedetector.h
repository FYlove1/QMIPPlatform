#pragma once
#include "algorithm.h"
#include <opencv2/objdetect.hpp>

class HaarFaceDetector : public Algorithm {
public:
    HaarFaceDetector();
    
    cv::Mat process(const cv::Mat& input) override;
    void setParameters(const QVariantMap& params) override;
    QVariantMap getParameters() const override;
    QString getName() const override;
    QString getDescription() const override;
    int getId() const override;
    QList<ParameterMeta> getParametersMeta() const override;
    Algorithm* clone() const override;
    
private:
    cv::CascadeClassifier m_faceCascade;
    cv::CascadeClassifier m_eyeCascade;
    double m_scaleFactor;
    int m_minNeighbors;
    int m_minSize;
    bool m_detectEyes;
    bool m_drawFeatures;
    
    bool loadCascades();
    QString findCascadeFile(const QString& filename);
};