#ifndef DETACHEDWINDOW_H
#define DETACHEDWINDOW_H

#include <QWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QMimeData>

class BasicViewWidget;

class DetachedWindow : public QWidget
{
    Q_OBJECT

public:
    explicit DetachedWindow(BasicViewWidget *widget, const QString &title, int originalIndex, QWidget *parent = nullptr);
    ~DetachedWindow();
    
    BasicViewWidget* getWidget() const { return m_widget; }
    int getOriginalIndex() const { return m_originalIndex; }
    QString getTitle() const { return m_title; }
    
signals:
    void windowClosing(DetachedWindow *window);
    void returnToMainWindow(DetachedWindow *window);
    
protected:
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    
private:
    BasicViewWidget *m_widget;
    QString m_title;
    int m_originalIndex;
    QPoint m_dragStartPosition;
    bool m_isDragging;
    QWidget *m_titleBar;
};

#endif // DETACHEDWINDOW_H