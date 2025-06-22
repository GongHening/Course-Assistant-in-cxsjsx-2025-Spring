#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "ScheduleManager.h"
#include "Notification.h"
#include "TaskManager.h"
#include "Settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

public slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleWindowVisibility();
    void showTrayMessage(const QString &title, const QString &message);
    void restoreFromTray();

private:
    void setupTrayIcon();
    Ui::MainWindow *ui;
    QMenu *m_trayMenu;
    QAction *m_actionToggleVisibility;
    QAction *m_actionExit;
    // 声明顺序必须与初始化列表一致
    Settings *m_settings;
    Notification *m_notification;
    ScheduleManager *m_scheduleManager;
    TaskManager *m_taskManager;
    QSystemTrayIcon *m_trayIcon;

    void setupCourseTable();
    void setupTaskList();
    void setupConnections();

    void updateCourseTable();
    void updateTaskList();
    void updateCurrentCourse();

    void addCourse();
    void editCourse();
    void deleteCourse();
    void selectEntireCourseSpan(int row, int col);

    void addTask();
    void completeTask();
    void deleteTask();
};

#endif // MAINWINDOW_H
