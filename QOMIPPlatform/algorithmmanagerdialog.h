#ifndef ALGORITHMMANAGERDIALOG_H
#define ALGORITHMMANAGERDIALOG_H

#include <QDialog>
//暂时不使用
namespace Ui {
class AlgorithmManagerDialog;
}

class AlgorithmManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AlgorithmManagerDialog(QWidget *parent = nullptr);
    ~AlgorithmManagerDialog();

private:
    Ui::AlgorithmManagerDialog *ui;
};

#endif // ALGORITHMMANAGERDIALOG_H
