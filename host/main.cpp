#include "serial.h"
#include "pci.h"

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>

#include <stdio.h>

int setGpu(int id, bool gpuPower)
{
    SerialUsb serial;
    PCI reset_pci("pcipowerplug");
    PCI vfio_pci("vfio-pci");

    if(!reset_pci.isDriverLoad())
    {
       printf("PCI reset fixer device driver not loaded\n");
       return -2;
    }

    if(!vfio_pci.isDriverLoad())
    {
       printf("PCI VFIO device driver not loaded\n");
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
        printf("GPU power on\n");
    }
    else {
        printf("GPU power off\n");
    }
}

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   QCommandLineParser parser;
       parser.setApplicationDescription("pcie reset fixer");
       parser.addHelpOption();
       parser.addVersionOption();

   QCommandLineOption showProgressOption("c", QCoreApplication::translate("main", "verbose"));
   parser.addOption(showProgressOption);

   parser.process(app);

   const QStringList args = parser.positionalArguments();

   return setGpu(0, args.at(0) == "1");
}
