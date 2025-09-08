#include "resourceextractor.h"
#include <QResource>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

QStringList ResourceExtractor::m_tempFiles;

QString ResourceExtractor::getTempDir()
{
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    tempDir += "/QOMIPPlatform_models";
    
    QDir dir;
    if (!dir.exists(tempDir)) {
        dir.mkpath(tempDir);
    }
    
    return tempDir;
}

QString ResourceExtractor::extractResource(const QString& resourcePath)
{
    QResource resource(resourcePath);
    
    if (!resource.isValid()) {
        qDebug() << "Resource not found:" << resourcePath;
        return QString();
    }
    
    // 生成临时文件路径
    QString tempDir = getTempDir();
    QString fileName = QFileInfo(resourcePath).fileName();
    QString tempFilePath = tempDir + "/" + fileName;
    
    // 如果文件已存在，直接返回路径
    if (QFile::exists(tempFilePath)) {
        return tempFilePath;
    }
    
    // 提取资源到临时文件
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot create temp file:" << tempFilePath;
        return QString();
    }
    
    const uchar* data = resource.data();
    qint64 size = resource.size();
    
    if (tempFile.write(reinterpret_cast<const char*>(data), size) != size) {
        qDebug() << "Failed to write resource data to temp file";
        tempFile.remove();
        return QString();
    }
    
    tempFile.close();
    
    // 添加到临时文件列表，用于后续清理
    m_tempFiles.append(tempFilePath);
    
    qDebug() << "Extracted resource" << resourcePath << "to" << tempFilePath;
    return tempFilePath;
}

void ResourceExtractor::cleanupTempFiles()
{
    for (const QString& filePath : m_tempFiles) {
        if (QFile::exists(filePath)) {
            QFile::remove(filePath);
            qDebug() << "Cleaned up temp file:" << filePath;
        }
    }
    m_tempFiles.clear();
    
    // 尝试删除临时目录（如果为空）
    QString tempDir = getTempDir();
    QDir dir(tempDir);
    if (dir.isEmpty()) {
        dir.rmdir(tempDir);
    }
}

QString ResourceExtractor::getDefaultModelPath()
{
    // 直接使用外部文件路径，避免大文件嵌入资源导致编译崩溃
    QString modelPath = QCoreApplication::applicationDirPath() + "/resources/models/frozen_inference_graph.pb";
    
    // 如果程序目录没有，尝试项目目录
    if (!QFile::exists(modelPath)) {
        modelPath = "/home/fylove/QtProject/QOMIPPlatform/resources/models/frozen_inference_graph.pb";
    }
    
    qDebug() << "Using external model file:" << modelPath;
    return modelPath;
}

QString ResourceExtractor::getDefaultConfigPath()
{
    // 使用MobileNet SSD v2配置文件，与v2模型匹配
    return "/home/fylove/QtProject/QOMIPPlatform/resources/models/ssd_mobilenet_v2_coco_2018_03_29.pbtxt";
}