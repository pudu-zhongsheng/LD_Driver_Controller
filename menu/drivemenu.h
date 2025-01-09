#ifndef DRIVEMENU_H
#define DRIVEMENU_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class DriveMenu : public QWidget
{
    Q_OBJECT
public:
    explicit DriveMenu(QWidget *parent = nullptr);
    ~DriveMenu();

signals:
    void enterMainWindow(const QString &driver, const QString &load, const QString &meter);

private:
    // UI组件
    QLabel *m_titleLabel;
    QComboBox *m_driverSelect;
    QComboBox *m_loadSelect;
    QComboBox *m_meterSelect;
    QPushButton *m_enterButton;

    void initUI();
    void initConnections();
    void setupStyles();
};

#endif // DRIVEMENU_H 