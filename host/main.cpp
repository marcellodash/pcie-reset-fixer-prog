#include "serial.h"
#include "pci.h"

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include<QDebug>

static bool bVerbose = false;

int setGpu(const QString &device, const QString &command)
{
    SerialUsb serial;

    serial.setVerbose(bVerbose);

    PCI reset_pci("pcipowerplug");
    PCI vfio_pci("vfio-pci");

    if(command != "1" && command != "0") {
        return 1;
    }

    if(!reset_pci.isDriverLoad())
    {
       if(bVerbose) qCritical() << "PCI reset fixer device driver not loaded";
       return 2;
    }

    if(!vfio_pci.isDriverLoad())
    {
       if(bVerbose) qCritical() << "PCI VFIO device driver not loaded\n";
       return 3;
    }

    if(reset_pci.isBind(device)) {
        if(bVerbose) qInfo() << "Device binded for device";
    }

    serial.setPortName("ttyACM0");

    auto status = serial.open();

    if(!status) {
        return serial.getLastError();
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

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   QCommandLineParser parser;
       parser.setApplicationDescription("pcie reset fixer");
       parser.addHelpOption();
       parser.addVersionOption();

   QCommandLineOption verboseOption("c", QCoreApplication::translate("main", "verbose"));
   parser.addOption(verboseOption);


   // An option with a value
   QCommandLineOption deviceOption(QStringList() << "s" << "device",
           QCoreApplication::translate("main", "PCIe device"),
           QCoreApplication::translate("main", "[[[[<domain>]:]<bus>]:][<slot>][.[<func>]]"));
   parser.addOption(deviceOption);

   parser.process(app);

   const QStringList args = parser.positionalArguments();

   bVerbose = parser.isSet(verboseOption);

   auto device = parser.value(deviceOption);

   if(bVerbose) qInfo() << "Pcie reset fixer";
   if(bVerbose) qInfo() << "Device: " << device;

   if(args.size() >= 1) {
      return setGpu(device, args.at(0));
   }
   else {
      qWarning() << "Incorrect parameter";
   }

   return 0;
}
