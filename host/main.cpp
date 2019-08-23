#include "serial.h"
#include "pci.h"

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>
#include <QThread>

static bool bVerbose = false;

static int setGpu(bool value)
{
    SerialUsb serial;

    serial.setVerbose(bVerbose);

    serial.setPortName("ttyACM0");

    auto status = serial.open();

    if(!status) {
        qCritical() << "Cannot open serial";
        return serial.getLastError();
    }

    if(!serial.setGpuPower(value)) {
        return serial.getLastError();
    }

    if(value) {
        qInfo() << "GPU power on";
    }
    else {
        qInfo() << "GPU power off";
    }

    return 0;
}

static int reset_GPU(const QStringList &devices, unsigned int time1, unsigned int time2)
{
    PCI pci;
    PCIDRIVER pwplug("pwplug");
    PCIDRIVER vfio_pci("vfio-pci");

    SerialUsb serial;
    if(!pwplug.isDriverLoad()) {
       qCritical() << "pwplug driver not loaded";
       return 3;
    }

    if(!vfio_pci.isDriverLoad()) {
       qCritical() << "vfio-pci driver not loaded";
       return 4;
    }

    serial.setVerbose(bVerbose);

    serial.setPortName("ttyACM0");

    auto serialOpen = serial.open();

    if(!serialOpen) {
        qCritical() << "Cannot open serial " + serial.getPortName();
        return -1;
    }

    if(!serial.ping()) {
        qCritical() << "Ping failed";
        return -2;
    }

    qInfo() << "Unbind devices from vfio-pci";
    for(auto device : devices) {
        if(!vfio_pci.unbind(device)) {
            qCritical() << "Error unbinding device: " << device;
        }
    }

    qInfo() << "Unbind devices from pwplug";
    for(auto device : devices) {
        if(!pwplug.unbind(device)) {
            qCritical() << "Error unbinding device: " << device;
        }
    }

    qInfo() << "Bind devices to pwplug";
    for(auto device : devices) {
        if(!pwplug.bind(device)) {
            qCritical() << "Error binding device: " << device;
            return -1;
        }
    }

    QThread::msleep(100);

    qInfo() << "Check devices binds to pwplug";
    for(auto device : devices) {
        if(!pwplug.isBind(device)) {
            qCritical() << "Error device not binded to pwplug: " << device;
            return -1;
        }

        if(pci.isD3ColdAllowed(device)) {
            qInfo() << "D3Cold allowed " << device;
        }
    }

    qInfo() << "Remove devices from kernel:" << devices;
    for(auto device : devices) {
        if(!pci.remove(device)) {
            qInfo() << "Device already removed: " << device;
        }
    }

    QThread::msleep(500);

    // Power off GPU
    qInfo() << "Power off GPU";
    if(!serial.setGpuPower(false)) {
        qCritical() << "GPU power off failed";
        return -3;
    }

    QThread::msleep(time1);

    // Power on GPU
    qInfo() << "Power on GPU";
    if(!serial.setGpuPower(true)) {
        qCritical() << "GPU power on failed";
    }

    QThread::msleep(time2);

    qInfo() << "Rescan";

    if(!pci.rescan())
    {
        qCritical() << "Rescan failed";
        return -4;
    }

    QThread::msleep(300);

    qInfo() << "Unbind devices from pwplug";
    for(auto device : devices) {
        if(!pwplug.unbind(device)) {
            qCritical() << "Error unbinding device: " << device;
        }
    }

    qInfo() << "Bind devices to vfio-pci";
    for(auto device : devices) {
        if(!vfio_pci.bind(device)) {
            qCritical() << "Error binding device: " << device;
        }
    }

    QThread::msleep(300);

    for(auto device : devices) {
        if(!pci.isDeviceExists(device))
        {
            qCritical() << "Device not found: " << device;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
   int ret = 0;

   QCoreApplication app(argc, argv);

   QCommandLineParser parser;
       parser.setApplicationDescription("pcie reset fixer");
       parser.addHelpOption();
       parser.addVersionOption();

   QCommandLineOption verboseOption("c", QCoreApplication::translate("main", "verbose"));
   parser.addOption(verboseOption);

   QCommandLineOption resetOption("r", QCoreApplication::translate("main", "reset"));
   parser.addOption(resetOption);

   QCommandLineOption deviceOption(QStringList() << "s" << "device",
           "PCIe device",
           "domain:bus:slot.func");
   parser.addOption(deviceOption);

   QCommandLineOption delayOption1(QStringList() << "d" << "delay1",  "Delay 1", "miliseconds");
   parser.addOption(delayOption1);

   QCommandLineOption delayOption2(QStringList() << "y" << "delay2", "Delay 2", "miliseconds");
   parser.addOption(delayOption2);

   parser.process(app);

   const QStringList args = parser.positionalArguments();

   bVerbose = parser.isSet(verboseOption);

   if(bVerbose) qInfo() << "Pcie reset fixer";

   unsigned int delay1 = 4000, delay2 = 1000;

   if(parser.isSet(delayOption1)) {
      delay1 = parser.value(delayOption1).toUInt();
   }

   if(parser.isSet(delayOption2)) {
      delay2 = parser.value(delayOption2).toUInt();
   }

   QStringList devices;

   if(parser.isSet(deviceOption)) {
       QRegExp rx("[, ]");// match a comma or a space
       devices = parser.value(deviceOption).split(rx, QString::SkipEmptyParts);
   }

   if(parser.isSet(resetOption))
   {
      if(devices.size()) {
         ret = reset_GPU(devices, delay1, delay2);
      }
      else {
         qCritical() << "Incorrect parameter: devices";
         ret = -1;
      }
   }
   else if(args.size() >= 1) {
      if(args[0] == "on")
         ret = setGpu(true);
      else if(args[0] == "off") {
         ret = setGpu(false);
      }
      else {
         qCritical() << "Incorrect parameter";
         ret = -1;
      }
   }
   else {
      qCritical() << "Incorrect parameter";
      ret = -1;
   }

   return ret;
}
