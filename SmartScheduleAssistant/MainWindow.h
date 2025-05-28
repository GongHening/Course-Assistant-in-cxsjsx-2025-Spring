#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "ScheduleManager.h"
#include "Notification.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateCourseTable();
    void updateTaskList();
    void updateCurrentCourse();

    void addCourse();
    void editCourse();
    void deleteCourse();

    void addTask();
    void completeTask();
    void deleteTask();

    void toggleWindowVisibility();

private:
    void setupCourseTable();
    void setupTaskList();
    void setupTrayIcon();
    void setupConnections();

    Ui::MainWindow *ui;
    ScheduleManager *m_scheduleManager;
    Notification *m_notification;
    QSystemTrayIcon *m_trayIcon;
};

#endif // MAINWINDOW_H
