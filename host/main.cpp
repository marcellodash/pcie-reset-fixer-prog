#include <QCoreApplication>
#include <QTimer>

#include "serial.h"
#include "task.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   if(argc < 2)
   {
       return -10;
   }

   if(argv[1][1] != 0)
   {
       return -11;
   }

   if(!(argv[1][0] == '0' ||
        argv[1][0] == '1'))
   {
      return -12;
   }

   bool gpuPower = (argv[1][0] == '1');

   SerialUsb serial;

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
   //app.exec();
}


   /*
   // Task parented to the application so that it
   // will be deleted by the application.
   Task *task = new Task(&app);

   // This will cause the application to exit when
   // the task signals finished.
   QObject::connect(task, SIGNAL(finished()), &app, SLOT(quit()));

   // This will run the task from the application event loop.
   QTimer::singleShot(0, task, SLOT(run()));
   */

