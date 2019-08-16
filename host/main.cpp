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

    serial.setPortName("ttyACM0");

    auto status = serial.open();

    if(!status) {
        return serial.getLastError();
    }

    PCI power_plug_pci("pcipowerplug");
    PCI vfio_pci("vfio-pci");

    if(command != "1" && command != "0") {
        return 1;
    }

    if(!power_plug_pci.isDriverLoad()) {
       qCritical() << "PCI reset fixer device driver not loaded";
       return 2;
    }

    if(!vfio_pci.isDriverLoad()) {
       qCritical() << "PCI VFIO device driver not loaded";
       return 3;
    }

    if(!vfio_pci.isDeviceExists(device)) {
        qCritical() << "Device " + device + " don't exist";
        return 4;
    }

    if(!vfio_pci.isBind(device)) {
       qCritical() << "Device " << device << "must be binded on vfio-pci";
       return 4;
    }

    // Unbind device on vfio_pci. Preparing to reset
    if(!vfio_pci.unbind(device)) {
        return 5;
    }

    // Bind device on pci-power-plug. Preparing to reset
    if(!power_plug_pci.bind(device)) {
        return 5;
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

   if(args.size() >= 1) {
      return setGpu(device, args.at(0));
   }
   else {
      qCritical() << "Incorrect parameter";
   }

   return 0;
}
