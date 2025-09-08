#include "algorithmfactory.h"
#include "originalalgorithm.h"
#include "grayscalealgorithm.h"
#include "blurfilter.h"
#include "cannyedgedetector.h"
#include "thresholdfilter.h"
#include "hsvcolorextraction.h"
#include "adaptivethreshold.h"
#include "medianblur.h"
#include "morphologicaloperation.h"
#include "otsuthreshold.h"
#include "sobeledgedetector.h"
#include "framedifferencedetector.h"
#include "mog2backgroundsubtractor.h"
#include "blurdetector.h"
#include "hogpedestriandetector.h"
#include "haarfacedetector.h"
#include "orbfeaturedetector.h"
#include "farnebackopticalflow.h"
AlgorithmFactory& AlgorithmFactory::instance() {
    static AlgorithmFactory factory;
    return factory;
}

AlgorithmFactory::AlgorithmFactory() {
     // 注册基础算法
    registerAlgorithm(0, []() { return new OriginalAlgorithm(); });
    registerAlgorithm(1, []() { return new GrayscaleAlgorithm(); });
    registerAlgorithm(2, []() { return new BlurFilter(); });
    registerAlgorithm(3, []() { return new CannyEdgeDetector(); });
    registerAlgorithm(4, []() { return new ThresholdFilter(); });
    
    // 注册图像处理算法
    registerAlgorithm(5, []() { return new MedianBlur(); });
    registerAlgorithm(6, []() { return new OtsuThreshold(); });
    registerAlgorithm(7, []() { return new AdaptiveThreshold(); });
    registerAlgorithm(8, []() { return new SobelEdgeDetector(); });
    registerAlgorithm(9, []() { return new MorphologicalOperation(); });
    registerAlgorithm(10, []() { return new HSVColorExtraction(); });
    
    // 注册运动检测算法
    registerAlgorithm(11, []() { return new FrameDifferenceDetector(); });
    registerAlgorithm(12, []() { return new MOG2BackgroundSubtractor(); });
    
    // 注册质量检测算法
    registerAlgorithm(13, []() { return new BlurDetector(); });
    
    // 注册目标检测算法
    registerAlgorithm(14, []() { return new HOGPedestrianDetector(); });
    registerAlgorithm(15, []() { return new HaarFaceDetector(); });
    
    // 注册特征检测算法
    registerAlgorithm(16, []() { return new ORBFeatureDetector(); });
    
    // 注册光流算法
    registerAlgorithm(17, []() { return new FarnebackOpticalFlow(); });
}

void AlgorithmFactory::registerAlgorithm(int id, std::function<Algorithm*()> creator) {
    m_creators[id] = creator;
}

Algorithm* AlgorithmFactory::createAlgorithm(int id) {
    if (m_creators.contains(id)) {
        return m_creators[id]();
    }
    return nullptr;
}

QList<int> AlgorithmFactory::getRegisteredAlgorithmIds() const {
    return m_creators.keys();
}

QList<QPair<int, QString>> AlgorithmFactory::getAlgorithmInfoList() const {
    QList<QPair<int, QString>> result;
    
    for (auto it = m_creators.begin(); it != m_creators.end(); ++it) {
        Algorithm* algorithm = it.value()();
        if (algorithm) {
            result.append(qMakePair(it.key(), algorithm->getName()));
            delete algorithm;
        }
    }
    
    return result;
}

bool AlgorithmFactory::isAlgorithmRegistered(int id) const {
    return m_creators.contains(id);
}
