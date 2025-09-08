#include "videoexporter.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QApplication>

VideoExporter::VideoExporter(QObject *parent)
    : QObject(parent)
    , m_cancelled(false)
{
}

VideoExporter::~VideoExporter()
{
}

void VideoExporter::setSourceVideo(const QString &sourcePath)
{
    m_sourcePath = sourcePath;
}

void VideoExporter::addWidgetConfig(const QString &widgetName, const QVector<Algorithm*> &algorithms)
{
    WidgetExportConfig config;
    config.widgetName = widgetName;
    config.algorithms = algorithms;  // 直接复制算法指针，避免后续重复获取
    m_widgetConfigs.append(config);
}

void VideoExporter::setWidgetConfigs(const QVector<WidgetExportConfig> &configs)
{
    m_widgetConfigs = configs;  // 直接批量设置，更高效
}

void VideoExporter::startExport(const QString &exportDir)
{
    if (m_sourcePath.isEmpty()) {
        emit exportError("源视频路径为空");
        return;
    }
    
    if (m_widgetConfigs.isEmpty()) {
        emit exportError("没有可导出的Widget配置");
        return;
    }
    
    // 打开源视频获取信息
    cv::VideoCapture cap(m_sourcePath.toStdString());
    if (!cap.isOpened()) {
        emit exportError("无法打开源视频文件：" + m_sourcePath);
        return;
    }
    
    // 获取视频信息
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
    cap.release();
    
    m_cancelled = false;
    int exportedCount = 0;
    
    // 为每个widget导出视频
    for (int i = 0; i < m_widgetConfigs.size(); i++) {
        if (m_cancelled) {
            emit exportCancelled();
            return;
        }
        
        const WidgetExportConfig &config = m_widgetConfigs[i];
        QString outputPath = generateOutputFileName(m_sourcePath, config.widgetName, exportDir);
        
        if (exportSingleWidget(config, outputPath, i, m_widgetConfigs.size(), totalFrames)) {
            exportedCount++;
        } else {
            if (!m_cancelled) {
                emit exportError(QString("导出Widget %1 失败").arg(config.widgetName));
            }
            return;
        }
        
        QApplication::processEvents(); // 处理UI事件
    }
    
    if (!m_cancelled) {
        emit exportCompleted(exportedCount);
    }
}

void VideoExporter::cancelExport()
{
    m_cancelled = true;
}

bool VideoExporter::exportSingleWidget(const WidgetExportConfig &config, const QString &outputPath, 
                                      int widgetIndex, int totalWidgets, int totalFrames)
{
    // 打开源视频
    cv::VideoCapture cap(m_sourcePath.toStdString());
    if (!cap.isOpened()) {
        return false;
    }
    
    // 获取视频参数
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 0) fps = 30.0; // 默认帧率
    
    int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    
    // 创建输出视频写入器
    cv::VideoWriter writer;
    bool writerInitialized = false;
    
    cv::Mat frame;
    int frameIndex = 0;
    
    while (cap.read(frame) && !m_cancelled) {
        if (frame.empty()) {
            break;
        }
        
        // 应用算法处理帧
        cv::Mat processedFrame = applyAlgorithms(frame, config.algorithms);
        
        // 初始化writer（使用第一帧的尺寸）
        if (!writerInitialized) {
            int outputWidth = processedFrame.cols;
            int outputHeight = processedFrame.rows;
            
            // 使用MP4编码
            int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
            writer.open(outputPath.toStdString(), fourcc, fps, 
                       cv::Size(outputWidth, outputHeight), true);
            
            if (!writer.isOpened()) {
                qDebug() << "无法创建输出视频文件：" << outputPath;
                cap.release();
                return false;
            }
            writerInitialized = true;
        }
        
        // 写入处理后的帧
        writer.write(processedFrame);
        
        frameIndex++;
        
        // 发送进度信号（降低频率以减少UI更新开销）
        if (frameIndex % 5 == 0) { // 每5帧更新一次进度，而不是每帧
            QFileInfo fileInfo(outputPath);
            emit exportProgress(widgetIndex, totalWidgets, frameIndex, totalFrames, fileInfo.fileName());
        }
        
        // 处理UI事件（降低频率）
        if (frameIndex % 30 == 0) { // 每30帧处理一次UI事件，而不是每10帧
            QApplication::processEvents();
        }
    }
    
    cap.release();
    writer.release();
    
    return !m_cancelled && frameIndex > 0;
}

cv::Mat VideoExporter::applyAlgorithms(const cv::Mat &frame, const QVector<Algorithm*> &algorithms)
{
    cv::Mat result = frame.clone();
    
    // 如果没有算法，直接返回原帧
    if (algorithms.isEmpty()) {
        return result;
    }
    
    // 依次应用所有算法
    for (Algorithm* algorithm : algorithms) {
        if (algorithm && !m_cancelled) {
            try {
                result = algorithm->process(result);
            } catch (const std::exception &e) {
                qDebug() << "算法处理异常：" << e.what();
                // 发生异常时返回上一步的结果
                break;
            } catch (...) {
                qDebug() << "算法处理时发生未知异常";
                break;
            }
        }
    }
    
    return result;
}

QString VideoExporter::generateOutputFileName(const QString &sourcePath, const QString &widgetName, const QString &exportDir)
{
    QFileInfo sourceInfo(sourcePath);
    QString baseName = sourceInfo.completeBaseName(); // 不包含扩展名的文件名
    QString extension = "mp4"; // 统一使用mp4格式
    
    QString fileName = QString("%1_%2.%3").arg(baseName, widgetName, extension);
    return QDir(exportDir).absoluteFilePath(fileName);
}