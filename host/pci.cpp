#include "pci.h"

#include <QFile>

#define SYSDRIVER "/sys/bus/pci/drivers/"

bool PCI::bind(const QString &device)
{
    QFile file(SYSDRIVER + m_DriverName + "/bind");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(device.toUtf8()) < 0)
    {
        return false;
    }

    return true;
}

bool PCI::unbind(const QString &device)
{
    QFile file(SYSDRIVER + m_DriverName + "/unbind");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(device.toUtf8()) < 0)
    {
        return false;
    }

    return true;
}

bool PCI::new_id(const QString &device)
{
    QFile file(SYSDRIVER + m_DriverName + "/new_id");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(device.toUtf8()) < 0)
    {
        return false;
    }

    return true;
}
