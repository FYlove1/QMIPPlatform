#include "orbfeaturedetector.h"

ORBFeatureDetector::ORBFeatureDetector() 
    : m_nFeatures(500), m_scaleFactor(1.2f), m_nLevels(8), 
      m_edgeThreshold(31), m_drawMode(1), m_showDescriptors(false) {
    updateORB();
}

void ORBFeatureDetector::updateORB() {
    m_orb = cv::ORB::create(m_nFeatures, m_scaleFactor, m_nLevels, m_edgeThreshold);
}

cv::Mat ORBFeatureDetector::process(const cv::Mat& input) {
    if (input.empty() || !m_orb) {
        return input;
    }
    
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // 检测关键点和计算描述子
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    m_orb->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);
    
    cv::Mat output = input.clone();
    
    // 根据模式绘制关键点
    if (m_drawMode == 2) {
        // Rich mode - 显示方向和大小
        cv::drawKeypoints(input, keypoints, output, cv::Scalar(0, 255, 0),
            cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    } else {
        // 自定义绘制
        for (const auto& kp : keypoints) {
            cv::Point2f pt = kp.pt;
            float size = kp.size;
            float angle = kp.angle;
            
            if (m_drawMode == 0) {
                // 简单点模式
                if (input.channels() == 3) {
                    cv::circle(output, pt, 2, cv::Scalar(0, 255, 0), -1);
                } else {
                    cv::circle(output, pt, 2, cv::Scalar(255), -1);
                }
            } else if (m_drawMode == 1) {
                // 圆圈模式
                int radius = cvRound(size / 2);
                if (input.channels() == 3) {
                    cv::circle(output, pt, radius, cv::Scalar(0, 255, 0), 1);
                    // 绘制方向
                    if (angle >= 0) {
                        float angleRad = angle * CV_PI / 180;
                        cv::Point2f endPt(pt.x + radius * cos(angleRad),
                                          pt.y + radius * sin(angleRad));
                        cv::line(output, pt, endPt, cv::Scalar(0, 255, 0), 1);
                    }
                } else {
                    cv::circle(output, pt, radius, cv::Scalar(255), 1);
                    if (angle >= 0) {
                        float angleRad = angle * CV_PI / 180;
                        cv::Point2f endPt(pt.x + radius * cos(angleRad),
                                          pt.y + radius * sin(angleRad));
                        cv::line(output, pt, endPt, cv::Scalar(255), 1);
                    }
                }
            }
        }
    }
    
    // 显示统计信息
    char stats[256];
    sprintf(stats, "ORB Features: %zu keypoints", keypoints.size());
    cv::Scalar statsColor = input.channels() == 3 ? cv::Scalar(255, 255, 0) : cv::Scalar(255);
    cv::putText(output, stats, cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, statsColor, 2);
    
    if (m_showDescriptors && !descriptors.empty()) {
        sprintf(stats, "Descriptors: %dx%d", descriptors.rows, descriptors.cols);
        cv::putText(output, stats, cv::Point(10, 60),
            cv::FONT_HERSHEY_SIMPLEX, 0.7, statsColor, 2);
        
        // 显示前几个特征点的响应值
        int showCount = std::min(3, (int)keypoints.size());
        for (int i = 0; i < showCount; i++) {
            sprintf(stats, "KP%d: response=%.2f", i+1, keypoints[i].response);
            cv::putText(output, stats, cv::Point(10, 90 + i*25),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, statsColor, 1);
        }
    }
    
    // 绘制特征分布热力图（可选）
    if (keypoints.size() > 20) {
        // 创建密度图
        int gridSize = 50;
        int gridW = (input.cols + gridSize - 1) / gridSize;
        int gridH = (input.rows + gridSize - 1) / gridSize;
        cv::Mat density = cv::Mat::zeros(gridH, gridW, CV_32F);
        
        for (const auto& kp : keypoints) {
            int gx = kp.pt.x / gridSize;
            int gy = kp.pt.y / gridSize;
            if (gx >= 0 && gx < gridW && gy >= 0 && gy < gridH) {
                density.at<float>(gy, gx) += 1.0f;
            }
        }
        
        // 找出最密集的区域
        double maxDensity;
        cv::Point maxLoc;
        cv::minMaxLoc(density, nullptr, &maxDensity, nullptr, &maxLoc);
        
        if (maxDensity > 3) {
            cv::Rect densestRegion(maxLoc.x * gridSize, maxLoc.y * gridSize, 
                                   gridSize, gridSize);
            if (input.channels() == 3) {
                cv::rectangle(output, densestRegion, cv::Scalar(255, 0, 0), 2);
                cv::putText(output, "Dense", 
                    cv::Point(densestRegion.x, densestRegion.y - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
            } else {
                cv::rectangle(output, densestRegion, cv::Scalar(200), 2);
            }
        }
    }
    
    return output;
}

void ORBFeatureDetector::setParameters(const QVariantMap& params) {
    bool needUpdate = false;
    
    if (params.contains("nFeatures")) {
        int newFeatures = params["nFeatures"].toInt();
        if (newFeatures != m_nFeatures) {
            m_nFeatures = newFeatures;
            if (m_nFeatures < 10) m_nFeatures = 10;
            if (m_nFeatures > 5000) m_nFeatures = 5000;
            needUpdate = true;
        }
    }
    
    if (params.contains("scaleFactor")) {
        float newScale = params["scaleFactor"].toFloat();
        if (newScale != m_scaleFactor) {
            m_scaleFactor = newScale;
            if (m_scaleFactor < 1.1f) m_scaleFactor = 1.1f;
            if (m_scaleFactor > 2.0f) m_scaleFactor = 2.0f;
            needUpdate = true;
        }
    }
    
    if (params.contains("nLevels")) {
        int newLevels = params["nLevels"].toInt();
        if (newLevels != m_nLevels) {
            m_nLevels = newLevels;
            if (m_nLevels < 1) m_nLevels = 1;
            if (m_nLevels > 16) m_nLevels = 16;
            needUpdate = true;
        }
    }
    
    if (params.contains("edgeThreshold")) {
        int newThreshold = params["edgeThreshold"].toInt();
        if (newThreshold != m_edgeThreshold) {
            m_edgeThreshold = newThreshold;
            if (m_edgeThreshold < 0) m_edgeThreshold = 0;
            if (m_edgeThreshold > 100) m_edgeThreshold = 100;
            needUpdate = true;
        }
    }
    
    if (params.contains("drawMode")) {
        m_drawMode = params["drawMode"].toInt();
        if (m_drawMode < 0) m_drawMode = 0;
        if (m_drawMode > 2) m_drawMode = 2;
    }
    
    if (params.contains("showDescriptors")) {
        m_showDescriptors = params["showDescriptors"].toBool();
    }
    
    if (needUpdate) {
        updateORB();
    }
}

QVariantMap ORBFeatureDetector::getParameters() const {
    QVariantMap params;
    params["nFeatures"] = m_nFeatures;
    params["scaleFactor"] = m_scaleFactor;
    params["nLevels"] = m_nLevels;
    params["edgeThreshold"] = m_edgeThreshold;
    params["drawMode"] = m_drawMode;
    params["showDescriptors"] = m_showDescriptors;
    return params;
}

QString ORBFeatureDetector::getName() const {
    return "ORB特征点";
}

QString ORBFeatureDetector::getDescription() const {
    return "使用ORB算法检测特征点。\n"
           "返回关键点列表和描述子。\n"
           "参数说明：\n"
           "- nFeatures: 最大特征点数 (10-5000)\n"
           "- scaleFactor: 金字塔缩放因子 (1.1-2.0)\n"
           "- nLevels: 金字塔层数 (1-16)\n"
           "- edgeThreshold: 边缘阈值 (0-100)\n"
           "- drawMode: 绘制模式 (0:点, 1:圆, 2:富信息)\n"
           "- showDescriptors: 显示描述子信息";
}

int ORBFeatureDetector::getId() const {
    return 16;
}

QList<ParameterMeta> ORBFeatureDetector::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta featuresMeta;
    featuresMeta.name = "nFeatures";
    featuresMeta.displayName = "特征点数";
    featuresMeta.description = "最大特征点数量";
    featuresMeta.type = ParamType::Int;
    featuresMeta.defaultValue = 500;
    featuresMeta.minValue = 10;
    featuresMeta.maxValue = 5000;
    metaList.append(featuresMeta);
    
    ParameterMeta scaleMeta;
    scaleMeta.name = "scaleFactor";
    scaleMeta.displayName = "缩放因子";
    scaleMeta.description = "金字塔层间缩放比例";
    scaleMeta.type = ParamType::Double;
    scaleMeta.defaultValue = 1.2;
    scaleMeta.minValue = 1.1;
    scaleMeta.maxValue = 2.0;
    metaList.append(scaleMeta);
    
    ParameterMeta levelsMeta;
    levelsMeta.name = "nLevels";
    levelsMeta.displayName = "金字塔层数";
    levelsMeta.description = "图像金字塔层数";
    levelsMeta.type = ParamType::Int;
    levelsMeta.defaultValue = 8;
    levelsMeta.minValue = 1;
    levelsMeta.maxValue = 16;
    metaList.append(levelsMeta);
    
    ParameterMeta edgeMeta;
    edgeMeta.name = "edgeThreshold";
    edgeMeta.displayName = "边缘阈值";
    edgeMeta.description = "边缘排除阈值";
    edgeMeta.type = ParamType::Int;
    edgeMeta.defaultValue = 31;
    edgeMeta.minValue = 0;
    edgeMeta.maxValue = 100;
    metaList.append(edgeMeta);
    
    ParameterMeta drawMeta;
    drawMeta.name = "drawMode";
    drawMeta.displayName = "绘制模式";
    drawMeta.description = "特征点绘制方式";
    drawMeta.type = ParamType::Enum;
    drawMeta.defaultValue = 1;
    drawMeta.enumOptions = QStringList() << "点" << "圆圈" << "富信息";
    metaList.append(drawMeta);
    
    ParameterMeta descMeta;
    descMeta.name = "showDescriptors";
    descMeta.displayName = "显示描述子";
    descMeta.description = "显示描述子统计信息";
    descMeta.type = ParamType::Bool;
    descMeta.defaultValue = false;
    metaList.append(descMeta);
    
    return metaList;
}

Algorithm* ORBFeatureDetector::clone() const {
    ORBFeatureDetector* copy = new ORBFeatureDetector();
    copy->m_nFeatures = this->m_nFeatures;
    copy->m_scaleFactor = this->m_scaleFactor;
    copy->m_nLevels = this->m_nLevels;
    copy->m_edgeThreshold = this->m_edgeThreshold;
    copy->m_drawMode = this->m_drawMode;
    copy->m_showDescriptors = this->m_showDescriptors;
    copy->updateORB();
    return copy;
}