#pragma once

#include <QDialog>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "frameprocessor.h"

class AlgorithmDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit AlgorithmDialog(FrameProcessor* processor, QWidget* parent = nullptr);

private slots:
    void onAddAlgorithm();
    void onAlgorithmDeleted(int index);
    void onAlgorithmParamsChanged(int index, const QVariant& params);
    
private:
    FrameProcessor* m_processor;
    QQuickWidget* m_quickWidget;
    QPushButton* m_addButton;
    QPushButton* m_closeButton;
    
    void setupUI();
};
