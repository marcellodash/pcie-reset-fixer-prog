#include "serial.h"
#include "pci.h"

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include<QDebug>


bool bVerbose = false;

int setGpu(int id, bool gpuPower)
{
    SerialUsb serial;
    PCI reset_pci("pcipowerplug");
    PCI vfio_pci("vfio-pci");

    if(!reset_pci.isDriverLoad())
    {
       if(bVerbose) qCritical() << "PCI reset fixer device driver not loaded";
       return -2;
    }

    if(!vfio_pci.isDriverLoad())
    {
       if(bVerbose) qCritical() << "PCI VFIO device driver not loaded\n";
       //return -3;
    }

    if(reset_pci.isBind("0000:00:1f.3")) {
        //return -4;
    }

    serial.setPortName("ttyACM0");

    auto status = serial.open();

    if(!status) {
        return serial.getLastError();
    }

    if(!serial.setGpuPower(gpuPower)) {
        return serial.getLastError();
    }

    if(gpuPower) {
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

   if(bVerbose) qInfo() << "Pcie reset fixer";


   if(args.size() >= 1) {
      return setGpu(0, args.at(0) == "1");
   }
   else {
      qWarning() << "Incorrect parameter";
   }

   return -1;
}
