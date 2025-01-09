#include "load_base.h"

LoadBase::LoadBase(QWidget *parent) : QWidget(parent)
{
}

LoadBase::~LoadBase()
{
    if (m_serial) {
        if (m_serial->isConnected()) {
            m_serial->disconnectPort();
        }
        delete m_serial;
        m_serial = nullptr;
    }
}

bool LoadBase::connectToPort(const QString &portName)
{
    if (m_serial && m_serial->connectToPort(portName)) {
        emit serialConnected(portName);
        return true;
    }
    return false;
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
