#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QList>
#include "Task.h"

class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);

    void addTask(Task *task);
    void removeTask(Task *task);
    void setTaskCompleted(Task *task, bool completed);
    const QList<Task*>& getAllTasks() const;
    void loadTasks();
    void saveTasks() const;

signals:
    void tasksChanged();

private:
    QList<Task*> m_tasks;
    static const QString TASK_FILE_PATH;
};

#endif // TASKMANAGER_H
