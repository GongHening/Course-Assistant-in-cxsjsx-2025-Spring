#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 课程表相关操作
    void on_btn_add_course_clicked();
    void on_btn_del_course_clicked();

    // 作业管理相关操作
    void on_btn_add_hw_clicked();
    void on_btn_del_hw_clicked();

private:
    Ui::MainWindow *ui;

    // 初始化界面
    void initCourseTable();
    void initHomeworkTable();

    // 临时mock数据（后续替换为后端接口）
    QList<QStringList> mockCourses = {
        {"高等数学", "周一", "3-4节", "A201", "#FFCC99"},
        {"大学英语", "周三", "1-2节", "B305", "#CCFF99"}
    };

    QList<QStringList> mockHomeworks = {
        {"完成习题集", "高等数学", "2024-06-30", "未完成"},
        {"准备presentation", "大学英语", "2024-06-25", "进行中"}
    };
};

#endif
