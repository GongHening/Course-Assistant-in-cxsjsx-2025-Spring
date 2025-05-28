#include "Course.h"
#include <QJsonArray>

Course::Course() : m_color(Qt::blue) {}

Course::Course(QString name, QString location,
               QTime startTime, QTime endTime,
               const QStringList &weekdays,
               QString teacher,
               QColor color,
               QDate examDate) :
    m_name(name),
    m_location(location),
    m_startTime(startTime),
    m_endTime(endTime),
    m_weekdays(weekdays),
    m_teacher(teacher),
    m_color(color),
    m_examDate(examDate) {}

//=== Getter 方法 ===
QString Course::getName() const { return m_name; }
QString Course::getLocation() const { return m_location; }
QStringList Course::getWeekdays() const { return m_weekdays; }
QTime Course::getStartTime() const { return m_startTime; }
QTime Course::getEndTime() const { return m_endTime; }
QString Course::getTeacher() const { return m_teacher; }
QColor Course::getColor() const { return m_color; }
QDate Course::getExamDate() const { return m_examDate; }
bool Course::hasExam() const { return m_examDate.isValid(); }

//=== Setter 方法 ===
void Course::setName(const QString &name) { m_name = name; }
void Course::setLocation(const QString &location) { m_location = location; }
void Course::setTeacher(const QString &teacher) { m_teacher = teacher; }
void Course::setColor(const QColor &color) { m_color = color; }
void Course::setExamDate(const QDate &date) { m_examDate = date; }

void Course::setTimeRange(QTime start, QTime end) {
    m_startTime = start;
    m_endTime = end;
}

void Course::setWeekdays(const QStringList &weekdays) {
    m_weekdays = weekdays;
}

//=== 功能方法 ===
bool Course::isOnDay(const QString &weekday) const {
    return m_weekdays.contains(weekday);
}

int Course::durationMinutes() const {
    return m_startTime.secsTo(m_endTime) / 60;
}

QString Course::toString() const {
    return QString("%1 @ %2 (%3-%4)")
    .arg(m_name)
        .arg(m_location)
        .arg(m_startTime.toString("hh:mm"))
        .arg(m_endTime.toString("hh:mm"));
}

//=== JSON 序列化 ===
QJsonObject Course::toJson() const {
    QJsonObject json;
    json["name"] = m_name;
    json["location"] = m_location;
    json["startTime"] = m_startTime.toString("hh:mm:ss");
    json["endTime"] = m_endTime.toString("hh:mm:ss");

    QJsonArray weekdaysArray;
    for (const auto &day : m_weekdays) {
        weekdaysArray.append(day);
    }
    json["weekdays"] = weekdaysArray;

    json["teacher"] = m_teacher;
    json["color"] = m_color.name();
    if (m_examDate.isValid()) {
        json["examDate"] = m_examDate.toString(Qt::ISODate);
    }
    return json;
}

void Course::fromJson(const QJsonObject &json) {
    m_name = json["name"].toString();
    m_location = json["location"].toString();
    m_startTime = QTime::fromString(json["startTime"].toString(), "hh:mm:ss");
    m_endTime = QTime::fromString(json["endTime"].toString(), "hh:mm:ss");

    m_weekdays.clear();
    for (const auto &day : json["weekdays"].toArray()) {
        m_weekdays.append(day.toString());
    }

    m_teacher = json["teacher"].toString();
    m_color = QColor(json["color"].toString());

    if (json.contains("examDate")) {
        m_examDate = QDate::fromString(json["examDate"].toString(), Qt::ISODate);
    }
}
