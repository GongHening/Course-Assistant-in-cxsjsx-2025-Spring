//课程数据类
#pragma once
#include <QString>
#include <QDateTime>

class Course {
public:
    Course(QString name, QString location, 
           QTime startTime, QTime endTime, 
           QString weekdays);

    QString getName() const;
    QString getLocation() const;
    QString getWeekdays() const;
    QTime getStartTime() const;
    QTime getEndTime() const;

private:
    QString name;
    QString location;
    QTime startTime;
    QTime endTime;
    QString weekdays;  // 上课日字符串
};
