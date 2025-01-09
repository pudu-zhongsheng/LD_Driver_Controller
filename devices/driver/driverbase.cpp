#include "driverbase.h"

DriverBase::DriverBase(QWidget *parent)
    : QWidget(parent)
{
}

DriverBase::~DriverBase()
{
}

void DriverBase::startScan()
{
    if (m_scanStartBtn) {
        m_scanStartBtn->setEnabled(false);
    }
    if (m_scanStopBtn) {
        m_scanStopBtn->setEnabled(true);
    }
}

void DriverBase::stopScan()
{
    if (m_scanStartBtn) {
        m_scanStartBtn->setEnabled(true);
    }
    if (m_scanStopBtn) {
        m_scanStopBtn->setEnabled(false);
    }
}

void DriverBase::sendCommand(const QByteArray &cmd)
{
    if (m_serial && m_serial->isConnected()) {
        m_serial->sendData(cmd);
    }
} 
