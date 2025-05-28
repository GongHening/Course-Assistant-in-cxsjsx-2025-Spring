// 课程数据类
#pragma once
#include <QString>
#include <QTime>
#include <QDate>
#include <QColor>
#include <QJsonObject>
#include <QList>

class Course {
public:
    // 构造函数重载
    Course();
    Course(QString name, QString location,
           QTime startTime, QTime endTime,
           const QStringList &weekdays,
           QString teacher = "",
           QColor color = Qt::blue,
           QDate examDate = QDate());

    // 基础信息
    QString getName() const;
    QString getLocation() const;
    QStringList getWeekdays() const;
    QTime getStartTime() const;
    QTime getEndTime() const;
    QString getTeacher() const;
    QColor getColor() const;
    QDate getExamDate() const;
    bool hasExam() const;

    // 设置方法
    void setName(const QString &name);
    void setLocation(const QString &location);
    void setTimeRange(QTime start, QTime end);
    void setWeekdays(const QStringList &weekdays);
    void setTeacher(const QString &teacher);
    void setColor(const QColor &color);
    void setExamDate(const QDate &date);

    // 工具方法
    bool isOnDay(const QString &weekday) const;
    int durationMinutes() const;
    QString toString() const;

    // JSON 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    QString m_name;
    QString m_location;
    QTime m_startTime;
    QTime m_endTime;
    QStringList m_weekdays;  // ["Mon", "Wed", "Fri"]
    QString m_teacher;
    QColor m_color;
    QDate m_examDate;
};
