#include "haarfacedetector.h"
#include <QFile>
#include <QDir>
#include <QDebug>

HaarFaceDetector::HaarFaceDetector() 
    : m_scaleFactor(1.1), m_minNeighbors(3), m_minSize(30), 
      m_detectEyes(false), m_drawFeatures(true) {
    loadCascades();
}

QString HaarFaceDetector::findCascadeFile(const QString& filename) {
    // 可能的级联文件路径
    QStringList searchPaths = {
        QDir::currentPath() + "/haarcascades/",
        QDir::currentPath() + "/data/",
        QDir::currentPath() + "/cascades/",
        "/usr/share/opencv4/haarcascades/",
        "/usr/share/opencv/haarcascades/",
        "/usr/local/share/opencv4/haarcascades/",
        "/opt/opencv/share/haarcascades/",
        QDir::homePath() + "/.local/share/opencv/haarcascades/",
        // Windows路径
        "C:/opencv/data/haarcascades/",
        "C:/opencv/build/etc/haarcascades/",
        // Qt资源路径
        ":/haarcascades/",
        ":/data/haarcascades/"
    };
    
    for (const QString& path : searchPaths) {
        QString fullPath = path + filename;
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // 直接尝试文件名（可能在当前目录）
    if (QFile::exists(filename)) {
        return filename;
    }
    
    return QString();
}

bool HaarFaceDetector::loadCascades() {
    // 尝试加载人脸级联分类器
    QString faceCascadePath = findCascadeFile("haarcascade_frontalface_default.xml");
    if (!faceCascadePath.isEmpty()) {
        if (!m_faceCascade.load(faceCascadePath.toStdString())) {
            qWarning() << "Failed to load face cascade from:" << faceCascadePath;
            return false;
        }
    } else {
        qWarning() << "Face cascade file not found";
        // 尝试使用OpenCV内置路径（如果编译时包含）
        try {
            if (!m_faceCascade.load(cv::samples::findFile("haarcascades/haarcascade_frontalface_default.xml"))) {
                qWarning() << "Failed to load face cascade using cv::samples::findFile";
                return false;
            }
        } catch(...) {
            qWarning() << "Could not find face cascade file";
            return false;
        }
    }
    
    // 尝试加载眼睛级联分类器（可选）
    QString eyeCascadePath = findCascadeFile("haarcascade_eye.xml");
    if (!eyeCascadePath.isEmpty()) {
        m_eyeCascade.load(eyeCascadePath.toStdString());
    }
    
    return !m_faceCascade.empty();
}

cv::Mat HaarFaceDetector::process(const cv::Mat& input) {
    if (input.empty()) {
        return input;
    }
    
    cv::Mat output = input.clone();
    
    // 如果级联分类器未加载，显示错误信息
    if (m_faceCascade.empty()) {
        cv::Scalar color = input.channels() == 3 ? cv::Scalar(0, 0, 255) : cv::Scalar(255);
        cv::putText(output, "Error: Face cascade not loaded", cv::Point(10, 30),
            cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
        cv::putText(output, "Please check haarcascade files", cv::Point(10, 60),
            cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
        return output;
    }
    
    // 转换为灰度图进行检测
    cv::Mat gray;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input.clone();
    }
    
    // 直方图均衡化提高检测率
    cv::equalizeHist(gray, gray);
    
    // 检测人脸
    std::vector<cv::Rect> faces;
    m_faceCascade.detectMultiScale(gray, faces, m_scaleFactor, m_minNeighbors, 
        0, cv::Size(m_minSize, m_minSize));
    
    // 绘制检测结果
    for (size_t i = 0; i < faces.size(); i++) {
        cv::Scalar faceColor = input.channels() == 3 ? cv::Scalar(255, 0, 255) : cv::Scalar(255);
        
        if (m_drawFeatures) {
            // 绘制人脸椭圆
            cv::Point center(faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2);
            cv::ellipse(output, center, 
                cv::Size(faces[i].width/2, faces[i].height/2), 
                0, 0, 360, faceColor, 2);
        } else {
            // 绘制矩形框
            cv::rectangle(output, faces[i], faceColor, 2);
        }
        
        // 添加标签
        char label[50];
        sprintf(label, "Face %zu", i + 1);
        cv::putText(output, label, 
            cv::Point(faces[i].x, faces[i].y - 5),
            cv::FONT_HERSHEY_SIMPLEX, 0.5, faceColor, 1);
        
        // 如果启用眼睛检测
        if (m_detectEyes && !m_eyeCascade.empty()) {
            cv::Mat faceROI = gray(faces[i]);
            std::vector<cv::Rect> eyes;
            m_eyeCascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0, 
                cv::Size(m_minSize/4, m_minSize/4));
            
            // 绘制眼睛
            for (size_t j = 0; j < eyes.size() && j < 2; j++) {
                cv::Scalar eyeColor = input.channels() == 3 ? cv::Scalar(0, 255, 0) : cv::Scalar(200);
                cv::Point eye_center(faces[i].x + eyes[j].x + eyes[j].width/2,
                                    faces[i].y + eyes[j].y + eyes[j].height/2);
                int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
                cv::circle(output, eye_center, radius, eyeColor, 2);
            }
        }
    }
    
    // 显示统计信息
    char stats[100];
    sprintf(stats, "Detected: %zu face(s)", faces.size());
    cv::Scalar statsColor = input.channels() == 3 ? cv::Scalar(0, 255, 255) : cv::Scalar(255);
    cv::putText(output, stats, cv::Point(10, 30),
        cv::FONT_HERSHEY_SIMPLEX, 0.7, statsColor, 2);
    
    return output;
}

void HaarFaceDetector::setParameters(const QVariantMap& params) {
    if (params.contains("scaleFactor")) {
        m_scaleFactor = params["scaleFactor"].toDouble();
        if (m_scaleFactor < 1.01) m_scaleFactor = 1.01;
        if (m_scaleFactor > 2.0) m_scaleFactor = 2.0;
    }
    if (params.contains("minNeighbors")) {
        m_minNeighbors = params["minNeighbors"].toInt();
        if (m_minNeighbors < 1) m_minNeighbors = 1;
        if (m_minNeighbors > 10) m_minNeighbors = 10;
    }
    if (params.contains("minSize")) {
        m_minSize = params["minSize"].toInt();
        if (m_minSize < 10) m_minSize = 10;
        if (m_minSize > 200) m_minSize = 200;
    }
    if (params.contains("detectEyes")) {
        m_detectEyes = params["detectEyes"].toBool();
    }
    if (params.contains("drawFeatures")) {
        m_drawFeatures = params["drawFeatures"].toBool();
    }
    if (params.contains("reload") && params["reload"].toBool()) {
        loadCascades();
    }
}

QVariantMap HaarFaceDetector::getParameters() const {
    QVariantMap params;
    params["scaleFactor"] = m_scaleFactor;
    params["minNeighbors"] = m_minNeighbors;
    params["minSize"] = m_minSize;
    params["detectEyes"] = m_detectEyes;
    params["drawFeatures"] = m_drawFeatures;
    params["reload"] = false;
    return params;
}

QString HaarFaceDetector::getName() const {
    return "Haar人脸检测";
}

QString HaarFaceDetector::getDescription() const {
    return "使用Haar级联分类器检测人脸。\n"
           "返回检测到的人脸矩形列表。\n"
           "参数说明：\n"
           "- scaleFactor: 图像金字塔缩放因子 (1.01-2.0)\n"
           "- minNeighbors: 最小邻居数 (1-10)\n"
           "- minSize: 最小人脸尺寸 (10-200)\n"
           "- detectEyes: 是否检测眼睛\n"
           "- drawFeatures: 使用椭圆绘制人脸\n"
           "- reload: 重新加载级联文件";
}

int HaarFaceDetector::getId() const {
    return 15;
}

QList<ParameterMeta> HaarFaceDetector::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta scaleMeta;
    scaleMeta.name = "scaleFactor";
    scaleMeta.displayName = "缩放因子";
    scaleMeta.description = "图像金字塔缩放因子";
    scaleMeta.type = ParamType::Double;
    scaleMeta.defaultValue = 1.1;
    scaleMeta.minValue = 1.01;
    scaleMeta.maxValue = 2.0;
    metaList.append(scaleMeta);
    
    ParameterMeta neighborsMeta;
    neighborsMeta.name = "minNeighbors";
    neighborsMeta.displayName = "最小邻居数";
    neighborsMeta.description = "保留矩形的最小邻居数";
    neighborsMeta.type = ParamType::Int;
    neighborsMeta.defaultValue = 3;
    neighborsMeta.minValue = 1;
    neighborsMeta.maxValue = 10;
    metaList.append(neighborsMeta);
    
    ParameterMeta sizeMeta;
    sizeMeta.name = "minSize";
    sizeMeta.displayName = "最小尺寸";
    sizeMeta.description = "最小人脸尺寸（像素）";
    sizeMeta.type = ParamType::Int;
    sizeMeta.defaultValue = 30;
    sizeMeta.minValue = 10;
    sizeMeta.maxValue = 200;
    metaList.append(sizeMeta);
    
    ParameterMeta eyesMeta;
    eyesMeta.name = "detectEyes";
    eyesMeta.displayName = "检测眼睛";
    eyesMeta.description = "在人脸区域检测眼睛";
    eyesMeta.type = ParamType::Bool;
    eyesMeta.defaultValue = false;
    metaList.append(eyesMeta);
    
    ParameterMeta featuresMeta;
    featuresMeta.name = "drawFeatures";
    featuresMeta.displayName = "椭圆绘制";
    featuresMeta.description = "使用椭圆而非矩形绘制人脸";
    featuresMeta.type = ParamType::Bool;
    featuresMeta.defaultValue = true;
    metaList.append(featuresMeta);
    
    ParameterMeta reloadMeta;
    reloadMeta.name = "reload";
    reloadMeta.displayName = "重新加载";
    reloadMeta.description = "重新加载级联分类器";
    reloadMeta.type = ParamType::Bool;
    reloadMeta.defaultValue = false;
    metaList.append(reloadMeta);
    
    return metaList;
}

Algorithm* HaarFaceDetector::clone() const {
    HaarFaceDetector* copy = new HaarFaceDetector();
    copy->m_scaleFactor = this->m_scaleFactor;
    copy->m_minNeighbors = this->m_minNeighbors;
    copy->m_minSize = this->m_minSize;
    copy->m_detectEyes = this->m_detectEyes;
    copy->m_drawFeatures = this->m_drawFeatures;
    // 级联分类器会在构造函数中加载
    return copy;
}