#pragma once

#include <QObject>
#include <QList>
#include <QPair>
#include <opencv2/opencv.hpp>

/**
 * @class CameraManager
 * @brief 摄像头管理类，负责枚举系统中的摄像头设备
 */
class CameraManager : public QObject {
    Q_OBJECT
    
public:
    struct CameraInfo {
        int index;           // 摄像头索引
        QString name;        // 摄像头名称
        QString description; // 摄像头描述
        bool isAvailable;    // 是否可用
    };
    
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();
    
    /**
     * @brief 扫描系统中所有可用的摄像头
     * @return 返回摄像头列表
     */
    QList<CameraInfo> scanCameras();
    
    /**
     * @brief 测试摄像头是否可用
     * @param index 摄像头索引
     * @return 是否可用
     */
    bool testCamera(int index);
    
    /**
     * @brief 获取摄像头名称
     * @param index 摄像头索引
     * @return 摄像头名称
     */
    QString getCameraName(int index);
    
    /**
     * @brief 获取当前可用的摄像头列表
     */
    const QList<CameraInfo>& getAvailableCameras() const { return m_cameras; }
    
signals:
    /**
     * @brief 摄像头列表更新信号
     */
    void camerasUpdated(const QList<CameraInfo>& cameras);
    
    /**
     * @brief 扫描完成信号
     */
    void scanCompleted();
    
private:
    QList<CameraInfo> m_cameras;
    
    /**
     * @brief 获取系统摄像头信息（跨平台）
     */
    void getSystemCameraInfo();
};