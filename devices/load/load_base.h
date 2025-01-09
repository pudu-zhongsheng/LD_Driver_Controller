#ifndef LOAD_BASE_H
#define LOAD_BASE_H

#include <QWidget>
#include "../../serial/serialutil.h"

class LoadBase : public QWidget
{
    Q_OBJECT
public:
    explicit LoadBase(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~LoadBase() {}

    virtual void connectToPort(const QString &portName) = 0;
    virtual void disconnectPort() = 0;
    virtual bool isConnected() const = 0;

signals:
    void serialConnected(const QString &portName);
    void serialDisconnected();
    void serialError(const QString &msg);
    void statusUpdated();

protected:
    SerialUtil *m_serial = nullptr;
};

#endif // LOAD_BASE_H 