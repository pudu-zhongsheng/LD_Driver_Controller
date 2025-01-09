#ifndef DRIVERINFOWIDGET_H
#define DRIVERINFOWIDGET_H

#include <QWidget>
#include <QLineEdit>

class DriverInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DriverInfoWidget(QWidget *parent = nullptr);

    QString getCustomer() const;
    QString getDriverCode() const;
    QString getTestCode() const;

    void setCustomer(const QString &customer);
    void setDriverCode(const QString &code);
    void setTestCode(const QString &code);

signals:
    void infoChanged();

private:
    QLineEdit *m_customerEdit;    // 客户
    QLineEdit *m_driverCodeEdit;  // 驱动编码
    QLineEdit *m_testCodeEdit;    // 测试编码

    void initUI();
    void initConnections();
};

#endif // DRIVERINFOWIDGET_H 