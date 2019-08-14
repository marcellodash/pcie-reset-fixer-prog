#ifndef TASK_H
#define TASK_H

#include <QObject>

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = nullptr);

public slots:
    void run()
    {
        // Do processing here

        emit finished();
    }

signals:
    void finished();
};

#endif // TASK_H
