#include "serial.h"
#include "pci.h"

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>
#include <QThread>

static bool bVerbose = false;

int setGpu(const QString &device, const QString &command)
{
    SerialUsb serial;

    serial.setVerbose(bVerbose);

    serial.setPortName("ttyACM0");

    auto status = serial.open();

    if(!status) {
        qCritical() << "Cannot open serial";
        return serial.getLastError();
    }

    PCIDRIVER power_plug_pci("pwplug");
    PCIDRIVER vfio_pci("vfio-pci");

    if(command != "1" && command != "0") {
        return 1;
    }

    if(!vfio_pci.isDeviceExists(device)) {
        qCritical() << "Device " + device + " don't exist";
        return 2;
    }

    if(!power_plug_pci.isDriverLoad()) {
       qCritical() << "pwplug-pci driver not loaded";
       return 3;
    }

    if(!vfio_pci.isDriverLoad()) {
       qCritical() << "vfio-pci driver not loaded";
       return 4;
    }

    if(!vfio_pci.isBind(device)) {
       qCritical() << "Device " << device << "must be binded on vfio-pci";
       return 5;
    }

    // Unbind device on vfio_pci. Preparing to reset
    if(!vfio_pci.unbind(device)) {
        return 6;
    }

    // Bind device on pci-power-plug. Preparing to reset
    if(!power_plug_pci.bind(device)) {
        return 7;
    }

    if(!serial.setGpuPower(command == "1")) {
        return serial.getLastError();
    }

    if(command == "1") {
        if(bVerbose) qInfo() << "GPU power on";
    }
    else {
        if(bVerbose) qInfo() << "GPU power off";
    }

    return 0;
}

int reset_GPU(const QString &device, unsigned int time1, unsigned int time2)
{
    PCI pci;
    PCIDRIVER power_plug_pci("pwplug");
    PCIDRIVER vfio_pci("vfio-pci");

    SerialUsb serial;

    serial.setVerbose(bVerbose);

    serial.setPortName("ttyACM0");

    auto serialOpen = serial.open();

    if(!serialOpen) {
        qCritical() << "Cannot open serial";
        return -1;
    }

    if(!serial.ping()) {
        qCritical() << "Ping failed";
        return -2;
    }

    qInfo() << "Remove device:" << device;
    if(!pci.remove(device))
    {
        qInfo() << "Device already removed";
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

    if(!pci.isDeviceExists(device))
    {
        qCritical() << "Device not found";
        return -5;
    }

    return 0;
}

int main(int argc, char *argv[])
{
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

   auto device = parser.value(deviceOption);

   if(bVerbose) qInfo() << "Pcie reset fixer";

   unsigned int delay1 = 4000, delay2 = 1000;

   if(parser.isSet(delayOption1)) {
      delay1 = parser.value(delayOption1).toUInt();
   }

   if(parser.isSet(delayOption2)) {
      delay2 = parser.value(delayOption2).toUInt();
   }

   if(parser.isSet(resetOption) && parser.isSet(deviceOption))
   {
       return reset_GPU(device, delay1, delay2);
   }
   else if((args.size() >= 1)) {
      return setGpu(device, args.at(0));
   }
   else {
      qCritical() << "Incorrect parameter";
   }
   return 0;
}
