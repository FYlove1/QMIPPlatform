#ifndef VIDEOEXPORTER_H
#define VIDEOEXPORTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <opencv2/opencv.hpp>
#include "frameprocessor.h"

struct WidgetExportConfig {
    QString widgetName;
    QVector<Algorithm*> algorithms;  // 该widget的算法列表（直接缓存，避免重复获取）
};

class VideoExporter : public QObject
{
    Q_OBJECT
    
public:
    explicit VideoExporter(QObject *parent = nullptr);
    ~VideoExporter();
    
    // 设置要导出的视频源
    void setSourceVideo(const QString &sourcePath);
    
    // 添加widget的算法配置（优化版：直接传入预处理好的算法列表）
    void addWidgetConfig(const QString &widgetName, const QVector<Algorithm*> &algorithms);
    
    // 直接设置所有widget配置（批量优化版本）
    void setWidgetConfigs(const QVector<WidgetExportConfig> &configs);
    
    // 开始导出到指定目录
    void startExport(const QString &exportDir);
    
    // 取消导出
    void cancelExport();

signals:
    void exportProgress(int widgetIndex, int totalWidgets, int frameIndex, int totalFrames, const QString &currentFile);
    void exportCompleted(int exportedCount);
    void exportError(const QString &error);
    void exportCancelled();

private:
    QString m_sourcePath;
    QVector<WidgetExportConfig> m_widgetConfigs;
    bool m_cancelled;
    
    // 处理单个widget的导出
    bool exportSingleWidget(const WidgetExportConfig &config, const QString &outputPath, 
                          int widgetIndex, int totalWidgets, int totalFrames);
    
    // 应用算法处理帧
    cv::Mat applyAlgorithms(const cv::Mat &frame, const QVector<Algorithm*> &algorithms);
    
    // 生成输出文件名
    QString generateOutputFileName(const QString &sourcePath, const QString &widgetName, const QString &exportDir);
};

#endif // VIDEOEXPORTER_H