#include "pci.h"

#include "linux/module.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <QFile>
#include <QFileInfo>
#include <QDebug>

#define SYSDRIVER "/sys/bus/pci/drivers/"
#define SYSDEVICE "/sys/bus/pci/devices/"
#define SYSRESCAN "/sys/bus/pci/rescan"

#define init_module(module_image, len, param_values) syscall(__NR_init_module, module_image, len, param_values)
#define finit_module(fd, param_values, flags)        syscall(__NR_finit_module, fd, param_values, flags)
#define delete_module(name, flags)                   syscall(__NR_delete_module, name, flags)

bool PCI::enable(const QString &device, bool value)
{
    char v[2];

    v[0] = value ? '1' : '0';
    v[1] = 0;

    QFile file(SYSDEVICE + device + "/enable");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(v, 2))
    {
        return false;
    }

    return true;
}

bool PCI::remove(const QString &device)
{
    char value1[] = "1";
    QFile file(SYSDEVICE + device + "/remove");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(value1, sizeof(value1)))
    {
        return false;
    }

    return true;
}

bool PCI::rescan()
{
    char value1[] = "1";
    QFile file(SYSRESCAN);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(value1, sizeof(value1)))
    {
        return true; // FIXME
    }

    return true;
}

bool PCI::isDeviceExists(const QString &device)
{
    const QFileInfo dir(SYSDEVICE + device);

    return dir.exists();
}


bool PCIDRIVER::isBind(const QString &device)
{
    const QFileInfo dir(SYSDRIVER + m_DriverName + "/" + device);

    return dir.exists();
}

PCIDRIVER::PCIDRIVER(const QString &name)
{
    m_DriverName = name;
}

bool PCIDRIVER::loadDriver()
{
    return false;
}

bool PCIDRIVER::unloadDriver()
{
    return true;
}

bool PCIDRIVER::isDriverLoad()
{
    const QFileInfo dir(SYSDRIVER + m_DriverName);

    return dir.exists();
}


bool PCIDRIVER::bind(const QString &device)
{
    QFile file(SYSDRIVER + m_DriverName + "/bind");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(device.toUtf8()) < 0)
    {
        //return false;
    }

    return isBind(device);
}

bool PCIDRIVER::unbind(const QString &device)
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


bool PCIDRIVER::new_id(const QString &id)
{
    QFile file(SYSDRIVER + m_DriverName + "/new_id");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    if(file.write(id.toUtf8()) < 0)
    {
        return false;
    }

    return true;
}
