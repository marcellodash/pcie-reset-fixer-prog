#include "serial.h"
#include "pci.h"

#include <QCoreApplication>
#include <QTimer>

#include <stdio.h>

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   if(argc < 2)
   {
       return -1;
   }

   if(argv[1][1] != 0)
   {
       return -1;
   }

   if(!(argv[1][0] == '0' ||
        argv[1][0] == '1'))
   {
      return -1;
   }

   bool gpuPower = (argv[1][0] == '1');

   SerialUsb serial;
   PCI pci;

   pci.setDriverName("pcipowerplug");

   if(!pci.isDriverLoad())
   {
      printf("Driver not loaded");
      return -2;
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

   return 0;
}

