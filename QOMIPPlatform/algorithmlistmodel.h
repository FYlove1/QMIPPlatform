#pragma once

#include <QAbstractListModel>
#include <QVariantMap>
#include <QVector>
#include <QReadWriteLock>
#include <QListView>
#include "Algorithms/algorithm.h" // 必须包含此头文件

class AlgorithmListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // 定义数据角色
    enum Roles {
        IdRole = Qt::UserRole + 1,   // 算法ID
        NameRole,                    // 算法名称
        ParamsRole,                  // 算法参数
        DescriptionRole,             // 算法描述
        ParamsMetaRole               // 参数元数据 - 新增
    };
    
    explicit AlgorithmListModel(QObject *parent = nullptr);
    ~AlgorithmListModel(); // 声明析构函数
    
    // QAbstractListModel必须实现的方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // 添加新算法
    void addAlgorithm(int algorithmId, const QString &name = QString(), 
                      const QVariantMap &params = QVariantMap());
    
    // 移除算法
    bool removeAlgorithm(int index);
    
    // 清空所有算法
    void clearAlgorithms();
    
    // 获取特定行的算法信息
    int getAlgorithmId(int row) const;
    QString getAlgorithmName(int row) const;
    QVariantMap getAlgorithmParams(int row) const;
    
    // 更新算法参数
   // bool updateAlgorithmParams(int row, const QVariantMap &params);
    
    // 获取所有算法 - 修改返回类型
    QVector<Algorithm*> getAllAlgorithms() const;
    
    // QML可调用的安全方法
    Q_INVOKABLE QVariantMap getAlgorithmInfo(int index) const;
    Q_INVOKABLE bool updateAlgorithmParameters(int index, const QVariantMap &parameters);

private:
    // 直接存储算法指针，不再使用结构体
    QVector<Algorithm*> m_algorithms;   // 算法列表
    mutable QReadWriteLock m_lock;      // 读写锁
};