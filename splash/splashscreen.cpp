#include "splashscreen.h"
#include <QVBoxLayout>
#include <QApplication>

SplashScreen::SplashScreen(const QPixmap &pixmap, QWidget *parent)
    : QSplashScreen(parent, pixmap)
    , m_progress(0)
{
    // 创建进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: 2px solid grey;"
        "   border-radius: 5px;"
        "   text-align: center;"
        "   background-color: #f0f0f0;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #4CAF50;"
        "   width: 10px;"
        "   margin: 0.5px;"
        "}"
    );
    m_progressBar->setFixedSize(200, 20);
    m_progressBar->move((pixmap.width() - m_progressBar->width()) / 2,
                       pixmap.height() - m_progressBar->height() - 20);

    // 创建定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SplashScreen::updateProgress);
    m_timer->start(30); // 每30ms更新一次
}

SplashScreen::~SplashScreen()
{
    delete m_progressBar;
    delete m_timer;
}

void SplashScreen::setProgress(int value)
{
    m_progress = value;
    m_progressBar->setValue(value);
    
    // 显示加载信息
    QString message;
    if (value < 33) {
        message = "正在初始化...";
    } else if (value < 66) {
        message = "正在加载配置...";
    } else {
        message = "即将完成...";
    }
    showMessage(message, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
}

void SplashScreen::updateProgress()
{
    if (m_progress < 100) {
        setProgress(m_progress + 1);
    } else {
        m_timer->stop();
    }
}

void SplashScreen::finish(QWidget *mainWindow)
{
    m_timer->stop();
    QSplashScreen::finish(mainWindow);
} 