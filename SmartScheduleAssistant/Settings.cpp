#include "Settings.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

// 默认配置值
namespace {
const int DEFAULT_REMINDER_MINUTES = 15;
const bool DEFAULT_MUTE_STATE = false;
const QString DEFAULT_THEME = "light";
const QByteArray DEFAULT_WINDOW_GEOMETRY;
}

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
                                   "/config.ini", QSettings::IniFormat, this))
{
    // 确保配置目录存在
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!configDir.exists()) {
        configDir.mkpath(".");
    }
}

// 获取/设置提醒时间
int Settings::reminderMinutes() const
{
    return m_settings->value("Notification/ReminderMinutes", DEFAULT_REMINDER_MINUTES).toInt();
}

void Settings::setReminderMinutes(int minutes)
{
    if (minutes != reminderMinutes()) {
        m_settings->setValue("Notification/ReminderMinutes", minutes);
        emit reminderMinutesChanged(minutes);
    }
}

// 获取/设置静音状态
bool Settings::isMuted() const
{
    return m_settings->value("Notification/Muted", DEFAULT_MUTE_STATE).toBool();
}

void Settings::setMuted(bool muted)
{
    if (muted != isMuted()) {
        m_settings->setValue("Notification/Muted", muted);
        emit muteStateChanged(muted);
    }
}

// 获取/设置主题
QString Settings::theme() const
{
    return m_settings->value("Interface/Theme", DEFAULT_THEME).toString();
}

void Settings::setTheme(const QString &theme)
{
    if (theme != this->theme()) {
        m_settings->setValue("Interface/Theme", theme);
        emit themeChanged(theme);
    }
}

// 获取/设置窗口几何信息
QByteArray Settings::windowGeometry() const
{
    return m_settings->value("Window/Geometry", DEFAULT_WINDOW_GEOMETRY).toByteArray();
}

void Settings::setWindowGeometry(const QByteArray &geometry)
{
    m_settings->setValue("Window/Geometry", geometry);
}

// 获取/设置窗口状态
QByteArray Settings::windowState() const
{
    return m_settings->value("Window/State", QByteArray()).toByteArray();
}

void Settings::setWindowState(const QByteArray &state)
{
    m_settings->setValue("Window/State", state);
}

// 获取/设置最近使用的课程颜色
QColor Settings::lastUsedCourseColor() const
{
    return m_settings->value("Course/LastColor", QColor(100, 150, 255)).value<QColor>();
}

void Settings::setLastUsedCourseColor(const QColor &color)
{
    m_settings->setValue("Course/LastColor", color);
}

// 获取/设置数据文件路径
QString Settings::dataFilePath() const
{
    return m_settings->value("Data/FilePath",
                             QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                 "/schedule.dat").toString();
}

void Settings::setDataFilePath(const QString &path)
{
    m_settings->setValue("Data/FilePath", path);
}

// 重置所有设置为默认值
void Settings::resetToDefaults()
{
    m_settings->clear();
    emit settingsReset();
}
