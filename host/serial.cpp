#include "serial.h"

#include <QSerialPortInfo>

#define MAGIC "mc"

#define BASE_ERROR (-1000)

SerialUsb::SerialUsb()
{
}

bool SerialUsb::open()
{
    m_LastError = 0;
    m_Serial.close();
    m_Serial.setPortName(m_portName);
    if(!m_Serial.open(QIODevice::ReadWrite)) {
        m_LastError = BASE_ERROR - 2;
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

    m_LastError = 0;

    str[2] = cmd;
    str[3] = '\r';
    m_Serial.write(str, 4);

    if(m_Serial.waitForBytesWritten(m_Timeout)) {
        // Read response
        if(m_Serial.waitForReadyRead(m_Timeout)) {
            QByteArray responseData = m_Serial.readAll();

            if(responseData.size() < 7) {
                m_LastError = BASE_ERROR - 3;
                return false;
            }

            if(responseData[0] != '*'  ||
               responseData[1] != cmd  ||
               responseData[2] != '\r' ||
               responseData[3] != '\n'  ) {
                m_LastError = BASE_ERROR - 4;
                return false;
            }
        }
    }
    else {
        m_LastError = BASE_ERROR -2;
        return false;
    }

    return true;
}

bool SerialUsb::setGpuPower(bool value)
{
    return sendCommand(value ? '1' : '0');
}
