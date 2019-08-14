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
        m_LastError = -1;
        return false;
    }
    return true;
}

void SerialUsb::close()
{
    m_Serial.close();
}

bool SerialUsb::sendCommand(char cmd)
{
    char str[10] = MAGIC;
    str[2] = cmd;
    str[3] = '\r';
    m_Serial.write(str, 4);

    if(m_Serial.waitForBytesWritten(m_Timeout)) {
        // Read response
        if(m_Serial.waitForReadyRead(m_Timeout)) {
            QByteArray responseData = m_Serial.readAll();


            int a = 1;
        }
    }
    else {
        m_LastError = -2;
        return false;
    }

    return true;
}

bool SerialUsb::setGpuPower(bool value)
{
    return sendCommand(value ? '1' : '0');
}
