#include "TaskManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>

TaskManager::TaskManager(QObject *parent) : QObject(parent) {}

void TaskManager::addTask(Task *task)
{
    if (!task) return;
    m_tasks.append(task);
    emit tasksChanged();
}

void TaskManager::removeTask(Task *task)
{
    if (!task) {
        qWarning() << "尝试删除空任务指针";
        return;
    }

    if (!m_tasks.contains(task)) {
        qWarning() << "任务不在列表中:" << task->title();
        return;
    }

    m_tasks.removeOne(task);
    task->deleteLater();
    emit tasksChanged();
    qDebug() << "已删除任务:" << task->title();
}

void TaskManager::setTaskCompleted(Task *task, bool completed)
{
    if (!task) {
        qWarning() << "尝试设置空任务完成状态";
        return;
    }

    // 确保任务存在于列表中
    if (!m_tasks.contains(task)) {
        qWarning() << "任务不在列表中:" << task->title();
        return;
    }

    // 设置完成状态
    task->setCompleted(completed);

    // 通知变化
    emit tasksChanged();
}

const QList<Task*>& TaskManager::getAllTasks() const
{
    return m_tasks;
}


const QString TaskManager::TASK_FILE_PATH =
    QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasks.dat";

void TaskManager::saveTasks() const
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }

    QString filePath = dataDir + "/tasks.dat";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开任务文件进行写入:" << TASK_FILE_PATH;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15);
    out << static_cast<quint32>(m_tasks.size());

    for (const auto &task : m_tasks) {
        out << task->title()
        << task->courseName()
        << task->dueDate()
        << task->dueTime()
        << task->description()
        << task->isCompleted()
        << task->isExam();
    }
}

void TaskManager::loadTasks()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QString filePath = dataDir + "/tasks.dat";
    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开任务文件进行读取:" << TASK_FILE_PATH;
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_15);

    quint32 count;
    in >> count;

    for (quint32 i = 0; i < count; ++i) {
        QString title, courseName, description;
        QDate dueDate;
        QTime dueTime;
        bool isCompleted, isExam;

        // 确保读取所有字段
        in >> title >> courseName >> dueDate >> dueTime >> description >> isCompleted >> isExam;

        Task *task = new Task(this);
        task->setTitle(title);
        task->setCourseName(courseName);
        task->setDueDate(dueDate);
        task->setDueTime(dueTime);  // 设置时间
        task->setDescription(description);
        task->setCompleted(isCompleted);
        task->setExam(isExam);

        m_tasks.append(task);
    }
}
