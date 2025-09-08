#include "mobilenetssdprocessor.h"
#include <algorithm>
#include <iostream>
#include <QFileInfo>

MobileNetSSDProcessor::MobileNetSSDProcessor()
    : m_modelLoaded(false), m_frameCounter(0), m_frameSkipInterval(4)  // 每4帧处理一次，提升流畅度
{
    m_visualizer = std::make_unique<AIResultVisualizer>();
    initCOCOClassNames();
}

MobileNetSSDProcessor::~MobileNetSSDProcessor()
{
    unloadModel();
}

void MobileNetSSDProcessor::initCOCOClassNames()
{
    m_cocoClassNames = QStringList{
        "person", "bicycle", "car", "motorbike", "aeroplane", "bus", "train", "truck",
        "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
        "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
        "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard",
        "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
        "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza",
        "donut", "cake", "chair", "sofa", "pottedplant", "bed", "diningtable", "toilet",
        "tvmonitor", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave",
        "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors",
        "teddy bear", "hair drier", "toothbrush"
    };
}

bool MobileNetSSDProcessor::loadModel(const MobileNetSSDConfigDialog::MobileNetSSDConfig& config)
{
    try {
        // 检查文件是否存在
        if (config.modelPath.isEmpty() || config.configPath.isEmpty()) {
            std::cerr << "Error: Model path or config path is empty" << std::endl;
            return false;
        }
        
        QFileInfo modelFile(config.modelPath);
        QFileInfo configFile(config.configPath);
        
        if (!modelFile.exists()) {
            std::cerr << "Error: Model file does not exist: " << config.modelPath.toStdString() << std::endl;
            return false;
        }
        
        if (!configFile.exists()) {
            std::cerr << "Error: Config file does not exist: " << config.configPath.toStdString() << std::endl;
            return false;
        }
        
        // 检查文件扩展名
        if (!modelFile.suffix().toLower().contains("pb")) {
            std::cerr << "Warning: Model file should be a .pb file, got: " << modelFile.suffix().toStdString() << std::endl;
        }
        
        if (!configFile.suffix().toLower().contains("pbtxt")) {
            std::cerr << "Warning: Config file should be a .pbtxt file, got: " << configFile.suffix().toStdString() << std::endl;
        }
        
        std::cout << "Loading MobileNet SSD model..." << std::endl;
        std::cout << "Model file: " << config.modelPath.toStdString() << " (" << (modelFile.size() / (1024*1024)) << " MB)" << std::endl;
        std::cout << "Config file: " << config.configPath.toStdString() << std::endl;
        
        // 保存配置
        m_config = config;
        
        // 加载TensorFlow模型
        m_net = cv::dnn::readNetFromTensorflow(config.modelPath.toStdString(), 
                                              config.configPath.toStdString());
        
        if (m_net.empty()) {
            std::cerr << "Failed to load MobileNet SSD model - network is empty" << std::endl;
            std::cerr << "Common issues:" << std::endl;
            std::cerr << "1. Model and config files don't match" << std::endl;
            std::cerr << "2. Files are corrupted" << std::endl;
            std::cerr << "3. OpenCV version doesn't support TensorFlow models" << std::endl;
            return false;
        }
        
        // 设置计算后端和目标设备
        m_net.setPreferableBackend(config.backend);
        m_net.setPreferableTarget(config.target);
        
        m_modelLoaded = true;
        
        std::cout << "MobileNet SSD model loaded successfully" << std::endl;
        std::cout << "Backend: " << static_cast<int>(config.backend) << std::endl;
        std::cout << "Target: " << static_cast<int>(config.target) << std::endl;
        
        return true;
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error loading MobileNet SSD: " << e.what() << std::endl;
        std::cerr << "Error details:" << std::endl;
        std::cerr << "- Error code: " << e.code << std::endl;
        std::cerr << "- Function: " << e.func << std::endl;
        std::cerr << "- File: " << e.file << ":" << e.line << std::endl;
        std::cerr << std::endl;
        std::cerr << "Possible solutions:" << std::endl;
        std::cerr << "1. Ensure you have the correct MobileNet SSD files:" << std::endl;
        std::cerr << "   - frozen_inference_graph.pb (model file)" << std::endl;
        std::cerr << "   - ssd_mobilenet_v2_coco_2018_03_29.pbtxt (config file)" << std::endl;
        std::cerr << "2. Download from TensorFlow Model Zoo" << std::endl;
        std::cerr << "3. Make sure files are not corrupted" << std::endl;
        m_modelLoaded = false;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading MobileNet SSD: " << e.what() << std::endl;
        m_modelLoaded = false;
        return false;
    }
}

