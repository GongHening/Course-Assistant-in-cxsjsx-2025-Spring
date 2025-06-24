#include "Notification.h"
#include "ScheduleManager.h"
#include <QSettings>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QWidget>

Notification::Notification(ScheduleManager* scheduleMgr, QObject *parent)
    : QObject(parent),m_scheduleMgr(scheduleMgr)
{
    loadSettings();
    QSettings settings;
    m_reminderMinutes = settings.value("Notification/ReminderMinutes", 15).toInt();
    // 托盘图标
    m_trayIcon = new QSystemTrayIcon(QIcon(":/icons/app_icon"), this);
    m_trayIcon->show();

    // 定时器：每分钟检查一次
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &Notification::checkReminders);
    m_reminderTimer->start(60 * 1000);

    // 程序启动时立即检查一次
    QTimer::singleShot(0, this, &Notification::checkReminders);
}

Notification::~Notification()
{
    // 确保停止所有活动
    if (m_reminderTimer) {
        m_reminderTimer->stop();
        delete m_reminderTimer;
    }

    if (m_trayIcon) {
        delete m_trayIcon;
    }
}
void Notification::loadSettings()
{
    QSettings settings;  // now uses YourCompany/SmartScheduleAssistant
    m_isMuted         = settings.value("Notification/Muted", false).toBool();
    m_reminderMinutes = settings.value("Notification/ReminderMinutes", 30).toInt();
}

void Notification::saveSettings()
{
    QSettings settings;  // same file
    settings.setValue("Notification/Muted", m_isMuted);
    settings.setValue("Notification/ReminderMinutes", m_reminderMinutes);
    settings.sync();
}
void Notification::setMuted(bool muted)
{
    if (m_isMuted == muted) return;
    m_isMuted = muted;
    emit muteStateChanged(muted);
}

void Notification::setReminderMinutes(int minutes)
{
    if (minutes < 1 || minutes > 720 || minutes == m_reminderMinutes)
        return;

    m_reminderMinutes = minutes;

    // 保存设置到配置文件
    QSettings settings;
    settings.setValue("Notification/ReminderMinutes", minutes);
    settings.sync();

    emit reminderTimeChanged(minutes);
}

void Notification::checkReminders()
{
    if (m_isMuted || !m_scheduleMgr) {  // 添加空指针检查
        return;
    }

    Course* next = m_scheduleMgr->getNextCourse();
    if (!next) {
        return; // 没有下一节课
    }

    // 确保课程数据有效
    if (next->startSection() < 1 || next->startSection() > 12) {
        qWarning() << "无效的课程节次:" << next->startSection();
        return;
    }

    // 计算课程开始的具体日期时间
    QDate today = QDate::currentDate();
    int currentDayOfWeek = today.dayOfWeek(); // Qt: 1=周一, 7=周日

    // 计算距离课程日期的天数
    int daysDiff = next->dayOfWeek() - currentDayOfWeek;
    if (daysDiff < 0) {
        daysDiff += 7; // 处理跨周情况
    }

    QDate courseDate = today.addDays(daysDiff);
    QTime startTime = ScheduleManager::getSectionStartTime(next->startSection());

    if (!startTime.isValid()) {
        qWarning() << "无效的课程开始时间: 节次" << next->startSection();
        return;
    }

    QDateTime courseStart(courseDate, startTime);
    QDateTime now = QDateTime::currentDateTime();

    // 计算剩余分钟数
    int secondsLeft = now.secsTo(courseStart);
    if (secondsLeft < 0) {
        return; // 课程已经开始
    }

    int minutesLeft = secondsLeft / 60;

    // 检查是否在提醒时间范围内
    if (minutesLeft > m_reminderMinutes) {
        return; // 不在提醒时间范围内
    }

    // 构造唯一键避免重复提醒（格式：日期_节次）
    QString key = courseDate.toString("yyyyMMdd") + "_" +
                  QString::number(next->startSection());

    if (m_notifiedKeys.contains(key)) {
        return; // 已经提醒过此课程
    }

    // 添加调试信息
    qDebug() << "触发课程提醒:"
             << "课程:" << next->name()
             << "日期:" << courseDate.toString("yyyy-MM-dd")
             << "时间:" << startTime.toString("hh:mm")
             << "剩余分钟:" << minutesLeft
             << "设置提醒时间:" << m_reminderMinutes;

    // 标记为已提醒
    m_notifiedKeys.insert(key);

    // 显示提醒
    showCourseReminder(next, minutesLeft);
}

void Notification::showCourseReminder(Course *course, int minutesLeft)
{
    QString title = QString("课程即将开始：%1 分钟后").arg(minutesLeft);

    QString msg = QString("课程：%1\n"
                          "教室：%2\n"
                          "时间：%3-%4\n"
                          "节次：%5-%6")
                      .arg(course->name())
                      .arg(course->classroom())
                      .arg(ScheduleManager::getSectionStartTime(course->startSection()).toString("hh:mm"))
                      .arg(ScheduleManager::getSectionEndTime(course->endSection()).toString("hh:mm"))
                      .arg(course->startSection())
                      .arg(course->endSection());

    // 显示系统托盘通知
    if (m_trayIcon->supportsMessages()) {
        m_trayIcon->showMessage(title, msg, QSystemTrayIcon::Information, 5000);
    }

    // 如果主窗口可见，也显示消息框
    QWidget *mainWindow = qApp->activeWindow();
    if (mainWindow && mainWindow->isVisible()) {
        QMessageBox::information(mainWindow, title, msg);
    }
}

void Notification::showNotification(const QString &title, const QString &message, NotificationType type)
{
    if (m_isMuted) return;
    QSystemTrayIcon::MessageIcon icon =
        (type==Warning? QSystemTrayIcon::Warning :
             type==Critical? QSystemTrayIcon::Critical :
             QSystemTrayIcon::Information);
    m_trayIcon->showMessage(title, message, icon, 5000);
}
