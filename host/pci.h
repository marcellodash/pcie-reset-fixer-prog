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
   bool bind(const QString &device);
   bool unbind(const QString &device);
   bool new_id(const QString &device);
   void setDriverName(const QString &driver) { m_DriverName = driver; }

private:
   QString m_DriverName;
};



#endif // PCI_H
