#ifndef LOAD_BASE_H
#define LOAD_BASE_H

#include <QWidget>
#include "../../serial/serialutil.h"

class LoadBase : public QWidget
{
    Q_OBJECT
public:
    explicit LoadBase(QWidget *parent = nullptr);
    virtual ~LoadBase();

    virtual bool connectToPort(const QString &portName) = 0;
    virtual void disconnectPort() = 0;
    virtual bool isConnected() const = 0;
    virtual QJsonObject saveSettings() const = 0;
    virtual void loadSettings(const QJsonObject &settings) = 0;

signals:
    void serialConnected(const QString &portName);
    void serialDisconnected();
    void serialError(const QString &error);
    void statusUpdated(float voltage, float current, float power);

protected:
    SerialUtil *m_serial = nullptr;
};

#endif // LOAD_BASE_H 
