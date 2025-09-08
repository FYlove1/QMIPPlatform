#ifndef COMMONUTILS_H
#define COMMONUTILS_H
#include <QString>
#include <QVector>
#include <QFileDialog>
#include <QDirIterator>
#include "opencv2/opencv.hpp"
//FileFuction
namespace CU {
inline QVector<QString> pickMediaFilesFromDir(
    const QString &caption = {}, QWidget *parent = nullptr)
{
    const QString dirPath = QFileDialog::getExistingDirectory(
        parent, caption, QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty())
        return {};

    const QStringList exts = {
        "*.jpg","*.jpeg","*.png","*.bmp","*.gif","*.webp","*.svg","*.ico",
        "*.mp4","*.avi","*.mkv","*.mov","*.wmv","*.flv"
    };

    QVector<QString> result;
    QDirIterator it(dirPath, exts, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
        result.append(it.next());
    return result;
}
inline QImage mat2QImage_deep(const cv::Mat& src)
{
    cv::Mat rgb;        // 临时 Mat，用来放转色后的数据
    switch (src.type()) {
    case CV_8UC1:
        return QImage(src.data, src.cols, src.rows,
                      static_cast<int>(src.step),
                      QImage::Format_Grayscale8).copy();   // copy() 深拷贝
    case CV_8UC3:
        cv::cvtColor(src, rgb, cv::COLOR_BGR2RGB);         // 1. BGR→RGB
        return QImage(rgb.data, rgb.cols, rgb.rows,
                      static_cast<int>(rgb.step),
                      QImage::Format_RGB888).copy();       // 2. 再拷贝
    case CV_8UC4:
        cv::cvtColor(src, rgb, cv::COLOR_BGRA2RGBA);       // 3. BGRA→RGBA
        return QImage(rgb.data, rgb.cols, rgb.rows,
                      static_cast<int>(rgb.step),
                      QImage::Format_ARGB32).copy();       // 4. 再拷贝
    default:
        return {};
    }
}

inline QImage matToQImage(const cv::Mat &src)
{
    if (src.empty()) return QImage();

    const int w = src.cols;
    const int h = src.rows;
    const int ch = src.channels();
    const int depth = src.depth();   // CV_8U / CV_16U / CV_32F
    const bool isBigEndian = QSysInfo::ByteOrder == QSysInfo::BigEndian;

    switch (depth)
    {
    case CV_8U:
        switch (ch)
        {
        case 1:  return QImage(src.data, w, h, static_cast<int>(src.step),
                          QImage::Format_Grayscale8);
        case 3:  return QImage(src.data, w, h, static_cast<int>(src.step),
                          QImage::Format_BGR888);

        default: break;
        }
        break;

    case CV_16U:
        switch (ch)
        {
        case 1:  return QImage(src.data, w, h, static_cast<int>(src.step),
                          QImage::Format_Grayscale16);
        case 3:  return QImage(src.data, w, h, static_cast<int>(src.step),
                          QImage::Format_RGB32);   // 16bit RGB
        case 4:  return QImage(src.data, w, h, static_cast<int>(src.step),
                          QImage::Format_RGBA64);  // 16bit RGBA
        default: break;
        }
        break;

    case CV_32F:
        // 32F 没有直接格式，转成 32F 灰度/彩色需手动归一化
        // 这里简单返回空，需要时自己扩展
        break;
    }

    // 兜底：转 8U 后再转 QImage
    cv::Mat tmp;
    if (depth == CV_32F)
    {
        cv::normalize(src, tmp, 0, 255, cv::NORM_MINMAX);
        tmp.convertTo(tmp, CV_8U);
    }
    else
    {
        src.convertTo(tmp, CV_8U);
    }

    if (tmp.channels() == 3)
        cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);

    return QImage(tmp.data, tmp.cols, tmp.rows, static_cast<int>(tmp.step),
                  tmp.channels() == 1 ? QImage::Format_Grayscale8
                                      : QImage::Format_RGB888).copy();
}

/**
 * 一步把 cv::Mat 变成 QPixmap，默认深拷贝
 */
inline QPixmap matToPixmap(const cv::Mat &src, bool deepCopy = true)
{
    QImage img = matToQImage(src);
    return deepCopy ? QPixmap::fromImage(img.copy())
                    : QPixmap::fromImage(img);
}

