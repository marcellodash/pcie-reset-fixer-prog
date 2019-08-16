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
   PCI(const QString &name);
   PCI() {}

   void setDriverName(const QString &driver) { m_DriverName = driver; }
   bool bind(const QString &device);
   bool unbind(const QString &device);
   bool new_id(const QString &device);
   bool isDriverLoad();
   bool isDeviceExists(const QString &device);
   bool loadDriver();
   bool unloadDriver();
   bool isBind(const QString &device);

private:
   QString m_DriverName;
};



#endif // PCI_H
