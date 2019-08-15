#include "task.h"


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

Task::Task(QObject *parent) : QObject(parent)
{

}

void Task::run()
{
    // Do processing here

    emit finished();
}
