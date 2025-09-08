#include "mog2backgroundsubtractor.h"

MOG2BackgroundSubtractor::MOG2BackgroundSubtractor() 
    : m_history(500), m_varThreshold(16), m_detectShadows(true), 
      m_showForegroundOnly(false), m_learningRate(-1) {
    m_pMOG2 = cv::createBackgroundSubtractorMOG2(m_history, m_varThreshold, m_detectShadows);
}

MOG2BackgroundSubtractor::~MOG2BackgroundSubtractor() {
}

cv::Mat MOG2BackgroundSubtractor::process(const cv::Mat& input) {
    if (input.empty() || !m_pMOG2) {
        return input;
    }
    
    // 生成前景掩膜
    cv::Mat fgMask;
    m_pMOG2->apply(input, fgMask, m_learningRate);
    
    cv::Mat output;
    if (m_showForegroundOnly) {
        // 只返回前景掩膜
        if (input.channels() == 3) {
            cv::cvtColor(fgMask, output, cv::COLOR_GRAY2BGR);
        } else {
            output = fgMask;
        }
    } else {
        // 在原图上显示前景
        output = input.clone();
        
        if (input.channels() == 3) {
            // 找到前景轮廓
            std::vector<std::vector<cv::Point>> contours;
            cv::Mat fgMaskCopy = fgMask.clone();
            // 移除阴影（127为阴影，255为前景）
            cv::threshold(fgMaskCopy, fgMaskCopy, 200, 255, cv::THRESH_BINARY);
            cv::findContours(fgMaskCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            
            // 绘制前景轮廓和边界框
            for (size_t i = 0; i < contours.size(); i++) {
                double area = cv::contourArea(contours[i]);
                if (area > 100) {  // 过滤小区域
                    cv::Rect boundingBox = cv::boundingRect(contours[i]);
                    cv::rectangle(output, boundingBox, cv::Scalar(0, 255, 0), 2);
                    
                    // 在边界框上方添加标签
                    cv::putText(output, "Foreground", 
                        cv::Point(boundingBox.x, boundingBox.y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
                }
            }
            
            // 添加统计信息
            int foregroundPixels = cv::countNonZero(fgMaskCopy);
            double foregroundRatio = (double)foregroundPixels / (fgMask.rows * fgMask.cols) * 100;
            char text[100];
            sprintf(text, "Foreground: %.1f%%", foregroundRatio);
            cv::putText(output, text, cv::Point(10, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);
        } else {
            // 灰度图：叠加掩膜
            cv::Mat fgMaskNorm;
            cv::threshold(fgMask, fgMaskNorm, 200, 255, cv::THRESH_BINARY);
            cv::addWeighted(output, 0.7, fgMaskNorm, 0.3, 0, output);
        }
    }
    
    return output;
}

void MOG2BackgroundSubtractor::setParameters(const QVariantMap& params) {
    bool needReinit = false;
    
    if (params.contains("history")) {
        int newHistory = params["history"].toInt();
        if (newHistory != m_history) {
            m_history = newHistory;
            if (m_history < 1) m_history = 1;
            if (m_history > 10000) m_history = 10000;
            needReinit = true;
        }
    }
    
    if (params.contains("varThreshold")) {
        double newVarThreshold = params["varThreshold"].toDouble();
        if (newVarThreshold != m_varThreshold) {
            m_varThreshold = newVarThreshold;
            if (m_varThreshold < 0) m_varThreshold = 0;
            if (m_varThreshold > 100) m_varThreshold = 100;
            needReinit = true;
        }
    }
    
    if (params.contains("detectShadows")) {
        bool newDetectShadows = params["detectShadows"].toBool();
        if (newDetectShadows != m_detectShadows) {
            m_detectShadows = newDetectShadows;
            needReinit = true;
        }
    }
    
    if (params.contains("showForegroundOnly")) {
        m_showForegroundOnly = params["showForegroundOnly"].toBool();
    }
    
    if (params.contains("learningRate")) {
        m_learningRate = params["learningRate"].toDouble();
        if (m_learningRate < -1) m_learningRate = -1;
        if (m_learningRate > 1) m_learningRate = 1;
    }
    
    if (params.contains("reset") && params["reset"].toBool()) {
        needReinit = true;
    }
    
    // 重新创建背景减除器
    if (needReinit) {
        m_pMOG2 = cv::createBackgroundSubtractorMOG2(m_history, m_varThreshold, m_detectShadows);
    }
}

QVariantMap MOG2BackgroundSubtractor::getParameters() const {
    QVariantMap params;
    params["history"] = m_history;
    params["varThreshold"] = m_varThreshold;
    params["detectShadows"] = m_detectShadows;
    params["showForegroundOnly"] = m_showForegroundOnly;
    params["learningRate"] = m_learningRate;
    params["reset"] = false;
    return params;
}

QString MOG2BackgroundSubtractor::getName() const {
    return "MOG2背景减除";
}

QString MOG2BackgroundSubtractor::getDescription() const {
    return "使用MOG2算法进行背景建模和前景检测。\n"
           "返回二值掩膜，白色为前景，黑色为背景。\n"
           "参数说明：\n"
           "- history: 历史帧数 (1-10000)\n"
           "- varThreshold: 方差阈值 (0-100)\n"
           "- detectShadows: 是否检测阴影\n"
           "- showForegroundOnly: 只显示前景掩膜\n"
           "- learningRate: 学习率 (-1为自动)\n"
           "- reset: 重置背景模型";
}

int MOG2BackgroundSubtractor::getId() const {
    return 12;
}

QList<ParameterMeta> MOG2BackgroundSubtractor::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta historyMeta;
    historyMeta.name = "history";
    historyMeta.displayName = "历史帧数";
    historyMeta.description = "用于背景建模的历史帧数";
    historyMeta.type = ParamType::Int;
    historyMeta.defaultValue = 500;
    historyMeta.minValue = 1;
    historyMeta.maxValue = 10000;
    metaList.append(historyMeta);
    
    ParameterMeta varThresholdMeta;
    varThresholdMeta.name = "varThreshold";
    varThresholdMeta.displayName = "方差阈值";
    varThresholdMeta.description = "像素与模型匹配的马氏距离平方阈值";
    varThresholdMeta.type = ParamType::Double;
    varThresholdMeta.defaultValue = 16.0;
    varThresholdMeta.minValue = 0.0;
    varThresholdMeta.maxValue = 100.0;
    metaList.append(varThresholdMeta);
    
    ParameterMeta shadowsMeta;
    shadowsMeta.name = "detectShadows";
    shadowsMeta.displayName = "检测阴影";
    shadowsMeta.description = "是否检测并标记阴影";
    shadowsMeta.type = ParamType::Bool;
    shadowsMeta.defaultValue = true;
    metaList.append(shadowsMeta);
    
    ParameterMeta fgOnlyMeta;
    fgOnlyMeta.name = "showForegroundOnly";
    fgOnlyMeta.displayName = "仅显示前景掩膜";
    fgOnlyMeta.description = "只显示前景掩膜";
    fgOnlyMeta.type = ParamType::Bool;
    fgOnlyMeta.defaultValue = false;
    metaList.append(fgOnlyMeta);
    
    ParameterMeta learningRateMeta;
    learningRateMeta.name = "learningRate";
    learningRateMeta.displayName = "学习率";
    learningRateMeta.description = "背景模型学习率，-1为自动";
    learningRateMeta.type = ParamType::Double;
    learningRateMeta.defaultValue = -1.0;
    learningRateMeta.minValue = -1.0;
    learningRateMeta.maxValue = 1.0;
    metaList.append(learningRateMeta);
    
    ParameterMeta resetMeta;
    resetMeta.name = "reset";
    resetMeta.displayName = "重置";
    resetMeta.description = "重置背景模型";
    resetMeta.type = ParamType::Bool;
    resetMeta.defaultValue = false;
    metaList.append(resetMeta);
    
    return metaList;
}

Algorithm* MOG2BackgroundSubtractor::clone() const {
    MOG2BackgroundSubtractor* copy = new MOG2BackgroundSubtractor();
    copy->m_history = this->m_history;
    copy->m_varThreshold = this->m_varThreshold;
    copy->m_detectShadows = this->m_detectShadows;
    copy->m_showForegroundOnly = this->m_showForegroundOnly;
    copy->m_learningRate = this->m_learningRate;
    // 重新创建MOG2实例
    copy->m_pMOG2 = cv::createBackgroundSubtractorMOG2(
        copy->m_history, copy->m_varThreshold, copy->m_detectShadows);
    return copy;
}