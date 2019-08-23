#include "serial.h"

#include <QSerialPortInfo>
#include <QDebug>
#define MAGIC "mc"

#define BASE_ERROR (20)

SerialUsb::SerialUsb()
{
}

bool SerialUsb::open()
{
    m_LastError = 0;
    m_Serial.close();
    m_Serial.setPortName(m_portName);
    if(!m_Serial.open(QIODevice::ReadWrite)) {
        m_LastError = BASE_ERROR + 2;
        return false;
    }

    flush();
    return true;
}

void SerialUsb::close()
{
    m_Serial.close();
}

bool SerialUsb::flush()
{
    if(m_Serial.waitForReadyRead(0)) {
        m_Serial.readAll();
    }

    return true;
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

        QByteArray responseData;

        int retry = 100;

        while(responseData.size() < 7)
        {
            if(m_Serial.waitForReadyRead(m_Timeout)) {
              responseData += m_Serial.readAll();
            }

            if(retry-- == 0) {
                m_LastError = BASE_ERROR + 3;
                if(m_Verbose) qInfo() << "Size error: " << responseData.size();
                return false;
            }
        }

        if(responseData[0] != '*'  ||
           responseData[1] != cmd  ||
           responseData[2] != '\r' ||
           responseData[3] != '\n'  ) {
            m_LastError = BASE_ERROR + 4;
            return false;
        }
    }
    else {
        m_LastError = BASE_ERROR + 2;
        return false;
    }

    return true;
}

bool SerialUsb::setGpuPower(bool value)
{
    return sendCommand(value ? '1' : '0');
}

bool SerialUsb::ping()
{
    return sendCommand('9');
}
