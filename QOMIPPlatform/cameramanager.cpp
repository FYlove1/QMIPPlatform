#include "cameramanager.h"
#include <QDebug>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCamera>

CameraManager::CameraManager(QObject *parent)
    : QObject(parent)
{
}

CameraManager::~CameraManager()
{
}

QList<CameraManager::CameraInfo> CameraManager::scanCameras()
{
    m_cameras.clear();
    
    // 使用Qt6的新API获取摄像头列表
    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
    
    if (!availableCameras.isEmpty()) {
        int index = 0;
        for (const QCameraDevice &cameraDevice : availableCameras) {
            CameraInfo info;
            info.index = index++;
            info.name = cameraDevice.description();
            info.description = QString("ID: %1").arg(cameraDevice.id());
            info.isAvailable = true;
            
            // 测试摄像头是否真的可用
            if (testCamera(info.index)) {
                m_cameras.append(info);
                // 减少debug输出频率，只输出摄像头总数
            }
        }
    }
    
    // 如果Qt方法没找到摄像头，尝试OpenCV枚举（这部分代码保持不变）
    if (m_cameras.isEmpty()) {
        // qDebug() << "No cameras found via Qt, trying OpenCV enumeration...";
        
        // 尝试打开前10个摄像头索引
        for (int i = 0; i < 10; ++i) {
            if (testCamera(i)) {
                CameraInfo info;
                info.index = i;
                info.name = QString("Camera %1").arg(i);
                info.description = QString("OpenCV Camera Index %1").arg(i);
                info.isAvailable = true;
                m_cameras.append(info);
                // 减少OpenCV摄像头发现的debug输出
            }
        }
    }
    
    // 如果还是没有找到，添加默认摄像头
    if (m_cameras.isEmpty()) {
        CameraInfo defaultCamera;
        defaultCamera.index = 0;
        defaultCamera.name = "Default Camera";
        defaultCamera.description = "System default camera (index 0)";
        defaultCamera.isAvailable = false; // 标记为可能不可用
        m_cameras.append(defaultCamera);
        // qDebug() << "No cameras detected, adding default camera placeholder";
    }
    
    // 每100次调用输出一次统计
    static int scanCallCount = 0;
    scanCallCount++;
    if (scanCallCount % 100 == 0) {
        qDebug() << "摄像头扫描完成，找到" << m_cameras.size() << "个摄像头";
    }
    emit camerasUpdated(m_cameras);
    emit scanCompleted();
    
    return m_cameras;
}

bool CameraManager::testCamera(int index)
{
    cv::VideoCapture cap;
    
    // 尝试打开摄像头
    bool opened = false;
    
    // 在不同平台上使用不同的后端
#ifdef Q_OS_WIN
    // Windows上优先使用DirectShow
    opened = cap.open(index, cv::CAP_DSHOW);
    if (!opened) {
        opened = cap.open(index);
    }
#elif defined(Q_OS_MAC)
    // macOS上使用AVFoundation
    opened = cap.open(index, cv::CAP_AVFOUNDATION);
    if (!opened) {
        opened = cap.open(index);
    }
#elif defined(Q_OS_LINUX)
    // Linux上使用V4L2
    opened = cap.open(index, cv::CAP_V4L2);
    if (!opened) {
        opened = cap.open(index);
    }
#else
    // 其他平台使用默认后端
    opened = cap.open(index);
#endif
    
    if (opened) {
        // 尝试读取一帧来确认摄像头工作正常
        cv::Mat testFrame;
        bool canRead = cap.read(testFrame);
        cap.release();
        
        if (canRead && !testFrame.empty()) {
            // 摄像头测试成功，减少debug输出
            return true;
        }
    }
    
    return false;
}

QString CameraManager::getCameraName(int index)
{
    for (const CameraInfo& info : m_cameras) {
        if (info.index == index) {
            return info.name;
        }
    }
    return QString("Camera %1").arg(index);
}

void CameraManager::getSystemCameraInfo()
{
    // 这个函数可以扩展，用于获取更详细的系统摄像头信息
    // 比如通过系统API获取摄像头的硬件信息
    
#ifdef Q_OS_WIN
    // Windows特定的摄像头枚举代码
#elif defined(Q_OS_MAC)
    // macOS特定的摄像头枚举代码
#elif defined(Q_OS_LINUX)
    // Linux特定的摄像头枚举代码（比如通过v4l2）
#endif
}