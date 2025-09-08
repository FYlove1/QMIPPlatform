#include "aimodelprocessor.h"
#include <algorithm>
#include <chrono>
#include <iostream>

AIModelProcessor::AIModelProcessor()
    : m_modelLoaded(false)
    , m_modelType(MODEL_UNKNOWN)
    , m_preprocessTime(0.0)
    , m_inferenceTime(0.0)
    , m_postprocessTime(0.0)
{
    m_visualizer = std::make_unique<AIResultVisualizer>();
    
    // 初始化默认blob参数
    m_blobParams.width = 416;
    m_blobParams.height = 416;
    m_blobParams.scaleFactor = 1.0/255.0;
    m_blobParams.meanR = 0.0;
    m_blobParams.meanG = 0.0;
    m_blobParams.meanB = 0.0;
    m_blobParams.swapRB = true;
}

AIModelProcessor::~AIModelProcessor()
{
    unloadModel();
}

bool AIModelProcessor::loadModel(const AlgorithmImportDialog::ModelConfig& config)
{
    try {
        // 保存配置
        m_modelPath = config.modelPath.toStdString();
        m_blobParams = config.blobParams;
        
        // 自动检测模型类型
        m_modelType = detectModelType(config.modelPath.toStdString());
        
        // 加载DNN模型
        m_net = cv::dnn::readNet(config.modelPath.toStdString());
        
        if (m_net.empty()) {
            std::cerr << "Failed to load model: " << config.modelPath.toStdString() << std::endl;
            return false;
        }
        
        // 设置计算后端和目标设备
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        
        // 获取默认类别名称
        m_postProcessParams.classNames = getDefaultClassNames(m_modelType);
        
        m_modelLoaded = true;
        
        std::cout << "Model loaded successfully: " << config.modelPath.toStdString() << std::endl;
        std::cout << "Model type: " << static_cast<int>(m_modelType) << std::endl;
        std::cout << "Input size: " << m_blobParams.width << "x" << m_blobParams.height << std::endl;
        
        return true;
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error loading model: " << e.what() << std::endl;
        m_modelLoaded = false;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        m_modelLoaded = false;
        return false;
    }
}

bool AIModelProcessor::loadModel(const std::string& modelPath, const std::string& configPath)
{
    AlgorithmImportDialog::ModelConfig config;
    config.modelPath = QString::fromStdString(modelPath);
    config.modelType = "Generic";
    // 使用默认的blob参数
    config.blobParams = m_blobParams;
    
    m_configPath = configPath;
    return loadModel(config);
}

void AIModelProcessor::unloadModel()
{
    if (m_modelLoaded) {
        m_net = cv::dnn::Net();
        m_modelLoaded = false;
        m_lastResults.clear();
        std::cout << "Model unloaded" << std::endl;
    }
}

