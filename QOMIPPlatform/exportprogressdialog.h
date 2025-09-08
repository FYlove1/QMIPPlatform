#ifndef EXPORTPROGRESSDIALOG_H
#define EXPORTPROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

class ExportProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportProgressDialog(QWidget *parent = nullptr);
    
    void setMaximum(int maximum);
    void setValue(int value);
    void setCurrentFile(const QString &fileName);
    void setExportComplete();

signals:
    void cancelRequested();

private slots:
    void onCancelClicked();

private:
    void setupUI();

    QLabel *m_statusLabel;
    QLabel *m_fileLabel;
    QProgressBar *m_progressBar;
    QPushButton *m_cancelButton;
    bool m_cancelled;
};

#endif // EXPORTPROGRESSDIALOG_H