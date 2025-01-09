#include <QApplication>
#include "splash/splashscreen.h"
#include "menu/drivemenu.h"
#include "mainwindow.h"
#include <QTimer>
#include <QFile>
#include <QThread>

void loadStyleSheet(QApplication &app)
{
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 加载样式表
    loadStyleSheet(a);

    // 显示启动画面
    QPixmap pixmap(":/images/splash.png");
    SplashScreen splash(pixmap);
    splash.show();

    // 创建驱动选择菜单和主窗口
    DriveMenu *menu = new DriveMenu();
    MainWindow *mainWindow = nullptr;

    // 连接驱动选择菜单的信号
    QObject::connect(menu, &DriveMenu::enterMainWindow,
                    [&mainWindow, menu](const QString &driver, 
                                      const QString &load, 
                                      const QString &meter) {
        // 如果主窗口已存在，先删除
        if (mainWindow) {
            delete mainWindow;
        }

        // 创建新的主窗口
        mainWindow = new MainWindow(driver, load, meter);
        
        // 连接主窗口的返回信号
        QObject::connect(mainWindow, &MainWindow::backToMenu, 
                        [menu, &mainWindow]() {
            mainWindow->hide();
            delete mainWindow;
            mainWindow = nullptr;
            menu->show();
        });

        mainWindow->show();
        menu->hide();  // 隐藏菜单窗口
    });

    // 模拟加载过程
    for(int i = 0; i <= 100; i++) {
        splash.setProgress(i);
        a.processEvents();
        QThread::msleep(30);
    }
    
    menu->show();
    splash.finish(menu);

    return a.exec();
}
