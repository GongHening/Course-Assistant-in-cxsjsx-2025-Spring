#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QList>
#include "Task.h"
#include "Course.h"

namespace Ui {
class TaskDialog;
}

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget *parent = nullptr);
    ~TaskDialog();

    void setTask(Task *task);
    Task getTask() const;

    void setCourseList(const QList<Course*> &courses);

signals:
    void taskConfirmed(const Task &task);  // 新增信号

private slots:
    void validateInput();

private:
    void setupUi(); // 添加 setupUi 函数的声明
    void updateCourseComboBox();

    Ui::TaskDialog *ui;
    QList<Course*> m_courses;
    Task *m_task = nullptr; // 用于编辑模式
};

#endif // TASKDIALOG_H
