#include "meterbase.h"

MeterBase::MeterBase(QWidget *parent)
    : QWidget(parent)
    , m_serial(new SerialUtil(this))
{
    // 连接串口信号
    connect(m_serial, &SerialUtil::portDisconnected,
            this, [this]() {
                emit serialDisconnected();
            });
}

MeterBase::~MeterBase()
{
    if (m_serial) {
        if (m_serial->isConnected()) {
            m_serial->disconnectPort();
        }
        delete m_serial;
        m_serial = nullptr;
    }
} 