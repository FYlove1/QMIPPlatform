#ifndef ALGORITHMITEMDELEGATE_H
#define ALGORITHMITEMDELEGATE_H

#include <QObject>
#include <QQuickWidget>
// 这个类目前没有实际功能，暂时不使用
class AlgorithmItemDelegate : public QQuickWidget
{
    Q_OBJECT
public:
    explicit AlgorithmItemDelegate(QWidget *parent = nullptr);

signals:
};

#endif // ALGORITHMITEMDELEGATE_H
