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

#define SYSDRIVER "/sys/bus/pci/drivers/"

#define init_module(module_image, len, param_values) syscall(__NR_init_module, module_image, len, param_values)
#define finit_module(fd, param_values, flags)        syscall(__NR_finit_module, fd, param_values, flags)
#define delete_module(name, flags)                   syscall(__NR_delete_module, name, flags)

PCI::PCI(const QString &name)
{
    m_DriverName = name;
}

bool PCI::loadDriver()
{
    return false;
}

bool PCI::unloadDriver()
{
    return true;
}

bool PCI::isBind(const QString &device)
{
    const QFileInfo dir(SYSDRIVER + m_DriverName + "/" + device);

    return dir.exists();
}

bool PCI::isDriverLoad()
{
    const QFileInfo dir(SYSDRIVER + m_DriverName);

    return dir.exists();
}

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

bool PCI::new_id(const QString &id)
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
