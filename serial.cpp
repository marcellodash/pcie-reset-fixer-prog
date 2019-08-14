#include "serial.h"

#include <QSerialPortInfo>

#define MAGIC "mc"

SerialUsb::SerialUsb()
{
}

bool SerialUsb::open()
{
    m_Serial.close();
    m_Serial.setPortName(m_portName);
    if(!m_Serial.open(QIODevice::ReadWrite)) {
        return false;
    }
    return true;
}

void SerialUsb::close()
{
    m_Serial.close();
}

void SerialUsb::setGpuPower(bool value)
{
    char cmd[10] = MAGIC;
    cmd[2] = (value ? '1' : '0');
    cmd[3] = '\r';
    m_Serial.write(cmd, 4);
}
