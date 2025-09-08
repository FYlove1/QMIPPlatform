#include "blurdetector.h"
#include <opencv2/imgproc.hpp>

BlurDetector::BlurDetector() 
    : m_threshold(100.0), m_showHeatmap(false), m_blockSize(64) {
}

double BlurDetector::calculateLaplacianVariance(const cv::Mat& src) {
    cv::Mat laplacian;
    cv::Laplacian(src, laplacian, CV_64F);
    
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);
    
    return stddev.val[0] * stddev.val[0];  // 返回方差
}

cv::Mat BlurDetector::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    cv::Mat output = input.clone();
    
    if (m_showHeatmap && m_blockSize > 0) {
        // 生成局部清晰度热力图
        cv::Mat heatmap = cv::Mat::zeros(gray.size(), CV_32F);
        
        int stepX = m_blockSize / 2;
        int stepY = m_blockSize / 2;
        
        for (int y = 0; y < gray.rows - m_blockSize; y += stepY) {
            for (int x = 0; x < gray.cols - m_blockSize; x += stepX) {
                cv::Rect roi(x, y, m_blockSize, m_blockSize);
                cv::Mat block = gray(roi);
                double variance = calculateLaplacianVariance(block);
                
                // 在热力图中填充该区域
                cv::rectangle(heatmap, roi, cv::Scalar(variance), -1);
            }
        }
        
        // 归一化热力图
        cv::normalize(heatmap, heatmap, 0, 255, cv::NORM_MINMAX);
        heatmap.convertTo(heatmap, CV_8U);
        
        // 应用颜色映射
        cv::Mat colormap;
        cv::applyColorMap(heatmap, colormap, cv::COLORMAP_JET);
        
        // 叠加到原图
        if (input.channels() == 3) {
            cv::addWeighted(output, 0.5, colormap, 0.5, 0, output);
        } else {
            cv::cvtColor(colormap, colormap, cv::COLOR_BGR2GRAY);
            cv::addWeighted(output, 0.5, colormap, 0.5, 0, output);
        }
    }
    
    // 计算整体清晰度
    double overallVariance = calculateLaplacianVariance(gray);
    
    // 判断是否模糊
    bool isBlurry = overallVariance < m_threshold;
    cv::Scalar textColor;
    QString status;
    
    if (input.channels() == 3) {
        textColor = isBlurry ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
    } else {
        textColor = cv::Scalar(255);
    }
    
    status = isBlurry ? "BLURRY" : "SHARP";
    
    // 在图像上显示清晰度信息
    char text[256];
    sprintf(text, "Sharpness: %.2f", overallVariance);
    cv::putText(output, text, cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, textColor, 2);
    
    sprintf(text, "Status: %s", status.toStdString().c_str());
    cv::putText(output, text, cv::Point(10, 60),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, textColor, 2);
    
    sprintf(text, "Threshold: %.2f", m_threshold);
    cv::putText(output, text, cv::Point(10, 90),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, textColor, 2);
    
    // 绘制清晰度条
    int barWidth = 200;
    int barHeight = 20;
    int barX = 10;
    int barY = 100;
    
    // 背景条
    cv::rectangle(output, cv::Point(barX, barY), 
        cv::Point(barX + barWidth, barY + barHeight),
        cv::Scalar(100, 100, 100), -1);
    
    // 清晰度条（根据值填充）
    double normalizedValue = std::min(overallVariance / 500.0, 1.0);  // 假设500是最大清晰度
    int fillWidth = (int)(barWidth * normalizedValue);
    
    cv::Scalar barColor;
    if (input.channels() == 3) {
        // 根据清晰度从红到绿渐变
        if (normalizedValue < 0.5) {
            barColor = cv::Scalar(0, (int)(normalizedValue * 2 * 255), 255);
        } else {
            barColor = cv::Scalar(0, 255, (int)((1 - normalizedValue) * 2 * 255));
        }
    } else {
        barColor = cv::Scalar(200);
    }
    
    cv::rectangle(output, cv::Point(barX, barY), 
        cv::Point(barX + fillWidth, barY + barHeight),
        barColor, -1);
    
    // 边框
    cv::rectangle(output, cv::Point(barX, barY), 
        cv::Point(barX + barWidth, barY + barHeight),
        textColor, 2);
    
    return output;
}

void BlurDetector::setParameters(const QVariantMap& params) {
    if (params.contains("threshold")) {
        m_threshold = params["threshold"].toDouble();
        if (m_threshold < 0) m_threshold = 0;
        if (m_threshold > 1000) m_threshold = 1000;
    }
    if (params.contains("showHeatmap")) {
        m_showHeatmap = params["showHeatmap"].toBool();
    }
    if (params.contains("blockSize")) {
        m_blockSize = params["blockSize"].toInt();
        if (m_blockSize < 16) m_blockSize = 16;
        if (m_blockSize > 256) m_blockSize = 256;
    }
}

QVariantMap BlurDetector::getParameters() const {
    QVariantMap params;
    params["threshold"] = m_threshold;
    params["showHeatmap"] = m_showHeatmap;
    params["blockSize"] = m_blockSize;
    return params;
}

QString BlurDetector::getName() const {
    return "模糊度检测";
}

QString BlurDetector::getDescription() const {
    return "使用拉普拉斯方差评估图像清晰度。\n"
           "返回清晰度数值，值越大越清晰。\n"
           "参数说明：\n"
           "- threshold: 模糊判定阈值 (0-1000)\n"
           "- showHeatmap: 显示局部清晰度热力图\n"
           "- blockSize: 热力图块大小 (16-256)";
}

int BlurDetector::getId() const {
    return 13;
}

QList<ParameterMeta> BlurDetector::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta thresholdMeta;
    thresholdMeta.name = "threshold";
    thresholdMeta.displayName = "模糊阈值";
    thresholdMeta.description = "低于此值判定为模糊";
    thresholdMeta.type = ParamType::Double;
    thresholdMeta.defaultValue = 100.0;
    thresholdMeta.minValue = 0.0;
    thresholdMeta.maxValue = 1000.0;
    metaList.append(thresholdMeta);
    
    ParameterMeta heatmapMeta;
    heatmapMeta.name = "showHeatmap";
    heatmapMeta.displayName = "显示热力图";
    heatmapMeta.description = "显示局部清晰度热力图";
    heatmapMeta.type = ParamType::Bool;
    heatmapMeta.defaultValue = false;
    metaList.append(heatmapMeta);
    
    ParameterMeta blockSizeMeta;
    blockSizeMeta.name = "blockSize";
    blockSizeMeta.displayName = "块大小";
    blockSizeMeta.description = "热力图计算块大小";
    blockSizeMeta.type = ParamType::Int;
    blockSizeMeta.defaultValue = 64;
    blockSizeMeta.minValue = 16;
    blockSizeMeta.maxValue = 256;
    metaList.append(blockSizeMeta);
    
    return metaList;
}

Algorithm* BlurDetector::clone() const {
    BlurDetector* copy = new BlurDetector();
    copy->m_threshold = this->m_threshold;
    copy->m_showHeatmap = this->m_showHeatmap;
    copy->m_blockSize = this->m_blockSize;
    return copy;
}