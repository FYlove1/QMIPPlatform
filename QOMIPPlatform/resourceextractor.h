#ifndef RESOURCEEXTRACTOR_H
#define RESOURCEEXTRACTOR_H

#include <QString>

class ResourceExtractor
{
public:
    // 从Qt资源中提取文件到临时目录
    static QString extractResource(const QString& resourcePath);
    
    // 清理临时提取的文件
    static void cleanupTempFiles();
    
    // 获取MobileNet SSD默认模型路径
    static QString getDefaultModelPath();
    static QString getDefaultConfigPath();
    
private:
    static QString getTempDir();
    static QStringList m_tempFiles;
};

#endif // RESOURCEEXTRACTOR_H