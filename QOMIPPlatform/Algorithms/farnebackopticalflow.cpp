#include "farnebackopticalflow.h"
#include <opencv2/imgproc.hpp>

FarnebackOpticalFlow::FarnebackOpticalFlow() 
    : m_pyrScale(0.5), m_levels(3), m_winSize(15), m_iterations(3),
      m_polyN(5), m_polySigma(1.2), m_visualMode(0), m_arrowSpacing(16) {
}

cv::Mat FarnebackOpticalFlow::flowToColor(const cv::Mat& flow) {
    // 将光流转换为HSV颜色表示
    cv::Mat flow_parts[2];
    cv::split(flow, flow_parts);
    
    cv::Mat magnitude, angle;
    cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
    
    // 归一化幅度
    cv::normalize(magnitude, magnitude, 0, 255, cv::NORM_MINMAX);
    
    // 创建HSV图像
    cv::Mat hsv(flow.size(), CV_8UC3);
    cv::Mat hsv_parts[3];
    hsv_parts[0] = angle * 180 / 360;  // 色调表示方向
    hsv_parts[1] = cv::Mat::ones(flow.size(), CV_8U) * 255;  // 饱和度最大
    hsv_parts[2] = magnitude;  // 亮度表示幅度
    
    cv::merge(hsv_parts, 3, hsv);
    
    cv::Mat bgr;
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
    
    return bgr;
}

void FarnebackOpticalFlow::drawOptFlowMap(const cv::Mat& flow, cv::Mat& dst, 
                                          int step, const cv::Scalar& color) {
    for (int y = 0; y < flow.rows; y += step) {
        for (int x = 0; x < flow.cols; x += step) {
            const cv::Point2f& fxy = flow.at<cv::Point2f>(y, x);
            cv::Point p1(x, y);
            cv::Point p2(cvRound(x + fxy.x), cvRound(y + fxy.y));
            
            // 只绘制有明显运动的箭头
            float magnitude = sqrt(fxy.x * fxy.x + fxy.y * fxy.y);
            if (magnitude > 0.5) {
                cv::arrowedLine(dst, p1, p2, color, 1, cv::LINE_AA, 0, 0.3);
            }
        }
    }
}

cv::Mat FarnebackOpticalFlow::visualizeFlow(const cv::Mat& flow, const cv::Mat& original) {
    cv::Mat output;
    
    switch (m_visualMode) {
        case 0: {
            // 色轮模式
            output = flowToColor(flow);
            
            // 添加色轮图例
            int legendSize = 60;
            cv::Mat legend = cv::Mat::zeros(legendSize, legendSize, CV_8UC3);
            cv::Point center(legendSize/2, legendSize/2);
            
            for (int i = 0; i < 360; i += 10) {
                float angleRad = i * CV_PI / 180;
                cv::Point2f endPt(center.x + 25 * cos(angleRad),
                                 center.y + 25 * sin(angleRad));
                
                // HSV颜色
                cv::Mat hsvPixel(1, 1, CV_8UC3);
                hsvPixel.at<cv::Vec3b>(0, 0) = cv::Vec3b(i/2, 255, 255);
                cv::Mat bgrPixel;
                cv::cvtColor(hsvPixel, bgrPixel, cv::COLOR_HSV2BGR);
                cv::Vec3b color = bgrPixel.at<cv::Vec3b>(0, 0);
                
                cv::line(legend, center, endPt, cv::Scalar(color[0], color[1], color[2]), 2);
            }
            
            // 将图例放在右上角
            cv::Mat roi = output(cv::Rect(output.cols - legendSize - 10, 10, 
                                         legendSize, legendSize));
            cv::addWeighted(roi, 0.3, legend, 0.7, 0, roi);
            
            break;
        }
        case 1: {
            // 箭头模式
            if (original.channels() == 3) {
                output = original.clone();
            } else {
                cv::cvtColor(original, output, cv::COLOR_GRAY2BGR);
            }
            
            // 降低原图亮度
            output *= 0.5;
            
            // 绘制光流箭头
            drawOptFlowMap(flow, output, m_arrowSpacing, cv::Scalar(0, 255, 0));
            
            break;
        }
        case 2: {
            // 幅度模式
            cv::Mat flow_parts[2];
            cv::split(flow, flow_parts);
            
            cv::Mat magnitude, angle;
            cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle);
            
            // 归一化并转换为彩色
            cv::normalize(magnitude, magnitude, 0, 255, cv::NORM_MINMAX);
            magnitude.convertTo(magnitude, CV_8U);
            
            cv::applyColorMap(magnitude, output, cv::COLORMAP_JET);
            
            // 叠加原图轮廓
            if (original.channels() == 3) {
                cv::Mat gray;
                cv::cvtColor(original, gray, cv::COLOR_BGR2GRAY);
                cv::Mat edges;
                cv::Canny(gray, edges, 50, 150);
                cv::Mat edgesBGR;
                cv::cvtColor(edges, edgesBGR, cv::COLOR_GRAY2BGR);
                output = output * 0.7 + edgesBGR * 0.3;
            }
            
            break;
        }
    }
    
    return output;
}

