#include "detachedwindow.h"
#include "basicviewwidget.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QKeyEvent>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

DetachedWindow::DetachedWindow(BasicViewWidget *widget, const QString &title, int originalIndex, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint)
    , m_widget(widget)
    , m_title(title)
    , m_originalIndex(originalIndex)
    , m_isDragging(false)
{
    setWindowTitle(title);
    setAcceptDrops(true);
    
    // 设置窗口大小
    resize(800, 600);
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(2);
    
    // 创建标题栏（用于拖动）
    QWidget *titleBar = new QWidget(this);
    titleBar->setFixedHeight(30);
    titleBar->setStyleSheet("QWidget { background-color: #3498db; border-radius: 5px; }");
    titleBar->setCursor(Qt::OpenHandCursor);
    
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    
    QLabel *titleLabel = new QLabel(QString("🔗 %1 - 拖动此栏返回主窗口").arg(title), titleBar);
    titleLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    
    QPushButton *returnBtn = new QPushButton("返回主窗口", titleBar);
    returnBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; padding: 5px 10px; border-radius: 3px; }"
                            "QPushButton:hover { background-color: #27ae60; }");
    connect(returnBtn, &QPushButton::clicked, [this]() {
        emit returnToMainWindow(this);
    });
    titleLayout->addWidget(returnBtn);
    
    mainLayout->addWidget(titleBar);
    mainLayout->addWidget(m_widget, 1);
    
    // 设置widget的父窗口
    m_widget->setParent(this);
    m_widget->show();
    
    // 保存标题栏引用用于拖动
    m_titleBar = titleBar;
}

DetachedWindow::~DetachedWindow()
{
    // widget会被父窗口自动删除
}

void DetachedWindow::closeEvent(QCloseEvent *event)
{
    emit windowClosing(this);
    QWidget::closeEvent(event);
}

void DetachedWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_titleBar && m_titleBar->geometry().contains(event->pos())) {
        m_dragStartPosition = event->pos();
        m_isDragging = true;
        
        // 开始拖动操作
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        
        // 设置MIME数据，标识这是一个分离窗口
        mimeData->setData("application/x-detachedwindow", QByteArray::number((qintptr)this));
        drag->setMimeData(mimeData);
        
        // 创建拖动时的缩略图
        QPixmap pixmap(200, 100);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(QColor(52, 152, 219, 180)));
        painter.setPen(QPen(QColor(41, 128, 185), 2));
        painter.drawRoundedRect(pixmap.rect().adjusted(2, 2, -2, -2), 10, 10);
        painter.setPen(Qt::white);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, m_title);
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(100, 50));
        
        // 执行拖动
        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
        
        // 如果拖动成功，窗口会被返回到主窗口
        m_isDragging = false;
    }
    QWidget::mousePressEvent(event);
}

void DetachedWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 拖动操作已经由QDrag处理，这里不需要额外的处理
    QWidget::mouseMoveEvent(event);
}

void DetachedWindow::keyPressEvent(QKeyEvent *event)
{
    // 按ESC键返回主窗口
    if (event->key() == Qt::Key_Escape) {
        emit returnToMainWindow(this);
    }
    QWidget::keyPressEvent(event);
}