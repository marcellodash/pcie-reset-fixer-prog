#ifndef PCI_H
#define PCI_H

#include <QString>

/*
 * bind
 * new_id
 * remove_id
 * uevent
 * unbind
 */

class PCI
{
public:
   PCI() {}

   bool enable(const QString &device, bool value);
   bool remove(const QString &device);
   bool isDeviceExists(const QString &device);
   bool rescan();

private:
};

class PCIDRIVER : public PCI
{
public:
    PCIDRIVER(const QString &name);
    void setDriverName(const QString &driver) { m_DriverName = driver; }
    bool bind(const QString &device);
    bool unbind(const QString &device);
    bool new_id(const QString &device);
    bool isBind(const QString &device);
    bool isDriverLoad();
    bool loadDriver();
    bool unloadDriver();

private:
    QString m_DriverName;
};

#endif // PCI_H
