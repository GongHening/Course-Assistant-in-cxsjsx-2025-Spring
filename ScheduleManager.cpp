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
            qWarning() << "课程时间冲突:" << course.name();
            return false;
        }
    }

    // 创建新课程对象，并设置父对象为this
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

    Course* course = m_courses.takeAt(index);
    course->deleteLater(); // 安全删除

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

    // 如果当前不在任何节次内，设置为0以便找到所有未来的课程
    if (currentSection == -1) {
        currentSection = 0;
    }

    Course *nextCourse = nullptr;
    qint64 minTimeDiff = std::numeric_limits<qint64>::max();

    for (auto course : m_courses) {
        if (!course) continue; // 跳过空指针

        // 计算课程日期
        int daysToAdd = course->dayOfWeek() - currentDay;
        if (daysToAdd < 0) {
            daysToAdd += 7; // 处理跨周情况
        }

        QDate courseDate = now.date().addDays(daysToAdd);
        QTime startTime = getSectionStartTime(course->startSection());

        if (!startTime.isValid()) {
            qWarning() << "无效的课程开始时间: 节次" << course->startSection();
            continue;
        }

        QDateTime courseStart(courseDate, startTime);

        // 跳过已结束的课程
        if (courseStart < now) continue;

        // 计算时间差
        qint64 timeDiff = now.secsTo(courseStart);

        // 找到时间差最小的课程
        if (timeDiff < minTimeDiff) {
            minTimeDiff = timeDiff;
            nextCourse = course;
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
    const auto& sectionTimes = getSectionTimes();

    for (int i = 0; i < sectionTimes.size(); ++i) {
        const auto& timePair = sectionTimes[i];
        if (currentTime >= timePair.first && currentTime <= timePair.second) {
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


// ScheduleManager.cpp
const QVector<QPair<QTime, QTime>>& ScheduleManager::getSectionTimes()
{
    static const QVector<QPair<QTime, QTime>> SECTION_TIMES = {
        {QTime(8, 0), QTime(8, 50)},
        {QTime(9, 0), QTime(9, 50)},
        {QTime(10, 10), QTime(11, 0)},
        {QTime(11, 10), QTime(12, 0)},
        {QTime(13, 0), QTime(13, 50)},
        {QTime(14, 0), QTime(14, 50)},
        {QTime(15, 10), QTime(16, 0)},
        {QTime(16, 10), QTime(17, 0)},
        {QTime(17, 10), QTime(18, 0)},
        {QTime(18, 40), QTime(19, 30)},
        {QTime(19, 40), QTime(20, 30)},
        {QTime(20, 40), QTime(21, 30)}
    };
    return SECTION_TIMES; // 返回引用
}

QTime ScheduleManager::getSectionStartTime(int section)
{
    if (section >= 1 && section <= getSectionTimes().size()) {
        return getSectionTimes().at(section-1).first;
    }
    qWarning() << "无效的节次:" << section;
    return QTime(0, 0); // 返回默认时间
}

QTime ScheduleManager::getSectionEndTime(int section)
{
    if (section >= 1 && section <= getSectionTimes().size()) {
        return getSectionTimes().at(section-1).second;
    }
    qWarning() << "无效的节次:" << section;
    return QTime(0, 0); // 返回默认时间
}
