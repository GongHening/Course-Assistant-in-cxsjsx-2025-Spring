#include "Course.h"
#include <QDataStream>

// 构造函数
Course::Course(QObject *parent)
    : QObject(parent),
    m_dayOfWeek(1),
    m_startSection(1),
    m_endSection(1),
    m_color(Qt::blue)
{
}

// 带参数的构造函数
Course::Course(const QString &name, int day, int startSection, int endSection,
               const QString &classroom, QObject *parent)
    : QObject(parent),
    m_name(name),
    m_dayOfWeek(day),
    m_startSection(startSection),
    m_endSection(endSection),
    m_classroom(classroom),
    m_color(QColor::fromHsv((day * 50 + startSection * 10) % 360, 150, 230))
{
}
Course::Course(const Course &other, QObject *parent)
    : QObject(parent),
    m_name(other.m_name),
    m_dayOfWeek(other.m_dayOfWeek),
    m_startSection(other.m_startSection),
    m_endSection(other.m_endSection),
    m_classroom(other.m_classroom),
    m_teacher(other.m_teacher),
    m_note(other.m_note),
    m_color(other.m_color)
{
}
Course& Course::operator=(const Course &other)
{
    if (this != &other) {
        m_name = other.m_name;
        m_dayOfWeek = other.m_dayOfWeek;
        m_startSection = other.m_startSection;
        m_endSection = other.m_endSection;
        m_classroom = other.m_classroom;
        m_teacher = other.m_teacher;
        m_note = other.m_note;
        m_color = other.m_color;
    }
    return *this;
}
// 序列化操作(写入数据流)
QDataStream &operator<<(QDataStream &out, const Course &course)
{
    out << course.m_name
        << course.m_dayOfWeek
        << course.m_startSection
        << course.m_endSection
        << course.m_classroom
        << course.m_teacher
        << course.m_note
        << course.m_color;
    return out;
}

// 反序列化操作(从数据流读取)
QDataStream &operator>>(QDataStream &in, Course &course)
{
    in >> course.m_name
        >> course.m_dayOfWeek
        >> course.m_startSection
        >> course.m_endSection
        >> course.m_classroom
        >> course.m_teacher
        >> course.m_note
        >> course.m_color;
    return in;
}

// 检查时间冲突
bool Course::hasTimeConflictWith(const Course &other) const
{
    if (m_dayOfWeek != other.m_dayOfWeek) {
        return false;
    }

    return !(m_endSection < other.m_startSection ||
             other.m_endSection < m_startSection);
}

// 获取课程持续时间(节数)
int Course::duration() const
{
    return m_endSection - m_startSection + 1;
}

// 设置课程节次范围
void Course::setSections(int start, int end)
{
    if (start > end || start < 1 || end > MAX_SECTION) {
        return;
    }

    m_startSection = start;
    m_endSection = end;
}

// 生成显示文本
QString Course::displayText() const
{
    return QString("%1\n%2\n%3").arg(m_name, m_classroom, m_teacher);
}

// 属性getter和setter实现
QString Course::name() const { return m_name; }
void Course::setName(const QString &name) { m_name = name; }

int Course::dayOfWeek() const { return m_dayOfWeek; }
void Course::setDayOfWeek(int day)
{
    if (day >= 1 && day <= 7) {
        m_dayOfWeek = day;
    }
}

int Course::startSection() const { return m_startSection; }
void Course::setStartSection(int section)
{
    if (section >= 1 && section <= MAX_SECTION) {
        m_startSection = section;
    }
}

int Course::endSection() const { return m_endSection; }
void Course::setEndSection(int section)
{
    if (section >= 1 && section <= MAX_SECTION) {
        m_endSection = section;
    }
}

QString Course::classroom() const { return m_classroom; }
void Course::setClassroom(const QString &classroom) { m_classroom = classroom; }

QString Course::teacher() const { return m_teacher; }
void Course::setTeacher(const QString &teacher) { m_teacher = teacher; }

QString Course::note() const { return m_note; }
void Course::setNote(const QString &note) { m_note = note; }

QColor Course::color() const { return m_color; }
void Course::setColor(const QColor &color) { m_color = color; }
