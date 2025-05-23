#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QColorDialog>
#include "AddHomeworkDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化表格
    initCourseTable();
    initHomeworkTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 初始化课程表
void MainWindow::initCourseTable()
{
    QStringList headers = {"周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    ui->table->setColumnCount(7);
    ui->table->setHorizontalHeaderLabels(headers);
    ui->table->setRowCount(10); // 假设每天最多10节课

    // 填充mock数据
    for(auto &course : mockCourses){
        int col = headers.indexOf(course[1]);
        int row = course[2].left(1).toInt() - 1;

        QTableWidgetItem *item = new QTableWidgetItem(course[0]);
        item->setBackground(QColor(course[4]));
        ui->table->setItem(row, col, item);
    }
}

// 初始化作业表
void MainWindow::initHomeworkTable()
{
    QStringList hwHeaders = {"作业名称", "关联课程", "截止日期", "状态"};
    ui->homeworkTable->setColumnCount(4);
    ui->homeworkTable->setHorizontalHeaderLabels(hwHeaders);
    ui->homeworkTable->verticalHeader()->setVisible(false);

    // 填充mock数据
    int row = 0;
    for(auto &hw : mockHomeworks){
        ui->homeworkTable->insertRow(row);
        for(int col=0; col<4; col++){
            QTableWidgetItem *item = new QTableWidgetItem(hw[col]);
            if(col == 3) { // 状态列特殊样式
                item->setForeground(hw[3] == "未完成" ? Qt::red : Qt::darkGreen);
            }
            ui->homeworkTable->setItem(row, col, item);
        }
        row++;
    }
}

// 添加课程按钮
void MainWindow::on_btn_add_course_clicked()
{
    // 弹出课程信息输入对话框
    QDialog dialog(this);
    dialog.setWindowTitle("添加课程");
    // 这里应创建包含多个输入字段的自定义对话框
    // 暂时使用QInputDialog简化实现
    bool ok;
    QString name = QInputDialog::getText(this, "添加课程",
                                         "课程名称:",
                                         QLineEdit::Normal,
                                         "", &ok);
    if(ok && !name.isEmpty()){
        // 添加到表格（临时前端实现）
        int row = ui->table->currentRow();
        int col = ui->table->currentColumn();
        if(col == -1) col = 0;

        QTableWidgetItem *item = new QTableWidgetItem(name);
        item->setBackground(Qt::yellow); // 默认颜色
        ui->table->setItem(row, col, item);
    }
}

// 删除课程按钮
void MainWindow::on_btn_del_course_clicked()
{
    QList<QTableWidgetItem*> selected = ui->table->selectedItems();
    if(selected.isEmpty()){
        QMessageBox::warning(this, "警告", "请先选择要删除的课程!");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
                                    "确定要删除选中的课程吗？");
    if(ret == QMessageBox::Yes){
        foreach(QTableWidgetItem* item, selected){
            ui->table->removeRow(item->row());
        }
    }
}

// 添加作业按钮
void MainWindow::on_btn_add_hw_clicked() {
    // 获取课程列表（示例数据，需替换为实际数据源）
    QStringList courses;
    for(int i=0; i<ui->table->rowCount(); i++) {
        for(int j=0; j<ui->table->columnCount(); j++) {
            if(auto item = ui->table->item(i, j)) {
                if(!item->text().isEmpty() && !courses.contains(item->text())) {
                    courses.append(item->text());
                }
            }
        }
    }

    AddHomeworkDialog dialog(courses, this);
    if(dialog.exec() == QDialog::Accepted) {
        // 获取输入数据
        QString title = dialog.getHomeworkTitle();
        QString course = dialog.getRelatedCourse();
        QDateTime deadline = dialog.getDeadline();

        // 添加到表格（示例）
        int row = ui->homeworkTable->rowCount();
        ui->homeworkTable->insertRow(row);
        ui->homeworkTable->setItem(row, 0, new QTableWidgetItem(title));
        ui->homeworkTable->setItem(row, 1, new QTableWidgetItem(course));
        ui->homeworkTable->setItem(row, 2, new QTableWidgetItem(deadline.toString("yyyy-MM-dd HH:mm")));
        ui->homeworkTable->setItem(row, 3, new QTableWidgetItem("未完成"));
    }
}

// 删除作业按钮
void MainWindow::on_btn_del_hw_clicked()
{
    QList<QTableWidgetItem*> selected = ui->homeworkTable->selectedItems();
    if(selected.isEmpty()){
        QMessageBox::warning(this, "警告", "请先选择要删除的作业!");
        return;
    }

    int ret = QMessageBox::question(this, "确认删除",
                                    "确定要删除选中的作业吗？");
    if(ret == QMessageBox::Yes){
        QSet<int> rows;
        foreach(QTableWidgetItem* item, selected){
            rows.insert(item->row());
        }

        QList<int> rowList = rows.values();
        std::sort(rowList.begin(), rowList.end(), std::greater<int>());
        foreach(int row, rowList){
            ui->homeworkTable->removeRow(row);
        }
    }
}
