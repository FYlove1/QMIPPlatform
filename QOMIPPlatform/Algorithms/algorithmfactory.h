#pragma once
#include "algorithm.h"
#include <QMap>
#include <functional>
#include <memory>

class AlgorithmFactory {
public:
    // 获取单例实例
    static AlgorithmFactory& instance();
    
    // 注册算法创建函数
    void registerAlgorithm(int id, std::function<Algorithm*()> creator);
    
    // 创建算法实例
    Algorithm* createAlgorithm(int id);
    
    // 获取所有已注册算法ID
    QList<int> getRegisteredAlgorithmIds() const;
    
    // 获取所有算法信息（ID和名称对）
    QList<QPair<int, QString>> getAlgorithmInfoList() const;
    
    // 检查算法是否已注册
    bool isAlgorithmRegistered(int id) const;
    
private:
    // 私有构造函数（单例模式）
    AlgorithmFactory();
    
    // 存储算法创建函数的映射
    QMap<int, std::function<Algorithm*()>> m_creators;
};