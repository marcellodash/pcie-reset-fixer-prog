#include <QCoreApplication>
#include <QTimer>

#include "serial.h"
#include "task.h"

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   // Task parented to the application so that it
   // will be deleted by the application.
   Task *task = new Task(&app);

   // This will cause the application to exit when
   // the task signals finished.
   QObject::connect(task, SIGNAL(finished()), &app, SLOT(quit()));

   // This will run the task from the application event loop.
   QTimer::singleShot(0, task, SLOT(run()));


   SerialUsb serial;

   serial.setPortName("ttyACM0");

   auto status = serial.open();

   if(!status) {
       return -1;
   }

   serial.setGpuPower(true);

   return app.exec();
}
