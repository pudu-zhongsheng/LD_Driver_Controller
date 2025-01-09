#ifndef TOASTMESSAGE_H
#define TOASTMESSAGE_H

#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QPropertyAnimation>   // 动画

class ToastMessage : public QLabel{
    Q_OBJECT
public:
    explicit ToastMessage(const QString &message, QWidget *parent = nullptr)
        :QLabel(parent){
        setText(message);
        setAlignment(Qt::AlignCenter);
        setStyleSheet("background-color: rgba(0, 0, 0, 180);"
                      "color: white; "
                      "border-radius: 10px;"
                      "padding: 10px;");
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
        adjustSize();
        // 获取父窗口的全局位置
        QPoint parentPos;
        parentPos = parent->mapToGlobal(QPoint(0,0));
        // 在父窗口中心显示
        move(parentPos.x() + (parent->width() - width()) / 2,
             parentPos.y() + (parent->height() - height()) / 2);

    }

    void showToast(int timeout = 3000){
        show();
        QTimer::singleShot(timeout,this,&ToastMessage::fadeOut);
    }

private slots:
    void fadeOut(){
        auto *animation = new QPropertyAnimation(this,"windowOpacity");
        animation->setDuration(1000);   // 淡出持续时间
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        connect(animation,&QPropertyAnimation::finished,this,&ToastMessage::close);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
};


#endif // TOASTMESSAGE_H