cv::Mat AIModelProcessor::processFrame(const cv::Mat& inputFrame)
{
    if (!m_modelLoaded || inputFrame.empty()) {
        return inputFrame.clone();
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // 1. 预处理
        cv::Mat blob = preprocessImage(inputFrame);
        
        auto preprocessEnd = std::chrono::high_resolution_clock::now();
        m_preprocessTime = std::chrono::duration<double, std::milli>(preprocessEnd - startTime).count();
        
        // 2. 推理
        std::vector<cv::Mat> outputs = runInference(blob);
        
        auto inferenceEnd = std::chrono::high_resolution_clock::now();
        m_inferenceTime = std::chrono::duration<double, std::milli>(inferenceEnd - preprocessEnd).count();
        
        // 3. 后处理
        m_lastResults = postProcessResults(outputs, inputFrame.size());
        
        auto postprocessEnd = std::chrono::high_resolution_clock::now();
        m_postprocessTime = std::chrono::duration<double, std::milli>(postprocessEnd - inferenceEnd).count();
        
        // 4. 可视化
        cv::Mat result = m_visualizer->visualizeResults(inputFrame, m_lastResults);
        
        // 添加性能信息
        std::string perfInfo = "Preprocess: " + std::to_string(m_preprocessTime) + "ms, " +
                              "Inference: " + std::to_string(m_inferenceTime) + "ms, " +
                              "Postprocess: " + std::to_string(m_postprocessTime) + "ms";
        
        cv::putText(result, perfInfo, cv::Point(10, result.rows - 20), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        
        return result;
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error in processFrame: " << e.what() << std::endl;
        return inputFrame.clone();
    } catch (const std::exception& e) {
        std::cerr << "Error in processFrame: " << e.what() << std::endl;
        return inputFrame.clone();
    }
}

cv::Mat AIModelProcessor::preprocessImage(const cv::Mat& image)
{
    cv::Mat blob;
    cv::Vec3d mean(m_blobParams.meanR, m_blobParams.meanG, m_blobParams.meanB);
    
    // 使用blobFromImage进行预处理
    cv::dnn::blobFromImage(image, blob, m_blobParams.scaleFactor, 
                          cv::Size(m_blobParams.width, m_blobParams.height),
                          mean, m_blobParams.swapRB, false, CV_32F);
    
    return blob;
}

std::vector<cv::Mat> AIModelProcessor::runInference(const cv::Mat& blob)
{
    // 设置网络输入
    m_net.setInput(blob);
    
    // 获取输出层名称
    std::vector<std::string> outNames = m_net.getUnconnectedOutLayersNames();
    
    // 运行推理
    std::vector<cv::Mat> outputs;
    m_net.forward(outputs, outNames);
    
    return outputs;
}

AIResultVisualizer::AIResult AIModelProcessor::postProcessResults(const std::vector<cv::Mat>& outputs, 
                                                                  const cv::Size& originalSize)
{
    switch (m_modelType) {
        case MODEL_YOLO_DETECTION:
            return postProcessYOLO(outputs, originalSize);
        case MODEL_SSD_DETECTION:
            return postProcessSSD(outputs, originalSize);
        case MODEL_CLASSIFICATION:
            return postProcessClassification(outputs);
        case MODEL_POSE_ESTIMATION:
            return postProcessPoseEstimation(outputs, originalSize);
        case MODEL_FACE_DETECTION:
            return postProcessFaceDetection(outputs, originalSize);
        default:
            return postProcessYOLO(outputs, originalSize); // 默认使用YOLO后处理
    }
}

AIResultVisualizer::AIResult AIModelProcessor::postProcessYOLO(const std::vector<cv::Mat>& outputs, 
                                                              const cv::Size& originalSize)
{
    AIResultVisualizer::AIResult result;
    result.modelType = "YOLO Detection";
    
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    
    for (const auto& output : outputs) {
        float* data = (float*)output.data;
        
        for (int i = 0; i < output.rows; ++i) {
            float confidence = data[4];
            if (confidence > m_postProcessParams.confidenceThreshold) {
                int centerX = (int)(data[0] * originalSize.width);
                int centerY = (int)(data[1] * originalSize.height);
                int width = (int)(data[2] * originalSize.width);
                int height = (int)(data[3] * originalSize.height);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                
                cv::Mat scores = output.row(i).colRange(5, output.cols);
                cv::Point classIdPoint;
                double score;
                cv::minMaxLoc(scores, 0, &score, 0, &classIdPoint);
                
                if (score > m_postProcessParams.confidenceThreshold) {
                    classIds.push_back(classIdPoint.x);
                    confidences.push_back(score);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
            data += output.cols;
        }
    }
    
    // 应用NMS
    std::vector<int> indices;
    if (m_postProcessParams.useNMS) {
        applyNMS(boxes, confidences, classIds, indices);
    } else {
        indices.resize(boxes.size());
        std::iota(indices.begin(), indices.end(), 0);
    }
    
    // 构建检测结果
    for (int idx : indices) {
        AIResultVisualizer::DetectionBox detection;
        detection.rect = boxes[idx];
        detection.confidence = confidences[idx];
        detection.classId = classIds[idx];
        
        if (classIds[idx] < m_postProcessParams.classNames.size()) {
            detection.className = m_postProcessParams.classNames[classIds[idx]];
        } else {
            detection.className = "Class " + std::to_string(classIds[idx]);
        }
        
        result.detections.push_back(detection);
    }
    
    return result;
}

AIResultVisualizer::AIResult AIModelProcessor::postProcessSSD(const std::vector<cv::Mat>& outputs, 
                                                             const cv::Size& originalSize)
{
    AIResultVisualizer::AIResult result;
    result.modelType = "SSD Detection";
    
    // SSD输出格式: [batchId, classId, confidence, left, top, right, bottom]
    if (!outputs.empty()) {
        cv::Mat output = outputs[0];
        float* data = (float*)output.data;
        
        for (int i = 0; i < output.size[2]; ++i) {
            float confidence = data[i * 7 + 2];
            if (confidence > m_postProcessParams.confidenceThreshold) {
                int classId = (int)data[i * 7 + 1];
                int left = (int)(data[i * 7 + 3] * originalSize.width);
                int top = (int)(data[i * 7 + 4] * originalSize.height);
                int right = (int)(data[i * 7 + 5] * originalSize.width);
                int bottom = (int)(data[i * 7 + 6] * originalSize.height);
                
                AIResultVisualizer::DetectionBox detection;
                detection.rect = cv::Rect(left, top, right - left, bottom - top);
                detection.confidence = confidence;
                detection.classId = classId;
                
                if (classId < m_postProcessParams.classNames.size()) {
                    detection.className = m_postProcessParams.classNames[classId];
                } else {
                    detection.className = "Class " + std::to_string(classId);
                }
                
                result.detections.push_back(detection);
            }
        }
    }
    
    return result;
}

AIResultVisualizer::AIResult AIModelProcessor::postProcessClassification(const std::vector<cv::Mat>& outputs)
{
    AIResultVisualizer::AIResult result;
    result.modelType = "Classification";
    
    if (!outputs.empty()) {
        cv::Mat output = outputs[0];
        float* data = (float*)output.data;
        
        // 找出前5个最高置信度的类别
        std::vector<std::pair<float, int>> scores;
        for (int i = 0; i < output.total(); ++i) {
            scores.push_back(std::make_pair(data[i], i));
        }
        
        std::sort(scores.begin(), scores.end(), std::greater<std::pair<float, int>>());
        
        int topK = std::min(5, (int)scores.size());
        for (int i = 0; i < topK; ++i) {
            if (scores[i].first > m_postProcessParams.confidenceThreshold) {
                AIResultVisualizer::Classification cls;
                cls.confidence = scores[i].first;
                cls.classId = scores[i].second;
                
                if (cls.classId < m_postProcessParams.classNames.size()) {
                    cls.className = m_postProcessParams.classNames[cls.classId];
                } else {
                    cls.className = "Class " + std::to_string(cls.classId);
                }
                
                result.classifications.push_back(cls);
            }
        }
    }
    
    return result;
}

AIResultVisualizer::AIResult AIModelProcessor::postProcessPoseEstimation(const std::vector<cv::Mat>& outputs, 
                                                                        const cv::Size& originalSize)
{
    AIResultVisualizer::AIResult result;
    result.modelType = "Pose Estimation";
    
    // 这里实现人体姿态估计的后处理
    // 具体实现取决于模型的输出格式
    
    return result;
}

AIResultVisualizer::AIResult AIModelProcessor::postProcessFaceDetection(const std::vector<cv::Mat>& outputs, 
                                                                       const cv::Size& originalSize)
{
    AIResultVisualizer::AIResult result;
    result.modelType = "Face Detection";
    
    // 这里实现人脸检测的后处理
    // 可以参考SSD的实现方式
    
    return result;
}

void AIModelProcessor::applyNMS(std::vector<cv::Rect>& boxes, std::vector<float>& confidences, 
                               std::vector<int>& classIds, std::vector<int>& indices)
{
    cv::dnn::NMSBoxes(boxes, confidences, m_postProcessParams.confidenceThreshold, 
                      m_postProcessParams.nmsThreshold, indices);
}

// 配置函数实现
void AIModelProcessor::setPostProcessParams(const PostProcessParams& params)
{
    m_postProcessParams = params;
}

void AIModelProcessor::setModelType(ModelType type)
{
    m_modelType = type;
}

void AIModelProcessor::setClassNames(const std::vector<std::string>& classNames)
{
    m_postProcessParams.classNames = classNames;
}

void AIModelProcessor::setConfidenceThreshold(float threshold)
{
    m_postProcessParams.confidenceThreshold = threshold;
}

void AIModelProcessor::setNMSThreshold(float threshold)
{
    m_postProcessParams.nmsThreshold = threshold;
}

// 静态辅助函数
AIModelProcessor::ModelType AIModelProcessor::detectModelType(const std::string& modelPath)
{
    // 根据文件名和路径推测模型类型
    std::string lowerPath = modelPath;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
    
    if (lowerPath.find("yolo") != std::string::npos) {
        return MODEL_YOLO_DETECTION;
    } else if (lowerPath.find("ssd") != std::string::npos) {
        return MODEL_SSD_DETECTION;
    } else if (lowerPath.find("mobilenet") != std::string::npos) {
        return MODEL_CLASSIFICATION;
    } else if (lowerPath.find("pose") != std::string::npos) {
        return MODEL_POSE_ESTIMATION;
    } else if (lowerPath.find("face") != std::string::npos) {
        return MODEL_FACE_DETECTION;
    } else {
        return MODEL_GENERIC;
    }
}

std::vector<std::string> AIModelProcessor::getDefaultClassNames(ModelType modelType)
{
    switch (modelType) {
        case MODEL_YOLO_DETECTION:
        case MODEL_SSD_DETECTION:
            // COCO数据集的80个类别
            return {"person", "bicycle", "car", "motorbike", "aeroplane", "bus", "train", "truck", 
                   "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench",
                   "bird", "cat", "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
                   "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard",
                   "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
                   "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
                   "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza",
                   "donut", "cake", "chair", "sofa", "pottedplant", "bed", "diningtable", "toilet",
                   "tvmonitor", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave",
                   "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors",
                   "teddy bear", "hair drier", "toothbrush"};
        default:
            return {"object"};
    }
}