#ifndef SERIAL_H
#define SERIAL_H

#include <QSerialPort>
#include <QTime>

class SerialUsb
{
public:
    SerialUsb();

    bool open();
    void close();

    void setPortName(const QString &name) { m_portName = name; }

    bool setGpuPower(bool value);
    int getLastError() { return m_LastError; }

private:
    QString m_portName;
    QSerialPort m_Serial;
    int m_Timeout = 100;
    int m_LastError = 0;

    bool sendCommand(char cmd);
};


#endif // SERIAL_H
