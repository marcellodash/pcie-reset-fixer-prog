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

    void setGpuPower(bool value);

private:
    QString m_portName;
    QSerialPort m_Serial;
};


#endif // SERIAL_H
