#ifndef SERIAL_H
#define SERIAL_H

#include <QSerialPort>
#include <QTime>

class SerialUsb
{
public:
    SerialUsb();

    bool open();
    bool flush();
    void close();

    void setPortName(const QString &name) { m_portName = name; }
    QString getPortName() { return m_portName; }

    bool setGpuPower(bool value);
    bool ping();
    int getLastError() { return m_LastError; }
    void setVerbose(bool value) { m_Verbose = value; }

private:
    QString m_portName;
    QSerialPort m_Serial;
    int m_Timeout = 500;
    int m_LastError = 0;
    bool m_Verbose = false;

    bool sendCommand(char cmd);
};


#endif // SERIAL_H
