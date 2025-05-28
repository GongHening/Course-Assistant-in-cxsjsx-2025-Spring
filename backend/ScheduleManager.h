// 课程表管理类
#pragma once
#include <QObject>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QTimer>
#include <QJsonDocument>
#include <QFile>
#include "Course.h"

struct Assignment {
    QString courseName;
    QDateTime dueDate;
    QString description;
    bool isCompleted = false;
};

class ScheduleManager : public QObject {
    Q_OBJECT
public:
    explicit ScheduleManager(QObject *parent = nullptr);
    ~ScheduleManager();

    // 课程管理
    void addCourse(const Course &course);
    bool removeCourse(const QString &courseName);
    void editCourse(const QString &oldName, const Course &newCourse);
    QVector<Course> getCoursesByDay(const QString &weekday) const;
    Course getCurrentCourse() const;
    Course getNextCourse() const;

    // 数据导入/导出
    bool importFromJson(const QString &filePath);
    bool exportToJson(const QString &filePath);
    bool importFromCsv(const QString &filePath); // 需实现解析逻辑

    // 作业管理
    void addAssignment(const Assignment &assignment);
    void removeAssignment(int index);
    void markAssignmentCompleted(int index, bool completed);
    QVector<Assignment> getDueAssignments() const;

    // 提醒设置
    void setClassReminderMinutes(int minutes);
    void setMuteMode(bool muted);

signals:
    void classReminder(const QString &message);
    void assignmentDue(const QString &message);
    void examComingSoon(const QString &message); // 新增考试提醒

private slots:
    void checkReminders();

private:
    void checkClassReminders();
    void checkAssignmentDue();
    void checkExams(); // 检查临近考试

    QVector<Course> m_courses;
    QVector<Assignment> m_assignments;
    QHash<QString, QColor> m_courseColors; // 课程颜色映射
    QTimer *m_reminderTimer;
    int m_reminderMinutes = 5; // 默认课前5分钟提醒
    bool m_isMuted = false;
};
