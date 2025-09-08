#include "algorithmlistmodel.h"
#include "Algorithms/algorithmfactory.h"

AlgorithmListModel::AlgorithmListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AlgorithmListModel::~AlgorithmListModel()
{
    // 清理所有算法对象
    QWriteLocker locker(&m_lock);
    qDeleteAll(m_algorithms);
    m_algorithms.clear();
}

int AlgorithmListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    QReadLocker locker(&m_lock);
    return m_algorithms.size();
}

QVariant AlgorithmListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    QReadLocker locker(&m_lock);
    
    if (index.row() >= m_algorithms.size() || !m_algorithms[index.row()])
        return QVariant();
    
    Algorithm* algorithm = m_algorithms[index.row()];
    
    switch (role) {
        case Qt::DisplayRole:
        case NameRole:
            return algorithm->getName();
            
        case IdRole:
            return algorithm->getId();
            
        case ParamsRole:
            return algorithm->getParameters();
            
        case DescriptionRole:
            return algorithm->getDescription();
            
        case ParamsMetaRole:
            {
                // 转换参数元数据为QVariantList以便QML使用
                QVariantList metaList;
                const QList<ParameterMeta> paramsMeta = algorithm->getParametersMeta();
                for (const ParameterMeta& meta : paramsMeta) {
                    QVariantMap metaMap;
                    metaMap["name"] = meta.name;
                    metaMap["displayName"] = meta.displayName;
                    metaMap["description"] = meta.description;
                    metaMap["type"] = static_cast<int>(meta.type);
                    metaMap["defaultValue"] = meta.defaultValue;
                    metaMap["minValue"] = meta.minValue;
                    metaMap["maxValue"] = meta.maxValue;
                    if (!meta.enumOptions.isEmpty()) {
                        metaMap["enumOptions"] = meta.enumOptions;
                    }
                    metaList.append(metaMap);
                }
                return metaList; // 添加这一行返回元数据列表
            }
    }
    
    return QVariant();
}

bool AlgorithmListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != ParamsRole)
        return false;
    
    QWriteLocker locker(&m_lock);
    
    if (index.row() >= m_algorithms.size() || !m_algorithms[index.row()])
        return false;
    
    Algorithm* algorithm = m_algorithms[index.row()];
    
    // 仅支持更新参数
    if (role == ParamsRole && value.canConvert<QVariantMap>()) {
        algorithm->setParameters(value.toMap());
        emit dataChanged(index, index, {role});
        return true;
    }
    
    return false;
}

void AlgorithmListModel::addAlgorithm(int algorithmId, const QString &name, const QVariantMap &params)
{
    // 使用工厂创建算法实例
    Algorithm* algorithm = AlgorithmFactory::instance().createAlgorithm(algorithmId);
    if (!algorithm) {
        return;  // 无法创建算法
    }
    
    // 设置参数
    algorithm->setParameters(params);
    
    // 获取插入位置
    int insertPosition;
    {
        QReadLocker readLock(&m_lock);
        insertPosition = m_algorithms.size();
    }
    
    // 开始插入行
    beginInsertRows(QModelIndex(), insertPosition, insertPosition);
    
    // 数据修改
    {
        QWriteLocker writeLock(&m_lock);
        m_algorithms.append(algorithm);
    }
    
    // 结束插入
    endInsertRows();
}

bool AlgorithmListModel::removeAlgorithm(int index)
{
    QWriteLocker locker(&m_lock);
    
    if (index < 0 || index >= m_algorithms.size())
        return false;
    
    beginRemoveRows(QModelIndex(), index, index);
    
    // 删除算法对象并从列表中移除
    delete m_algorithms[index];
    m_algorithms.removeAt(index);
    
    endRemoveRows();
    return true;
}

void AlgorithmListModel::clearAlgorithms()
{
    QWriteLocker locker(&m_lock);
    
    if (m_algorithms.isEmpty())
        return;
    
    beginResetModel();
    
    // 删除所有算法对象
    qDeleteAll(m_algorithms);
    m_algorithms.clear();
    
    endResetModel();
}

int AlgorithmListModel::getAlgorithmId(int row) const
{
    QReadLocker locker(&m_lock);
    
    if (row < 0 || row >= m_algorithms.size() || !m_algorithms[row])
        return -1;
    
    return m_algorithms[row]->getId();
}

QString AlgorithmListModel::getAlgorithmName(int row) const
{
    QReadLocker locker(&m_lock);
    
    if (row < 0 || row >= m_algorithms.size() || !m_algorithms[row])
        return QString();
    
    return m_algorithms[row]->getName();
}

QVariantMap AlgorithmListModel::getAlgorithmParams(int row) const
{
    QReadLocker locker(&m_lock);
    
    if (row < 0 || row >= m_algorithms.size() || !m_algorithms[row])
        return QVariantMap();
    
    return m_algorithms[row]->getParameters();
}

