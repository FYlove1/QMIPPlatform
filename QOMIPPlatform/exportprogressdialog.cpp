#include "exportprogressdialog.h"
#include <QApplication>

ExportProgressDialog::ExportProgressDialog(QWidget *parent)
    : QDialog(parent)
    , m_cancelled(false)
{
    setupUI();
    setWindowModality(Qt::ApplicationModal);  // 设置为应用程序模态
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void ExportProgressDialog::setupUI()
{
    setWindowTitle("导出视频");
    setFixedSize(400, 150);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 状态标签
    m_statusLabel = new QLabel("正在导出视频...", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(m_statusLabel);

    // 当前文件标签
    m_fileLabel = new QLabel("", this);
    m_fileLabel->setAlignment(Qt::AlignCenter);
    m_fileLabel->setStyleSheet("font-size: 12px; color: #666;");
    m_fileLabel->setWordWrap(true);
    mainLayout->addWidget(m_fileLabel);

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: 1px solid #ccc;"
        "   border-radius: 4px;"
        "   text-align: center;"
        "   height: 20px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #4CAF50;"
        "   border-radius: 3px;"
        "}"
    );
    mainLayout->addWidget(m_progressBar);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("取消导出", this);
    m_cancelButton->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 20px;"
        "   background-color: #f44336;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d32f2f;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #c62828;"
        "}"
    );
    connect(m_cancelButton, &QPushButton::clicked, this, &ExportProgressDialog::onCancelClicked);

    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void ExportProgressDialog::setMaximum(int maximum)
{
    m_progressBar->setMaximum(maximum);
}

void ExportProgressDialog::setValue(int value)
{
    m_progressBar->setValue(value);
    QApplication::processEvents(); // 更新UI
}

void ExportProgressDialog::setCurrentFile(const QString &fileName)
{
    m_fileLabel->setText(QString("当前文件: %1").arg(fileName));
    QApplication::processEvents(); // 更新UI
}

void ExportProgressDialog::setExportComplete()
{
    m_statusLabel->setText("导出完成！");
    m_cancelButton->setText("关闭");
    m_cancelButton->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 20px;"
        "   background-color: #4CAF50;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3d8b40;"
        "}"
    );
    QApplication::processEvents(); // 更新UI
}

void ExportProgressDialog::onCancelClicked()
{
    if (m_statusLabel->text() == "导出完成！") {
        accept(); // 完成时关闭对话框
    } else {
        m_cancelled = true;
        m_statusLabel->setText("正在取消...");
        m_cancelButton->setEnabled(false);
        emit cancelRequested();
    }
}