cv::Mat FarnebackOpticalFlow::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // 第一帧初始化
    if (m_previousFrame.empty()) {
        m_previousFrame = gray.clone();
        return input;  // 第一帧返回原图
    }
    
    // 计算光流
    cv::Mat flow;
    cv::calcOpticalFlowFarneback(m_previousFrame, gray, flow,
        m_pyrScale, m_levels, m_winSize, m_iterations,
        m_polyN, m_polySigma, 0);
    
    // 可视化光流
    cv::Mat output = visualizeFlow(flow, input);
    
    // 计算统计信息
    cv::Mat flow_parts[2];
    cv::split(flow, flow_parts);
    cv::Mat magnitude, angle;
    cv::cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle);
    
    double minMag, maxMag, avgMag;
    cv::minMaxLoc(magnitude, &minMag, &maxMag);
    avgMag = cv::mean(magnitude)[0];
    
    // 显示统计信息
    char stats[256];
    sprintf(stats, "Optical Flow Statistics:");
    cv::putText(output, stats, cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
    
    sprintf(stats, "Avg: %.2f, Max: %.2f pixels", avgMag, maxMag);
    cv::putText(output, stats, cv::Point(10, 55),
        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
    
    // 检测主要运动方向
    cv::Scalar meanFlow = cv::mean(flow);
    float mainAngle = atan2(meanFlow[1], meanFlow[0]) * 180 / CV_PI;
    sprintf(stats, "Main direction: %.1f deg", mainAngle);
    cv::putText(output, stats, cv::Point(10, 80),
        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);
    
    // 可视化模式标签
    const char* modeNames[] = {"Color Wheel", "Arrows", "Magnitude"};
    sprintf(stats, "Mode: %s", modeNames[m_visualMode]);
    cv::putText(output, stats, cv::Point(10, output.rows - 10),
        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200, 200, 200), 1);
    
    // 更新前一帧
    m_previousFrame = gray.clone();
    
    return output;
}

void FarnebackOpticalFlow::setParameters(const QVariantMap& params) {
    if (params.contains("pyrScale")) {
        m_pyrScale = params["pyrScale"].toDouble();
        if (m_pyrScale < 0.1) m_pyrScale = 0.1;
        if (m_pyrScale > 0.9) m_pyrScale = 0.9;
    }
    if (params.contains("levels")) {
        m_levels = params["levels"].toInt();
        if (m_levels < 1) m_levels = 1;
        if (m_levels > 10) m_levels = 10;
    }
    if (params.contains("winSize")) {
        m_winSize = params["winSize"].toInt();
        if (m_winSize < 5) m_winSize = 5;
        if (m_winSize > 51) m_winSize = 51;
        if (m_winSize % 2 == 0) m_winSize++;  // 确保是奇数
    }
    if (params.contains("iterations")) {
        m_iterations = params["iterations"].toInt();
        if (m_iterations < 1) m_iterations = 1;
        if (m_iterations > 10) m_iterations = 10;
    }
    if (params.contains("polyN")) {
        m_polyN = params["polyN"].toInt();
        if (m_polyN < 5) m_polyN = 5;
        if (m_polyN > 7) m_polyN = 7;
    }
    if (params.contains("polySigma")) {
        m_polySigma = params["polySigma"].toDouble();
        if (m_polySigma < 1.1) m_polySigma = 1.1;
        if (m_polySigma > 1.5) m_polySigma = 1.5;
    }
    if (params.contains("visualMode")) {
        m_visualMode = params["visualMode"].toInt();
        if (m_visualMode < 0) m_visualMode = 0;
        if (m_visualMode > 2) m_visualMode = 2;
    }
    if (params.contains("arrowSpacing")) {
        m_arrowSpacing = params["arrowSpacing"].toInt();
        if (m_arrowSpacing < 8) m_arrowSpacing = 8;
        if (m_arrowSpacing > 64) m_arrowSpacing = 64;
    }
    if (params.contains("reset") && params["reset"].toBool()) {
        m_previousFrame = cv::Mat();  // 重置前一帧
    }
}

QVariantMap FarnebackOpticalFlow::getParameters() const {
    QVariantMap params;
    params["pyrScale"] = m_pyrScale;
    params["levels"] = m_levels;
    params["winSize"] = m_winSize;
    params["iterations"] = m_iterations;
    params["polyN"] = m_polyN;
    params["polySigma"] = m_polySigma;
    params["visualMode"] = m_visualMode;
    params["arrowSpacing"] = m_arrowSpacing;
    params["reset"] = false;
    return params;
}

QString FarnebackOpticalFlow::getName() const {
    return "Farneback光流";
}

