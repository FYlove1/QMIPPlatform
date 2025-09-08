#include "mainwindow.h"
#include "basicviewwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{


    
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // BasicViewWidget w;
    // w.show();
    // cv::Mat mat= cv::imread("/home/fylove/Pictures/4.jpg");


    // w.setImage(mat);
    return a.exec();
}
