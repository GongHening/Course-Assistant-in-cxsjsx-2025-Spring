#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QColor>
#include <QDataStream>

class Task : public QObject
{
    Q_OBJECT

    // 属性声明
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QDate dueDate READ dueDate WRITE setDueDate)
    Q_PROPERTY(QString courseName READ courseName WRITE setCourseName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(bool isCompleted READ isCompleted WRITE setCompleted NOTIFY statusChanged)
    Q_PROPERTY(bool isExam READ isExam WRITE setExam NOTIFY statusChanged)
    Q_PROPERTY(int daysRemaining READ daysRemaining NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QColor priorityColor READ priorityColor NOTIFY statusChanged)

public:
    explicit Task(QObject *parent = nullptr);
    Task(const QString &title, const QDate &dueDate, bool isExam = false, QObject *parent = nullptr);

    // 序列化支持
    friend QDataStream &operator<<(QDataStream &out, const Task &task);
    friend QDataStream &operator>>(QDataStream &in, Task &task);

    // 状态计算
    int daysRemaining() const;
    QString statusText() const;
    QColor priorityColor() const;

    // Getter和Setter
    QString title() const;
    void setTitle(const QString &title);

    QDate dueDate() const;
    void setDueDate(const QDate &date);

    QString courseName() const;
    void setCourseName(const QString &name);

    QString description() const;
    void setDescription(const QString &desc);

    bool isCompleted() const;
    void setCompleted(bool completed);

    bool isExam() const;
    void setExam(bool exam);

signals:
    void statusChanged();

private:
    QString m_title;         // 任务标题
    QDate m_dueDate;         // 截止日期
    QString m_courseName;    // 关联课程名称
    QString m_description;   // 任务描述
    bool m_isCompleted;      // 是否完成
    bool m_isExam;           // 是否为考试
};

#endif // TASK_H
