#pragma once
#include <QThread>
#include <QMutex>
#include <opencv2/opencv.hpp>

class Worker : public QThread {
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    void setSource(const QString &file);
    void play();
    void pause();
    void stop();

signals:
    void frameReady(const cv::Mat &);   // 回传帧

protected:
    void run() override;

private:
    QString m_path;
    QMutex  m_mutex;
    bool    m_running = true;
    bool    m_play    = false;
    cv::VideoCapture m_cap;
};
