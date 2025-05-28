#include "Notification.h"
#include <QSystemTrayIcon>
#include <QTime>
#include <QSettings>
#include <QDebug>

Notification::Notification(QObject *parent)
    : QObject(parent)
{
    // 初始化系统托盘图标
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/icons/app_icon.png"));
    m_trayIcon->show();

    // 加载设置
    loadSettings();

    // 初始化提醒计时器
    m_reminderTimer = new QTimer(this);
    m_reminderTimer->setSingleShot(false);
    connect(m_reminderTimer, &QTimer::timeout, this, &Notification::checkReminders);

    // 每分钟检查一次提醒
    m_reminderTimer->start(60 * 1000);
}

Notification::~Notification()
{
    saveSettings();
}

// 检查课程提醒
void Notification::checkReminder(Course *nextCourse)
{
    if (!nextCourse || m_isMuted) return;

    QDateTime now = QDateTime::currentDateTime();
    QDate today = now.date();
    int currentDay = today.dayOfWeek(); // Qt中1=周一，7=周日
    int daysUntilCourse = nextCourse->dayOfWeek() - currentDay;

    // 处理跨周情况
    if (daysUntilCourse < 0) {
        daysUntilCourse += 7;
    }

    // 如果是今天课程，计算剩余分钟
    if (daysUntilCourse == 0) {
        // 获取课程开始时间（这里需要根据节次转换为具体时间）
        QTime courseStartTime = getSectionStartTime(nextCourse->startSection());
        int minutesLeft = now.time().msecsTo(courseStartTime) / (60 * 1000);

        // 如果正好在提醒时间范围内
        if (minutesLeft > 0 && minutesLeft <= m_reminderMinutes) {
            showCourseReminder(nextCourse, minutesLeft);
        }
    }
}

// 显示课程提醒
void Notification::showCourseReminder(Course *course, int minutesLeft)
{
    QString title = QString("课程提醒 - %1分钟后").arg(minutesLeft);
    QString message = QString("课程: %1\n教室: %2\n时间: 第%3-%4节")
                          .arg(course->name())
                          .arg(course->classroom())
                          .arg(course->startSection())
                          .arg(course->endSection());

    // 显示系统通知
    if (m_trayIcon->supportsMessages()) {
        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 5000);
    }

    qDebug() << "Showing reminder:" << title << message;
}

// 获取节次的开始时间
QTime Notification::getSectionStartTime(int section)
{
    // 这里应该与MainWindow中的节次时间表一致
    static const QVector<QTime> startTimes = {
        QTime(8, 0),    // 第1节
        QTime(8, 55),   // 第2节
        QTime(10, 0),   // 第3节
        QTime(10, 55),  // 第4节
        QTime(14, 0),   // 第5节
        QTime(14, 55),  // 第6节
        QTime(16, 0),   // 第7节
        QTime(16, 55),  // 第8节
        QTime(19, 0),   // 第9节
        QTime(19, 55),  // 第10节
        QTime(20, 50),  // 第11节
        QTime(21, 45)   // 第12节
    };

    if (section >= 1 && section <= startTimes.size()) {
        return startTimes[section - 1];
    }
    return QTime();
}

// 显示通用通知
void Notification::showNotification(const QString &title, const QString &message, NotificationType type)
{
    if (m_isMuted) return;

    QSystemTrayIcon::MessageIcon icon;
    switch (type) {
    case Information:
        icon = QSystemTrayIcon::Information;
        break;
    case Warning:
        icon = QSystemTrayIcon::Warning;
        break;
    case Critical:
        icon = QSystemTrayIcon::Critical;
        break;
    default:
        icon = QSystemTrayIcon::NoIcon;
    }

    m_trayIcon->showMessage(title, message, icon, 5000);
}

// 静音控制
void Notification::setMuted(bool muted)
{
    if (m_isMuted != muted) {
        m_isMuted = muted;
        emit muteStateChanged(m_isMuted);

        if (!m_isMuted) {
            showNotification("提示", "提醒功能已开启", Information);
        }
    }
}

// 设置提醒提前时间
void Notification::setReminderMinutes(int minutes)
{
    if (minutes >= 1 && minutes <= 60 && m_reminderMinutes != minutes) {
        m_reminderMinutes = minutes;
        emit reminderTimeChanged(m_reminderMinutes);
    }
}

// 加载设置
void Notification::loadSettings()
{
    QSettings settings;
    m_isMuted = settings.value("Notification/Muted", false).toBool();
    m_reminderMinutes = settings.value("Notification/ReminderMinutes", 15).toInt();
}

// 保存设置
void Notification::saveSettings()
{
    QSettings settings;
    settings.setValue("Notification/Muted", m_isMuted);
    settings.setValue("Notification/ReminderMinutes", m_reminderMinutes);
}

// 检查所有提醒（定时器触发）
void Notification::checkReminders()
{
    // 这里应该从ScheduleManager获取下一节课
    // 示例代码，实际应该替换为从ScheduleManager获取
    Course* nextCourse = nullptr; // 应该从ScheduleManager获取

    if (nextCourse) {
        checkReminder(nextCourse);
    }
}
