#ifndef COURSE_H
#define COURSE_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QDataStream>

class Course : public QObject
{
    Q_OBJECT

    // 属性声明，用于Qt属性系统
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int dayOfWeek READ dayOfWeek WRITE setDayOfWeek)
    Q_PROPERTY(int startSection READ startSection WRITE setStartSection)
    Q_PROPERTY(int endSection READ endSection WRITE setEndSection)
    Q_PROPERTY(QString classroom READ classroom WRITE setClassroom)
    Q_PROPERTY(QString teacher READ teacher WRITE setTeacher)
    Q_PROPERTY(QString note READ note WRITE setNote)
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    static const int MAX_SECTION = 12; // 最大节次

    explicit Course(QObject *parent = nullptr);
    Course(const QString &name, int day, int startSection, int endSection,
           const QString &classroom, QObject *parent = nullptr);
    // 在 Course 类的 public 部分添加
    Course(const Course &other, QObject *parent = nullptr);
    // 序列化操作
    friend QDataStream &operator<<(QDataStream &out, const Course &course);
    friend QDataStream &operator>>(QDataStream &in, Course &course);
    // 在 Course 类的 public 部分添加
    Course& operator=(const Course &other);
    // 检查时间冲突
    bool hasTimeConflictWith(const Course &other) const;

    // 获取课程持续时间(节数)
    int duration() const;

    // 设置课程节次范围
    void setSections(int start, int end);

    // 生成显示文本
    QString displayText() const;

    // Getter和Setter
    QString name() const;
    void setName(const QString &name);

    int dayOfWeek() const;
    void setDayOfWeek(int day);

    int startSection() const;
    void setStartSection(int section);

    int endSection() const;
    void setEndSection(int section);

    QString classroom() const;
    void setClassroom(const QString &classroom);

    QString teacher() const;
    void setTeacher(const QString &teacher);

    QString note() const;
    void setNote(const QString &note);

    QColor color() const;
    void setColor(const QColor &color);

private:
    QString m_name;          // 课程名称
    int m_dayOfWeek;         // 星期几(1-7)
    int m_startSection;      // 开始节次
    int m_endSection;        // 结束节次
    QString m_classroom;     // 教室
    QString m_teacher;       // 教师姓名
    QString m_note;          // 备注信息
    QColor m_color;          // 显示颜色
};

#endif // COURSE_H
