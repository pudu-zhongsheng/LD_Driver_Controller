#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    explicit SplashScreen(const QPixmap &pixmap = QPixmap(), QWidget *parent = nullptr);
    ~SplashScreen();

    void setProgress(int value);
    void finish(QWidget *mainWindow);

private:
    QProgressBar *m_progressBar;
    QTimer *m_timer;
    int m_progress;

private slots:
    void updateProgress();
};

#endif // SPLASHSCREEN_H 