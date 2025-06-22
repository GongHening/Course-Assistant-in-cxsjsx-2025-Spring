#include "ScheduleManager.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QDate>
#include <algorithm>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
const QString ScheduleManager::DATA_FILE_PATH = "schedule.dat";
ScheduleManager::ScheduleManager(QObject *parent)
    : QObject(parent)
{
    loadCourses();
}

ScheduleManager::~ScheduleManager()
{
    saveCourses();
}

// 添加课程
bool ScheduleManager::addCourse(const Course &course)
{
    // 检查时间冲突
    for (const auto &existingCourse : m_courses) {
        if (existingCourse->hasTimeConflictWith(course)) {
            qWarning() << "Course time conflict:" << course.name();
            return false;
        }
    }

    // 无冲突则添加
    Course *newCourse = new Course(course, this);
    m_courses.append(newCourse);
    emit coursesChanged();
    return true;
}

// 编辑课程
bool ScheduleManager::editCourse(int index, const Course &newCourse)
{
    if (index < 0 || index >= m_courses.size()) {
        return false;
    }

    // 检查时间冲突（排除自身）
    for (int i = 0; i < m_courses.size(); ++i) {
        if (i != index && m_courses[i]->hasTimeConflictWith(newCourse)) {
            qWarning() << "Course time conflict:" << newCourse.name();
            return false;
        }
    }

    // 更新课程信息
    *m_courses[index] = newCourse;
    emit coursesChanged();
    return true;
}

// 删除课程
bool ScheduleManager::removeCourse(int index)
{
    if (index < 0 || index >= m_courses.size()) {
        return false;
    }

    m_courses[index]->deleteLater();
    m_courses.removeAt(index);
    emit coursesChanged();
    return true;
}

// 获取某天的所有课程
QList<Course*> ScheduleManager::getCoursesByDay(int dayOfWeek) const
{
    QList<Course*> result;
    for (auto course : m_courses) {
        if (course->dayOfWeek() == dayOfWeek) {
            result.append(course);
        }
    }

    // 按节次排序
    std::sort(result.begin(), result.end(), [](Course *a, Course *b) {
        return a->startSection() < b->startSection();
    });

    return result;
}

// 获取当前课程
Course* ScheduleManager::getCurrentCourse() const
{
    QDateTime now = QDateTime::currentDateTime();
    int currentDay = now.date().dayOfWeek(); // Qt中1=周一，7=周日
    int currentSection = getCurrentSection();

    if (currentSection == -1) {
        return nullptr;
    }

    for (auto course : m_courses) {
        if (course->dayOfWeek() == currentDay &&
            course->startSection() <= currentSection &&
            course->endSection() >= currentSection) {
            return course;
        }
    }

    return nullptr;
}

// 获取下一节课
Course* ScheduleManager::getNextCourse() const
{
    QDateTime now = QDateTime::currentDateTime();
    int currentDay = now.date().dayOfWeek();
    int currentSection = getCurrentSection();

    if (currentSection == -1) {
        return nullptr;
    }

    Course *nextCourse = nullptr;
    int minDiff = INT_MAX;

    // 检查当天剩余的课程
    for (auto course : m_courses) {
        if (course->dayOfWeek() == currentDay &&
            course->startSection() > currentSection) {
            int diff = course->startSection() - currentSection;
            if (diff < minDiff) {
                minDiff = diff;
                nextCourse = course;
            }
        }
    }

    // 如果当天没有更多课程，检查后续天数
    if (!nextCourse) {
        for (int day = currentDay + 1; day <= 7; ++day) {
            auto courses = getCoursesByDay(day);
            if (!courses.isEmpty()) {
                return courses.first();
            }
        }
        // 如果下周也没有课程，检查周一
        for (int day = 1; day < currentDay; ++day) {
            auto courses = getCoursesByDay(day);
            if (!courses.isEmpty()) {
                return courses.first();
            }
        }
    }

    return nextCourse;
}

// 保存课程到文件
void ScheduleManager::saveCourses() const
{
    // 创建应用数据目录
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }

    QString filePath = dataDir + "/schedule.dat";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件进行写入:" << filePath;
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_15); // 设置版本确保兼容性

    // 写入版本和课程数量
    out << VERSION_CODE;
    out << static_cast<quint32>(m_courses.size()); // 使用quint32确保跨平台兼容

    // 写入每个课程
    for (const auto &course : m_courses) {
        out << *course;
    }
}

void ScheduleManager::loadCourses()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = dataDir + "/schedule.dat";
    QFile file(filePath);

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件进行读取:" << filePath;
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_15);

    int version;
    in >> version;

    if (version != VERSION_CODE) {
        qWarning() << "数据版本不匹配";
        return;
    }

    quint32 count;
    in >> count;

    m_courses.clear();
    for (quint32 i = 0; i < count; ++i) {
        Course *course = new Course(this);
        in >> *course;
        m_courses.append(course);
    }
}

// 获取当前节次
int ScheduleManager::getCurrentSection() const
{
    QTime currentTime = QTime::currentTime();

    // 定义节次时间表 (可根据实际情况调整)
    static const QVector<QPair<QTime, QTime>> sectionTimes = {
        {QTime(8, 0), QTime(8, 45)},    // 第1节
        {QTime(8, 55), QTime(9, 40)},    // 第2节
        {QTime(10, 0), QTime(10, 45)},   // 第3节
        {QTime(10, 55), QTime(11, 40)},  // 第4节
        {QTime(14, 0), QTime(14, 45)},   // 第5节
        {QTime(14, 55), QTime(15, 40)},  // 第6节
        {QTime(16, 0), QTime(16, 45)},   // 第7节
        {QTime(16, 55), QTime(17, 40)},  // 第8节
        {QTime(19, 0), QTime(19, 45)},   // 第9节
        {QTime(19, 55), QTime(20, 40)},  // 第10节
        {QTime(20, 50), QTime(21, 35)},  // 第11节
        {QTime(21, 45), QTime(22, 30)}   // 第12节
    };

    for (int i = 0; i < sectionTimes.size(); ++i) {
        if (currentTime >= sectionTimes[i].first &&
            currentTime <= sectionTimes[i].second) {
            return i + 1; // 节次从1开始
        }
    }

    return -1; // 不在任何节次时间内
}

// 获取所有课程
const QList<Course*>& ScheduleManager::getAllCourses() const
{
    return m_courses;
}
