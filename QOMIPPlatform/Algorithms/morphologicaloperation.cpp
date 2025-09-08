#include "morphologicaloperation.h"

MorphologicalOperation::MorphologicalOperation() 
    : m_operation(0), m_kernelSize(3), m_kernelShape(0), m_iterations(1) {
}

cv::Mat MorphologicalOperation::process(const cv::Mat& input) {
    cv::Mat output;
    
    int shape = (m_kernelShape == 0) ? cv::MORPH_RECT : 
                (m_kernelShape == 1) ? cv::MORPH_CROSS : cv::MORPH_ELLIPSE;
    
    cv::Mat kernel = cv::getStructuringElement(shape, cv::Size(m_kernelSize, m_kernelSize));
    
    int op;
    switch(m_operation) {
        case 0: op = cv::MORPH_ERODE; break;
        case 1: op = cv::MORPH_DILATE; break;
        case 2: op = cv::MORPH_OPEN; break;
        case 3: op = cv::MORPH_CLOSE; break;
        case 4: op = cv::MORPH_GRADIENT; break;
        case 5: op = cv::MORPH_TOPHAT; break;
        case 6: op = cv::MORPH_BLACKHAT; break;
        default: op = cv::MORPH_ERODE;
    }
    
    cv::morphologyEx(input, output, op, kernel, cv::Point(-1,-1), m_iterations);
    
    return output;
}

void MorphologicalOperation::setParameters(const QVariantMap& params) {
    if (params.contains("operation")) {
        m_operation = params["operation"].toInt();
    }
    if (params.contains("kernelSize")) {
        m_kernelSize = params["kernelSize"].toInt();
        if (m_kernelSize < 1) m_kernelSize = 1;
        if (m_kernelSize > 21) m_kernelSize = 21;
    }
    if (params.contains("kernelShape")) {
        m_kernelShape = params["kernelShape"].toInt();
    }
    if (params.contains("iterations")) {
        m_iterations = params["iterations"].toInt();
        if (m_iterations < 1) m_iterations = 1;
        if (m_iterations > 10) m_iterations = 10;
    }
}

QVariantMap MorphologicalOperation::getParameters() const {
    QVariantMap params;
    params["operation"] = m_operation;
    params["kernelSize"] = m_kernelSize;
    params["kernelShape"] = m_kernelShape;
    params["iterations"] = m_iterations;
    return params;
}

QString MorphologicalOperation::getName() const {
    return "形态学操作";
}

QString MorphologicalOperation::getDescription() const {
    return "执行形态学操作（腐蚀、膨胀、开运算、闭运算等）。\n"
           "参数需求：\n"
           "- operation (枚举): 操作类型，0-腐蚀，1-膨胀，2-开运算，3-闭运算，4-梯度，5-顶帽，6-黑帽，默认值 0\n"
           "- kernelSize (整数): 结构元素的大小，范围 1-21，默认值 3\n"
           "- kernelShape (枚举): 结构元素的形状，0-矩形，1-十字形，2-椭圆形，默认值 0\n"
           "- iterations (整数): 操作的迭代次数，范围 1-10，默认值 1";
}

int MorphologicalOperation::getId() const {
    return 9;
}

QList<ParameterMeta> MorphologicalOperation::getParametersMeta() const {
    QList<ParameterMeta> metaList;
    
    ParameterMeta operationMeta;
    operationMeta.name = "operation";
    operationMeta.displayName = "操作类型";
    operationMeta.description = "形态学操作类型";
    operationMeta.type = ParamType::Enum;
    operationMeta.defaultValue = 0;
    operationMeta.enumOptions = QStringList() << "腐蚀" << "膨胀" << "开运算" << "闭运算" 
                                              << "梯度" << "顶帽" << "黑帽";
    metaList.append(operationMeta);
    
    ParameterMeta kernelSizeMeta;
    kernelSizeMeta.name = "kernelSize";
    kernelSizeMeta.displayName = "核大小";
    kernelSizeMeta.description = "结构元素的大小";
    kernelSizeMeta.type = ParamType::Int;
    kernelSizeMeta.defaultValue = 3;
    kernelSizeMeta.minValue = 1;
    kernelSizeMeta.maxValue = 21;
    metaList.append(kernelSizeMeta);
    
    ParameterMeta kernelShapeMeta;
    kernelShapeMeta.name = "kernelShape";
    kernelShapeMeta.displayName = "核形状";
    kernelShapeMeta.description = "结构元素的形状";
    kernelShapeMeta.type = ParamType::Enum;
    kernelShapeMeta.defaultValue = 0;
    kernelShapeMeta.enumOptions = QStringList() << "矩形" << "十字形" << "椭圆形";
    metaList.append(kernelShapeMeta);
    
    ParameterMeta iterationsMeta;
    iterationsMeta.name = "iterations";
    iterationsMeta.displayName = "迭代次数";
    iterationsMeta.description = "操作的迭代次数";
    iterationsMeta.type = ParamType::Int;
    iterationsMeta.defaultValue = 1;
    iterationsMeta.minValue = 1;
    iterationsMeta.maxValue = 10;
    metaList.append(iterationsMeta);
    
    return metaList;
}

Algorithm* MorphologicalOperation::clone() const {
    MorphologicalOperation* copy = new MorphologicalOperation();
    copy->m_operation = this->m_operation;
    copy->m_kernelSize = this->m_kernelSize;
    copy->m_kernelShape = this->m_kernelShape;
    copy->m_iterations = this->m_iterations;
    return copy;
}