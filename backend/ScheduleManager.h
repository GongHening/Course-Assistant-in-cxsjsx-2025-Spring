//课程表管理类
#pragma once
#include <QObject>
#include <QVector>
#include "Course.h"

class ScheduleManager : public QObject {
    Q_OBJECT
public:
    explicit ScheduleManager(QObject *parent = nullptr);

    // 课程管理
    void addCourse(const Course &course);
    void removeCourse(int index);
    QVector<Course> getTodayCourses() const;

    // 作业提醒
    void addAssignment(QString courseName, QDateTime dueDate, QString description);
    void checkDueAssignments() const;

signals:
    void classReminder(QString message);  // 上课提醒信号
    void assignmentDue(QString message);  // 作业到期信号

private:
    QVector<Course> courses;
    QTimer *reminderTimer;
};
