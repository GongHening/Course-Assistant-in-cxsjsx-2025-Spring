#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QColor>
#include <QByteArray>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);

    // 通知设置
    int reminderMinutes() const;
    bool isMuted() const;

    // 界面设置
    QString theme() const;
    QByteArray windowGeometry() const;
    QByteArray windowState() const;

    // 课程相关设置
    QColor lastUsedCourseColor() const;

    // 数据文件设置
    QString dataFilePath() const;

public slots:
    void setReminderMinutes(int minutes);
    void setMuted(bool muted);
    void setTheme(const QString &theme);
    void setWindowGeometry(const QByteArray &geometry);
    void setWindowState(const QByteArray &state);
    void setLastUsedCourseColor(const QColor &color);
    void setDataFilePath(const QString &path);
    void resetToDefaults();

signals:
    void reminderMinutesChanged(int minutes);
    void muteStateChanged(bool muted);
    void themeChanged(const QString &theme);
    void settingsReset();

private:
    QSettings *m_settings;
};

#endif // SETTINGS_H