void MobileNetSSDProcessor::unloadModel()
{
    if (m_modelLoaded) {
        m_net = cv::dnn::Net();
        m_modelLoaded = false;
        m_lastResults.clear();
        m_perfStats.reset();
        std::cout << "MobileNet SSD model unloaded" << std::endl;
    }
}

cv::Mat MobileNetSSDProcessor::processFrame(const cv::Mat& inputFrame)
{
    if (!m_modelLoaded || inputFrame.empty()) {
        return inputFrame.clone();
    }
    
    // 跳帧优化：每N帧处理一次检测，其他帧直接使用上次结果
    m_frameCounter++;
    bool shouldProcess = (m_frameCounter % m_frameSkipInterval == 0);
    
    if (!shouldProcess) {
        // 跳过处理，直接可视化上次结果
        m_perfStats.frameSkipCount++;
        return m_visualizer->visualizeResults(inputFrame, m_lastResults);
    }
    
    startTimer();
    
    try {
        // 1. 预处理
        cv::Mat blob = preprocessImage(inputFrame);
        m_perfStats.preprocessTime = getElapsedTime();
        
        // 2. 推理
        startTimer();
        std::vector<cv::Mat> outputs = runInference(blob);
        m_perfStats.inferenceTime = getElapsedTime();
        
        // 3. 后处理
        startTimer();
        m_lastResults = postProcessDetections(outputs, inputFrame.size());
        m_perfStats.postprocessTime = getElapsedTime();
        
        // 4. 更新性能统计
        m_perfStats.totalTime = m_perfStats.preprocessTime + 
                               m_perfStats.inferenceTime + 
                               m_perfStats.postprocessTime;
        m_perfStats.fps = m_perfStats.totalTime > 0 ? 1000.0 / m_perfStats.totalTime : 0.0;
        m_perfStats.detectionCount = m_lastResults.detections.size();
        
        // 5. 可视化结果
        cv::Mat resultImage = m_visualizer->visualizeResults(inputFrame, m_lastResults);
        
        // 6. 添加性能信息
        if (m_config.showLabels) {
            QString perfText = QString("FPS: %1 | Detections: %2 | Inference: %3ms")
                              .arg(m_perfStats.fps, 0, 'f', 1)
                              .arg(m_perfStats.detectionCount)
                              .arg(m_perfStats.inferenceTime, 0, 'f', 1);
            
            cv::putText(resultImage, perfText.toStdString(), 
                       cv::Point(10, resultImage.rows - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
        
        return resultImage;
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error in MobileNet SSD processing: " << e.what() << std::endl;
        return inputFrame.clone();
    } catch (const std::exception& e) {
        std::cerr << "Error in MobileNet SSD processing: " << e.what() << std::endl;
        return inputFrame.clone();
    }
}

cv::Mat MobileNetSSDProcessor::preprocessImage(const cv::Mat& image)
{
    cv::Mat blob;
    cv::Size inputSize(m_config.inputWidth, m_config.inputHeight);
    cv::Scalar mean(m_config.meanR, m_config.meanG, m_config.meanB);
    
    // 使用MobileNet SSD特定的预处理参数
    cv::dnn::blobFromImage(image, blob, 
                          m_config.scaleFactor,    // 1/127.5
                          inputSize,               // 300x300
                          mean,                    // (127.5, 127.5, 127.5)
                          m_config.swapRB,         // BGR -> RGB
                          false,                   // crop
                          CV_32F);                 // ddepth
    
    return blob;
}

std::vector<cv::Mat> MobileNetSSDProcessor::runInference(const cv::Mat& blob)
{
    // 设置网络输入
    m_net.setInput(blob);
    
    // 运行推理
    std::vector<cv::Mat> outputs;
    m_net.forward(outputs, m_net.getUnconnectedOutLayersNames());
    
    return outputs;
}

AIResultVisualizer::AIResult MobileNetSSDProcessor::postProcessDetections(
    const std::vector<cv::Mat>& outputs, const cv::Size& originalSize)
{
    AIResultVisualizer::AIResult result;
    result.modelType = "MobileNet SSD";
    
    if (outputs.empty()) {
        std::cout << "DEBUG: No outputs from inference" << std::endl;
        return result;
    }
    
    // MobileNet SSD输出格式：[1, 1, N, 7]
    cv::Mat output = outputs[0];
    std::cout << "DEBUG: Output dimensions: " << output.dims << ", shape: ";
    for (int i = 0; i < output.dims; i++) {
        std::cout << output.size[i] << " ";
    }
    std::cout << std::endl;
    
    // 尝试不同的输出格式处理
    if (output.dims == 4 && output.size[3] == 7) {
        // 标准格式 [1, 1, N, 7]
    } else if (output.dims == 2) {
        // 可能是扁平格式 [N, 7]
        output = output.reshape(1, {1, 1, output.rows, output.cols});
    } else {
        std::cout << "DEBUG: Unknown output format, returning empty result" << std::endl;
        return result; // 未知格式，返回空结果
    }
    
    // 清空缓存
    m_boxes.clear();
    m_confidences.clear();
    m_classIds.clear();
    
    float* data = (float*)output.data;
    int numDetections = output.size[2];
    std::cout << "DEBUG: Processing " << numDetections << " detections" << std::endl;
    
    for (int i = 0; i < numDetections; i++) {
        float* detection = data + i * 7;
        
        // 解析检测结果
        int classId = static_cast<int>(detection[1]);
        float confidence = detection[2];
        float left = detection[3];
        float top = detection[4];  
        float right = detection[5];
        float bottom = detection[6];
        
        // 每10个检测打印一个样本用于调试
        if (i % 10 == 0) {
            std::cout << "DEBUG: Detection " << i << " - ClassId: " << classId 
                     << ", Confidence: " << confidence << ", Box: (" << left << ", " << top 
                     << ", " << right << ", " << bottom << ")" << std::endl;
        }
        
        // 过滤低置信度检测
        if (confidence < m_config.confidenceThreshold) {
            continue;
        }
        
        // 检查类别是否启用
        bool classEnabled = isClassEnabled(classId);
        if (!classEnabled) {
            if (i < 5) { // 只打印前几个被过滤的
                std::cout << "DEBUG: Detection " << i << " filtered by class - ClassId: " << classId 
                         << ", PersonOnly: " << m_config.personOnly << std::endl;
            }
            continue;
        }
        
        std::cout << "DEBUG: Detection " << i << " passed filters - ClassId: " << classId 
                 << ", Confidence: " << confidence << std::endl;
        
        // 转换归一化坐标到像素坐标
        cv::Rect box = denormalizeRect(left, top, right, bottom, originalSize);
        
        std::cout << "DEBUG: Denormalized box: (" << box.x << ", " << box.y 
                 << ", " << box.width << ", " << box.height << ") for image size (" 
                 << originalSize.width << "x" << originalSize.height << ")" << std::endl;
        
        // 验证边界框
        if (box.width <= 0 || box.height <= 0 || 
            box.x < 0 || box.y < 0 || 
            box.x + box.width > originalSize.width || 
            box.y + box.height > originalSize.height) {
            std::cout << "DEBUG: Detection " << i << " filtered by invalid box" << std::endl;
            continue;
        }
        
        std::cout << "DEBUG: Detection " << i << " passed all filters, adding to results" << std::endl;
        
        m_boxes.push_back(box);
        m_confidences.push_back(confidence);
        m_classIds.push_back(classId);
    }
    
    std::cout << "DEBUG: After filtering - " << m_boxes.size() << " detections remaining" << std::endl;
    
    // 应用NMS
    m_indices.clear();
    if (m_config.useNMS && !m_boxes.empty()) {
        cv::dnn::NMSBoxes(m_boxes, m_confidences, 
                         m_config.confidenceThreshold, 
                         m_config.nmsThreshold, 
                         m_indices);
    } else {
        // 不使用NMS，保留所有检测
        m_indices.resize(m_boxes.size());
        std::iota(m_indices.begin(), m_indices.end(), 0);
    }
    
    // 限制最大检测数量
    if (m_indices.size() > static_cast<size_t>(m_config.maxDetections)) {
        // 按置信度排序，保留前N个
        std::vector<std::pair<float, int>> confIndexPairs;
        for (int idx : m_indices) {
            confIndexPairs.push_back({m_confidences[idx], idx});
        }
        
        std::sort(confIndexPairs.begin(), confIndexPairs.end(), 
                 std::greater<std::pair<float, int>>());
        
        m_indices.clear();
        for (int i = 0; i < std::min(static_cast<int>(confIndexPairs.size()), 
                                    m_config.maxDetections); ++i) {
            m_indices.push_back(confIndexPairs[i].second);
        }
    }
    
    // 构建最终结果
    for (int idx : m_indices) {
        AIResultVisualizer::DetectionBox detection;
        detection.rect = m_boxes[idx];
        detection.confidence = m_confidences[idx];
        detection.classId = m_classIds[idx];
        
        // 设置类别名称
        if (m_classIds[idx] < m_cocoClassNames.size()) {
            detection.className = m_cocoClassNames[m_classIds[idx]].toStdString();
        } else {
            detection.className = QString("Class %1").arg(m_classIds[idx]).toStdString();
        }
        
        std::cout << "DEBUG: Adding final detection - Class: " << detection.className 
                 << ", Confidence: " << detection.confidence 
                 << ", Box: (" << detection.rect.x << ", " << detection.rect.y 
                 << ", " << detection.rect.width << ", " << detection.rect.height << ")" << std::endl;
        
        result.detections.push_back(detection);
    }
    
    std::cout << "DEBUG: Final result contains " << result.detections.size() << " detections" << std::endl;
    
    return result;
}

cv::Rect MobileNetSSDProcessor::denormalizeRect(float left, float top, float right, float bottom, 
                                               const cv::Size& imageSize)
{
    int x = static_cast<int>(left * imageSize.width);
    int y = static_cast<int>(top * imageSize.height);
    int width = static_cast<int>((right - left) * imageSize.width);
    int height = static_cast<int>((bottom - top) * imageSize.height);
    
    // 确保坐标在图像范围内
    x = std::max(0, std::min(x, imageSize.width - 1));
    y = std::max(0, std::min(y, imageSize.height - 1));
    width = std::max(1, std::min(width, imageSize.width - x));
    height = std::max(1, std::min(height, imageSize.height - y));
    
    return cv::Rect(x, y, width, height);
}

bool MobileNetSSDProcessor::isClassEnabled(int classIndex)
{
    if (m_config.personOnly) {
        // MobileNet SSD模型：背景类=0，person=1，bicycle=2...
        // 所以person类别ID应该是1，不是0
        return classIndex == 1;
    }
    
    // 对于其他类别，需要减1来匹配COCO索引（因为模型有背景类）
    int cocoIndex = classIndex - 1;
    if (cocoIndex < 0 || cocoIndex >= m_cocoClassNames.size()) {
        return false;
    }
    
    QString className = m_cocoClassNames[cocoIndex];
    return m_config.enabledClasses.contains(className);
}

int MobileNetSSDProcessor::getClassIndex(const QString& className)
{
    return m_cocoClassNames.indexOf(className);
}

// 配置管理函数
void MobileNetSSDProcessor::setConfiguration(const MobileNetSSDConfigDialog::MobileNetSSDConfig& config)
{
    m_config = config;
    
    // 如果模型已加载，更新网络设置
    if (m_modelLoaded) {
        m_net.setPreferableBackend(config.backend);
        m_net.setPreferableTarget(config.target);
    }
}

// 实时参数调整函数
void MobileNetSSDProcessor::setConfidenceThreshold(float threshold)
{
    m_config.confidenceThreshold = std::max(0.0f, std::min(1.0f, threshold));
}

void MobileNetSSDProcessor::setNMSThreshold(float threshold)
{
    m_config.nmsThreshold = std::max(0.0f, std::min(1.0f, threshold));
}

void MobileNetSSDProcessor::setEnabledClasses(const QStringList& classes)
{
    m_config.enabledClasses = classes;
    m_config.personOnly = false;  // 自定义类别列表时关闭人员模式
}

void MobileNetSSDProcessor::setPersonOnlyMode(bool enabled)
{
    m_config.personOnly = enabled;
    if (enabled) {
        m_config.enabledClasses.clear();
        m_config.enabledClasses.append("person");
    }
}
