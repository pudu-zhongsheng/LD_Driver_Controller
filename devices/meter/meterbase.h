#ifndef METERBASE_H
#define METERBASE_H

#include <QWidget>
#include "serial/serialutil.h"

class MeterBase : public QWidget
{
    Q_OBJECT
public:
    explicit MeterBase(QWidget *parent = nullptr);
    virtual ~MeterBase();

    virtual bool isConnected() const = 0;
    virtual void connectToPort(const QString &portName) = 0;
    virtual void disconnectPort() = 0;

signals:
    void serialConnected(const QString &portName);
    void serialDisconnected();
    void serialError(const QString &error);
    void measurementUpdated(float illuminance, float colorTemp, float r, float g, float b);

protected:
    SerialUtil *m_serial;
};

#endif // METERBASE_H 