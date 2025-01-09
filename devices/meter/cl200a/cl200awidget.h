#ifndef CL200AWIDGET_H
#define CL200AWIDGET_H

#include "../meterbase.h"
#include "communication/cl_twozerozeroacom.h"
#include <QTimer>

class CL200AWidget : public MeterBase
{
    Q_OBJECT
public:
    explicit CL200AWidget(QWidget *parent = nullptr);
    ~CL200AWidget();

    bool isConnected() const override;
    void connectToPort(const QString &portName) override;
    void disconnectPort() override;

    // CL-200A 特定功能
    void setHold(bool hold);
    void setBacklight(bool on);
    void setRange(int range);
    void setPCMode();
    void setEXTMode(bool on);

private slots:
    void handleSerialData(const QByteArray &data);
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    CL_TwoZeroZeroACOM *m_protocol;
    QTimer *m_statusTimer;

    void initUI();
    void initConnections();
    void startMeasurement();
    void stopMeasurement();
};

#endif // CL200AWIDGET_H 