QString FarnebackOpticalFlow::getDescription() const {
    return "使用Farneback算法计算稠密光流。\n"
           "返回二维向量场，每个像素存储(dx,dy)位移。\n"
           "参数说明：\n"
           "- pyrScale: 金字塔缩放 (0.1-0.9)\n"
           "- levels: 金字塔层数 (1-10)\n"
           "- winSize: 窗口大小 (5-51，奇数)\n"
           "- iterations: 迭代次数 (1-10)\n"
           "- polyN: 多项式展开邻域 (5-7)\n"
           "- polySigma: 高斯标准差 (1.1-1.5)\n"
           "- visualMode: 可视化模式\n"
           "- arrowSpacing: 箭头间距\n"
           "- reset: 重置前一帧";
}

int FarnebackOpticalFlow::getId() const {
    return 17;
}

QList<ParameterMeta> FarnebackOpticalFlow::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta pyrScaleMeta;
    pyrScaleMeta.name = "pyrScale";
    pyrScaleMeta.displayName = "金字塔缩放";
    pyrScaleMeta.description = "图像金字塔缩放系数";
    pyrScaleMeta.type = ParamType::Double;
    pyrScaleMeta.defaultValue = 0.5;
    pyrScaleMeta.minValue = 0.1;
    pyrScaleMeta.maxValue = 0.9;
    metaList.append(pyrScaleMeta);
    
    ParameterMeta levelsMeta;
    levelsMeta.name = "levels";
    levelsMeta.displayName = "金字塔层数";
    levelsMeta.description = "金字塔层数";
    levelsMeta.type = ParamType::Int;
    levelsMeta.defaultValue = 3;
    levelsMeta.minValue = 1;
    levelsMeta.maxValue = 10;
    metaList.append(levelsMeta);
    
    ParameterMeta winSizeMeta;
    winSizeMeta.name = "winSize";
    winSizeMeta.displayName = "窗口大小";
    winSizeMeta.description = "平均窗口大小";
    winSizeMeta.type = ParamType::Int;
    winSizeMeta.defaultValue = 15;
    winSizeMeta.minValue = 5;
    winSizeMeta.maxValue = 51;
    metaList.append(winSizeMeta);
    
    ParameterMeta iterMeta;
    iterMeta.name = "iterations";
    iterMeta.displayName = "迭代次数";
    iterMeta.description = "每层迭代次数";
    iterMeta.type = ParamType::Int;
    iterMeta.defaultValue = 3;
    iterMeta.minValue = 1;
    iterMeta.maxValue = 10;
    metaList.append(iterMeta);
    
    ParameterMeta polyNMeta;
    polyNMeta.name = "polyN";
    polyNMeta.displayName = "多项式大小";
    polyNMeta.description = "像素邻域大小";
    polyNMeta.type = ParamType::Int;
    polyNMeta.defaultValue = 5;
    polyNMeta.minValue = 5;
    polyNMeta.maxValue = 7;
    metaList.append(polyNMeta);
    
    ParameterMeta polySigmaMeta;
    polySigmaMeta.name = "polySigma";
    polySigmaMeta.displayName = "高斯标准差";
    polySigmaMeta.description = "多项式展开的高斯标准差";
    polySigmaMeta.type = ParamType::Double;
    polySigmaMeta.defaultValue = 1.2;
    polySigmaMeta.minValue = 1.1;
    polySigmaMeta.maxValue = 1.5;
    metaList.append(polySigmaMeta);
    
    ParameterMeta visualMeta;
    visualMeta.name = "visualMode";
    visualMeta.displayName = "可视化模式";
    visualMeta.description = "光流可视化方式";
    visualMeta.type = ParamType::Enum;
    visualMeta.defaultValue = 0;
    visualMeta.enumOptions = QStringList() << "色轮" << "箭头" << "幅度图";
    metaList.append(visualMeta);
    
    ParameterMeta arrowMeta;
    arrowMeta.name = "arrowSpacing";
    arrowMeta.displayName = "箭头间距";
    arrowMeta.description = "箭头模式下的间距";
    arrowMeta.type = ParamType::Int;
    arrowMeta.defaultValue = 16;
    arrowMeta.minValue = 8;
    arrowMeta.maxValue = 64;
    metaList.append(arrowMeta);
    
    ParameterMeta resetMeta;
    resetMeta.name = "reset";
    resetMeta.displayName = "重置";
    resetMeta.description = "重置前一帧缓存";
    resetMeta.type = ParamType::Bool;
    resetMeta.defaultValue = false;
    metaList.append(resetMeta);
    
    return metaList;
}

Algorithm* FarnebackOpticalFlow::clone() const {
    FarnebackOpticalFlow* copy = new FarnebackOpticalFlow();
    copy->m_pyrScale = this->m_pyrScale;
    copy->m_levels = this->m_levels;
    copy->m_winSize = this->m_winSize;
    copy->m_iterations = this->m_iterations;
    copy->m_polyN = this->m_polyN;
    copy->m_polySigma = this->m_polySigma;
    copy->m_visualMode = this->m_visualMode;
    copy->m_arrowSpacing = this->m_arrowSpacing;
    copy->m_previousFrame = this->m_previousFrame.clone();
    return copy;
}