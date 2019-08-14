#include <QCoreApplication>
#include <QTimer>

#include "serial.h"
#include "task.h"

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

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

   SerialUsb serial;

   serial.setPortName("ttyACM0");

   auto status = serial.open();

   if(!status) {
       return serial.getLastError();
   }

   if(!serial.setGpuPower(argv[1][0] == '1')) {
       return serial.getLastError();
   }

   return 0;//app.exec();
}
