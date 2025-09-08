#pragma once
#include <opencv2/opencv.hpp>
#include <QString>
#include <QVariantMap>
#include <QList>

// 参数类型枚举
enum class ParamType {
    Int,
    Double,
    String,
    Bool,
    Enum
};

// 参数元数据结构
struct ParameterMeta {
    QString name;           // 参数名称
    QString displayName;    // 显示名称
    QString description;    // 参数描述
    ParamType type;         // 参数类型
    QVariant defaultValue;  // 默认值
    QVariant minValue;      // 最小值 (用于数值类型)
    QVariant maxValue;      // 最大值 (用于数值类型)
    QStringList enumOptions; // 枚举选项 (用于枚举类型)
};

class Algorithm {
public:
    virtual ~Algorithm() = default;
    
    // 处理图像的核心方法
    virtual cv::Mat process(const cv::Mat& input) = 0;
    
    // 设置算法参数
    virtual void setParameters(const QVariantMap& params) = 0;
    
    // 获取算法参数
    virtual QVariantMap getParameters() const = 0;
    
    // 获取算法名称
    virtual QString getName() const = 0;
    
    // 获取算法描述
    virtual QString getDescription() const = 0;
    
    // 获取算法唯一标识符
    virtual int getId() const = 0;
    
    // 获取参数元数据列表
    virtual QList<ParameterMeta> getParametersMeta() const = 0;
    
    // 创建算法的深拷贝
    virtual Algorithm* clone() const = 0;
};
