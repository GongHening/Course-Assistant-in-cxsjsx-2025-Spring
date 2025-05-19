#include "ScheduleManager.h"
#include <QTimer>
#include <QDebug>

ScheduleManager::ScheduleManager(QObject *parent) : QObject(parent) {
    // 定时检查提醒（每分钟检测一次）
    reminderTimer = new QTimer(this);
    connect(reminderTimer, &QTimer::timeout, [this]() {
        auto now = QDateTime::currentDateTime();
        for (const auto &course : courses) {
            if (course.getWeekdays().contains(now.toString("ddd"))) {
                if (course.getStartTime().secsTo(now.time()) == -300) {  // 课前5分钟提醒
                    emit classReminder(
                        QString("5分钟后有课：%1 @ %2").arg(course.getName()).arg(course.getLocation())
                    );
                }
            }
        }
        checkDueAssignments();
    });
    reminderTimer->start(60000);  // 1分钟间隔
}

void ScheduleManager::addCourse(const Course &course) {
    courses.append(course);
}

void ScheduleManager::addAssignment(QString courseName, QDateTime dueDate, QString description) {
    // 实际开发中应存储作业数据
    qDebug() << "添加作业：" << courseName << dueDate.toString();
}
