#include "load_base.h"

LoadBase::LoadBase(QWidget *parent) : QWidget(parent)
{
}

LoadBase::~LoadBase()
{
}

void LoadBase::connectToPort(const QString &portName)
{
    if (m_serial && m_serial->connectToPort(portName)) {
        emit serialConnected(portName);
    }
}

void LoadBase::disconnectPort()
{
    if (m_serial) {
        m_serial->disconnectPort();
        emit serialDisconnected();
    }
}

bool LoadBase::isConnected() const
{
    return m_serial && m_serial->isConnected();
}

void LoadBase::error(const QString &msg)
{
    emit serialError(msg);
}