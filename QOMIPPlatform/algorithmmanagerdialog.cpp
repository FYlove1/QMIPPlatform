#include "algorithmmanagerdialog.h"
#include "ui_algorithmmanagerdialog.h"

AlgorithmManagerDialog::AlgorithmManagerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AlgorithmManagerDialog)
{
    ui->setupUi(this);
}

AlgorithmManagerDialog::~AlgorithmManagerDialog()
{
    delete ui;
}
