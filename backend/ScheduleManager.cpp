#include "ScheduleManager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

ScheduleManager::ScheduleManager(QObject *parent) : QObject(parent) {
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &ScheduleManager::checkReminders);
    m_reminderTimer->start(60000); // 每分钟检查一次
}

ScheduleManager::~ScheduleManager() {
    m_reminderTimer->stop();
    delete m_reminderTimer;
}

//=== 课程管理 ===
void ScheduleManager::addCourse(const Course &course) {
    m_courses.append(course);
    m_courseColors[course.getName()] = course.getColor(); // 存储课程颜色
}

bool ScheduleManager::removeCourse(const QString &courseName) {
    auto it = std::remove_if(m_courses.begin(), m_courses.end(),
                             [&](const Course &c) { return c.getName() == courseName; });

    if (it != m_courses.end()) {
        m_courses.erase(it, m_courses.end());
        m_courseColors.remove(courseName);
        return true;
    }
    return false;
}

QVector<Course> ScheduleManager::getCoursesByDay(const QString &weekday) const {
    QVector<Course> result;
    for (const auto &course : m_courses) {
        if (course.getWeekdays().contains(weekday)) {
            result.append(course);
        }
    }
    std::sort(result.begin(), result.end(), [](const Course &a, const Course &b) {
        return a.getStartTime() < b.getStartTime();
    });
    return result;
}

//=== 作业管理 ===
void ScheduleManager::addAssignment(const Assignment &assignment) {
    m_assignments.append(assignment);
    std::sort(m_assignments.begin(), m_assignments.end(),
              [](const Assignment &a, const Assignment &b) { return a.dueDate < b.dueDate; });
}

QVector<Assignment> ScheduleManager::getDueAssignments() const {
    QVector<Assignment> due;
    auto now = QDateTime::currentDateTime();
    for (const auto &assn : m_assignments) {
        if (!assn.isCompleted && assn.dueDate >= now) {
            due.append(assn);
        }
    }
    return due;
}

//=== 数据持久化 ===
bool ScheduleManager::importFromJson(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) return false;

    // 解析课程数据
    const auto courses = doc["courses"].toArray();
    for (const auto &c : courses) {
        Course course;
        course.fromJson(c.toObject());
        addCourse(course);
    }

    // 解析作业数据
    const auto assignments = doc["assignments"].toArray();
    for (const auto &a : assignments) {
        Assignment assn;
        assn.courseName = a["course"].toString();
        assn.dueDate = QDateTime::fromString(a["due"].toString(), Qt::ISODate);
        assn.description = a["desc"].toString();
        m_assignments.append(assn);
    }

    return true;
}

//=== 提醒系统 ===
void ScheduleManager::checkReminders() {
    if (!m_isMuted) {
        checkClassReminders();
        checkAssignmentDue();
        checkExams();
    }
}

void ScheduleManager::checkClassReminders() {
    auto now = QDateTime::currentDateTime();
    for (const auto &course : m_courses) {
        if (course.getWeekdays().contains(now.toString("ddd"))) {
            int secsToStart = now.time().secsTo(course.getStartTime());
            if (secsToStart > 0 && secsToStart <= m_reminderMinutes * 60) {
                emit classReminder(
                    QString("%1分钟后有课：%2 @ %3")
                        .arg(m_reminderMinutes)
                        .arg(course.getName())
                        .arg(course.getLocation())
                    );
            }
        }
    }
}

void ScheduleManager::checkExams() {
    auto now = QDate::currentDate();
    for (const auto &course : m_courses) {
        if (course.hasExam() && course.getExamDate().daysTo(now) == 7) {
            emit examComingSoon(
                QString("【考试提醒】%1 将于一周后考试").arg(course.getName())
                );
        }
    }
}
