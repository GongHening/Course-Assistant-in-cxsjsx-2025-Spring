#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTime>
#include <QSet>
#include "Course.h"
#include "ScheduleManager.h"

enum NotificationType {
    Information,
    Warning,
    Critical
};

class Notification : public QObject
{
    Q_OBJECT
public:
    explicit Notification(ScheduleManager* scheduleMgr, QObject *parent = nullptr);
    ~Notification();
    // 静音控制
    void setMuted(bool muted);
    // 设置提前提醒分钟数
    void setReminderMinutes(int minutes);
    int reminderMinutes() const { return m_reminderMinutes; }
    void resetNotifications() { m_notifiedKeys.clear(); }
    void checkReminders();


signals:
    void muteStateChanged(bool muted);
    void reminderTimeChanged(int minutes);

private:
    void loadSettings();
    void saveSettings();
    void showCourseReminder(Course *course, int minutesLeft);

    QSystemTrayIcon *m_trayIcon;
    QTimer *m_reminderTimer;
    bool m_isMuted;
    int m_reminderMinutes;
    ScheduleManager* m_scheduleMgr;
    QSet<QString> m_notifiedKeys;   // 存储已提醒过的“日期+节次”

    void showNotification(const QString &title, const QString &message, NotificationType type);
};

#endif // NOTIFICATION_H