    // 算法ID常量
    enum AlgorithmId {
        ALGO_ORIGINAL = 0,       // 原始图像
        ALGO_GRAYSCALE = 1,      // 灰度处理
        ALGO_BLUR = 2,           // 高斯模糊
        ALGO_CANNY = 3,          // Canny边缘检测
        ALGO_THRESHOLD = 4,      // 二值化
        // 可以添加更多算法...
    };
    
    // 获取算法名称
    inline QString getAlgorithmName(int algorithmId) {
        switch (algorithmId) {
            case ALGO_ORIGINAL: return "原始图像";
            case ALGO_GRAYSCALE: return "灰度处理";
            case ALGO_BLUR: return "高斯模糊";
            case ALGO_CANNY: return "边缘检测";
            case ALGO_THRESHOLD: return "二值化";
            default: return QString("未知算法(%1)").arg(algorithmId);
        }
    }
    
    // 图像处理算法子命名空间
    namespace ImageAlgo {
        // 原始图像 - 无需处理
        inline cv::Mat original(const cv::Mat& input) {
            return input; // 无需深拷贝，直接返回引用
        }
        
        // 灰度处理
        inline cv::Mat grayscale(const cv::Mat& input) {
            if (input.channels() != 3) return input;
            
            cv::Mat result;
            cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
            cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
            return result;
        }
        
        // 高斯模糊
        inline cv::Mat blur(const cv::Mat& input, int kernelSize = 15) {
            // 确保内核大小是奇数
            if (kernelSize % 2 == 0) kernelSize++;
            
            cv::Mat result;
            cv::GaussianBlur(input, result, cv::Size(kernelSize, kernelSize), 0);
            return result;
        }
        
        // Canny边缘检测
        inline cv::Mat canny(const cv::Mat& input, int threshold1 = 50, int threshold2 = 150) {
            cv::Mat gray, result;
            
            if (input.channels() == 3) {
                cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
            } else {
                gray = input;
            }
            
            cv::Canny(gray, result, threshold1, threshold2);
            
            if (input.channels() == 3) {
                cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
            }
            
            return result;
        }
        
        // 二值化
        inline cv::Mat threshold(const cv::Mat& input, int thresh = 128, int maxVal = 255) {
            cv::Mat gray, result;
            
            if (input.channels() == 3) {
                cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
            } else {
                gray = input;
            }
            
            cv::threshold(gray, result, thresh, maxVal, cv::THRESH_BINARY);
            
            if (input.channels() == 3) {
                cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
            }
            
            return result;
        }
        
        // 这里可以添加更多算法...
    }
    
    // 主处理函数 - 简化为路由功能
    inline cv::Mat processImage(const cv::Mat& input, int algorithmId, const QVariantMap& params = QVariantMap()) {
        // 如果输入为空，返回空矩阵
        if (input.empty()) return cv::Mat();
        
        // 根据算法ID路由到相应的处理函数
        switch (algorithmId) {
            case ALGO_ORIGINAL:
                return ImageAlgo::original(input);
                
            case ALGO_GRAYSCALE:
                return ImageAlgo::grayscale(input);
                
            case ALGO_BLUR:
                return ImageAlgo::blur(input, params.value("kernelSize", 15).toInt());
                
            case ALGO_CANNY:
                return ImageAlgo::canny(
                    input,
                    params.value("threshold1", 50).toInt(),
                    params.value("threshold2", 150).toInt()
                );
                
            case ALGO_THRESHOLD:
                return ImageAlgo::threshold(
                    input,
                    params.value("threshold", 128).toInt(),
                    params.value("maxVal", 255).toInt()
                );
                
            default:
                return input; // 未知算法ID，返回原图
        }
    }
}

#endif // COMMONUTILS_H
