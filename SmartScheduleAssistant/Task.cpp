#include "Task.h"
#include <QDate>
#include <QDebug>

// 构造函数
Task::Task(QObject *parent)
    : QObject(parent),
    m_dueDate(QDate::currentDate()),
    m_dueTime(QTime(23, 59)), // 默认时间 23:59
    m_isCompleted(false),
    m_isExam(false)
{
}
// 带参数的构造函数
Task::Task(const QString &title, const QDate &dueDate, bool isExam, QObject *parent)
    : QObject(parent),
    m_title(title),
    m_dueDate(dueDate),
    m_isCompleted(false),
    m_isExam(isExam)
{
}

// 计算剩余天数
int Task::daysRemaining() const
{
    QDate today = QDate::currentDate();
    if (m_isCompleted) {
        return 0;
    }
    return today.daysTo(m_dueDate);
}

// 获取状态文本
QString Task::statusText() const
{
    if (m_isCompleted) {
        return tr("已完成");
    }

    int days = daysRemaining();
    if (days < 0) {
        return tr("已过期 %1 天").arg(-days);
    } else if (days == 0) {
        return tr("今天截止");
    } else {
        return tr("还剩 %1 天").arg(days);
    }
}

// 获取优先级颜色
QColor Task::priorityColor() const
{
    if (m_isCompleted) {
        return QColor(200, 200, 200); // 灰色表示已完成
    }

    int days = daysRemaining();
    if (days < 0) {
        return QColor(255, 100, 100); // 红色表示已过期
    } else if (days <= 3) {
        return QColor(255, 200, 100); // 橙色表示即将截止
    } else if (m_isExam) {
        return QColor(100, 150, 255); // 蓝色表示考试
    } else {
        return QColor(150, 255, 150); // 绿色表示普通作业
    }
}

// 序列化操作
QDataStream &operator<<(QDataStream &out, const Task &task)
{
    out << task.m_title
        << task.m_dueDate
        << task.m_dueTime  // 添加
        << task.m_courseName
        << task.m_description
        << task.m_isCompleted
        << task.m_isExam;
    return out;
}

// 反序列化操作
QDataStream &operator>>(QDataStream &in, Task &task)
{
    in >> task.m_title
        >> task.m_dueDate
        >> task.m_dueTime  // 添加
        >> task.m_courseName
        >> task.m_description
        >> task.m_isCompleted
        >> task.m_isExam;
    return in;
}

// 属性getter和setter实现
QString Task::title() const { return m_title; }
void Task::setTitle(const QString &title) { m_title = title; }

QDate Task::dueDate() const { return m_dueDate; }
void Task::setDueDate(const QDate &date) { m_dueDate = date; }

QString Task::courseName() const { return m_courseName; }
void Task::setCourseName(const QString &name) { m_courseName = name; }

QString Task::description() const { return m_description; }
void Task::setDescription(const QString &desc) { m_description = desc; }

bool Task::isCompleted() const { return m_isCompleted; }
void Task::setCompleted(bool completed)
{
    if (m_isCompleted != completed) {
        m_isCompleted = completed;
        emit statusChanged();
    }
}

bool Task::isExam() const { return m_isExam; }
void Task::setExam(bool exam)
{
    if (m_isExam != exam) {
        m_isExam = exam;
        emit statusChanged();
    }
}
