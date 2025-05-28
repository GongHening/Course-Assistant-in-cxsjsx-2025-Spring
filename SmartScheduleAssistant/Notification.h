#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include "Course.h"

// 将 NotificationType 枚举定义移到 Notification 类声明之前
enum NotificationType {
    Information,
    Warning,
    Critical
};

class Notification : public QObject
{
    Q_OBJECT
public:
    explicit Notification(QObject *parent = nullptr);
    ~Notification();

    // 检查课程提醒
    void checkReminder(Course *nextCourse);

    // 显示课程提醒
    void showCourseReminder(Course *course, int minutesLeft);

    // 获取节次的开始时间
    QTime getSectionStartTime(int section);

    // 显示通用通知
    void showNotification(const QString &title, const QString &message, NotificationType type);

    // 静音控制
    void setMuted(bool muted);

    // 设置提醒提前时间
    void setReminderMinutes(int minutes);

signals:
    void muteStateChanged(bool muted);
    void reminderTimeChanged(int minutes);

private:
    void loadSettings();
    void saveSettings();
    void checkReminders();

    QSystemTrayIcon *m_trayIcon;
    QTimer *m_reminderTimer;
    bool m_isMuted;
    int m_reminderMinutes;
};

#endif // NOTIFICATION_H