bool AlgorithmListModel::updateAlgorithmParameters(int index, const QVariantMap &parameters)
{
    qDebug() << "============ [CPP-AlgorithmListModel] 参数更新流程开始 ============";
    qDebug() << "[CPP-STEP1] 接收到的索引:" << index;
    qDebug() << "[CPP-STEP2] 接收到的参数:";
    
    // 输出接收到的每个参数
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        qDebug() << "[CPP-RECV-PARAM]" << it.key() << ":" << it.value() 
                 << "(类型:" << it.value().typeName() << ")";
    }
    
    // 首先检查索引范围，避免不必要的锁获取
    if (index < 0 || index >= rowCount()) {
        qDebug() << "[CPP-ERROR] 索引超出范围: index=" << index << ", rowCount=" << rowCount();
        return false;
    }
    
    Algorithm* algorithm = nullptr;
    QModelIndex modelIndex;
    
    {
        // 缩小锁的范围，只在必要时锁定
        QWriteLocker locker(&m_lock);
        
        if (index >= m_algorithms.size() || !m_algorithms[index]) {
            qDebug() << "[CPP-ERROR] 算法对象无效或不存在";
            return false;
        }
        
        // 获取算法指针
        algorithm = m_algorithms[index];
        modelIndex = createIndex(index, 0);
        
        // 输出更新前的参数
        qDebug() << "[CPP-STEP3] 更新前的参数:";
        QVariantMap oldParams = algorithm->getParameters();
        for (auto it = oldParams.begin(); it != oldParams.end(); ++it) {
            qDebug() << "[CPP-OLD-PARAM]" << it.key() << ":" << it.value();
        }
    }
    
    // 锁外更新参数
    try {
        algorithm->setParameters(parameters);
        
        // 获取并输出更新后的参数
        QVariantMap newParams = algorithm->getParameters();
        qDebug() << "[CPP-STEP4] 更新后的参数:";
        for (auto it = newParams.begin(); it != newParams.end(); ++it) {
            qDebug() << "[CPP-NEW-PARAM]" << it.key() << ":" << it.value();
        }
        
        // 发送数据变更信号
        emit dataChanged(modelIndex, modelIndex, {ParamsRole});
        qDebug() << "[CPP-STEP5] dataChanged信号已发送";
        qDebug() << "============ [CPP-AlgorithmListModel] 参数更新流程结束 ============";

        return true;
    }
    catch (const std::exception& e) {
        qWarning() << "[CPP-ERROR] 更新参数时发生异常:" << e.what();
        return false;
    }
    catch (...) {
        qWarning() << "[CPP-ERROR] 更新参数时发生未知异常";
        return false;
    }
}

QVector<Algorithm*> AlgorithmListModel::getAllAlgorithms() const
{
    // 先复制算法指针列表，减少持锁时间
    QVector<Algorithm*> algorithmPtrs;
    {
        QReadLocker locker(&m_lock);
        algorithmPtrs = m_algorithms;  // 浅拷贝指针
    }
    
    // 在锁外进行clone操作，避免死锁
    QVector<Algorithm*> result;
    for (Algorithm* algorithm : algorithmPtrs) {
        if (algorithm) {
            result.append(algorithm->clone());
        }
    }
    
    return result;
}

QVariantMap AlgorithmListModel::getAlgorithmInfo(int index) const
{
    QVariantMap info;
    
    // 使用读锁保护数据访问
    QReadLocker locker(&m_lock);
    
    if (index < 0 || index >= m_algorithms.size() || !m_algorithms[index])
        return info; // 返回空map
    
    Algorithm* algorithm = m_algorithms[index];
    
    // 填充基本信息
    info["algorithmId"] = algorithm->getId();
    info["name"] = algorithm->getName();
    info["description"] = algorithm->getDescription();
    info["params"] = algorithm->getParameters();
    
    // 处理参数元数据
    QVariantList metaList;
    const QList<ParameterMeta> paramsMeta = algorithm->getParametersMeta();
    for (const ParameterMeta& meta : paramsMeta) {
        QVariantMap metaMap;
        metaMap["name"] = meta.name;
        metaMap["displayName"] = meta.displayName;
        metaMap["description"] = meta.description;
        metaMap["type"] = static_cast<int>(meta.type);
        metaMap["defaultValue"] = meta.defaultValue;
        if (meta.minValue.isValid()) metaMap["minValue"] = meta.minValue;
        if (meta.maxValue.isValid()) metaMap["maxValue"] = meta.maxValue;
        if (!meta.enumOptions.isEmpty()) {
            metaMap["enumOptions"] = meta.enumOptions;
        }
        metaList.append(metaMap);
    }
    info["paramsMeta"] = metaList;
    
    return info;
}

// 实现flags方法
Qt::ItemFlags AlgorithmListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
        
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

// 实现roleNames方法
QHash<int, QByteArray> AlgorithmListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "algorithmId";
    roles[NameRole] = "name";
    roles[ParamsRole] = "params";
    roles[DescriptionRole] = "description";
    roles[ParamsMetaRole] = "paramsMeta";
    return roles;
}
