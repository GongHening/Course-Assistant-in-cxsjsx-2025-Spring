#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CourseDialog.h"
#include "TaskDialog.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QBrush>

// 节次时间表
static const QVector<QPair<QString, QString>> SECTION_TIMES = {
    {"08:00", "08:45"},   // 第1节
    {"08:55", "09:40"},   // 第2节
    {"10:00", "10:45"},   // 第3节
    {"10:55", "11:40"},   // 第4节
    {"14:00", "14:45"},   // 第5节
    {"14:55", "15:40"},   // 第6节
    {"16:00", "16:45"},   // 第7节
    {"16:55", "17:40"},   // 第8节
    {"19:00", "19:45"},   // 第9节
    {"19:55", "20:40"},   // 第10节
    {"20:50", "21:35"},   // 第11节
    {"21:45", "22:30"}    // 第12节
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_scheduleManager(new ScheduleManager(this))
    , m_notification(new Notification(this))
{
    ui->setupUi(this);

    // 初始化UI
    setupCourseTable();
    setupTaskList();
    setupTrayIcon();
    setupConnections();

    // 启动定时器更新当前课程和下一节课
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateCurrentCourse);
    timer->start(60000); // 每分钟更新一次

    // 初始更新
    updateCourseTable();
    updateTaskList();
    updateCurrentCourse();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 初始化课程表
void MainWindow::setupCourseTable()
{
    ui->courseTable->setRowCount(SECTION_TIMES.size());
    ui->courseTable->setColumnCount(7); // 周一至周日

    // 设置表头
    QStringList headers;
    headers << "周一" << "周二" << "周三" << "周四" << "周五" << "周六" << "周日";
    ui->courseTable->setHorizontalHeaderLabels(headers);

    // 设置行头（节次和时间）
    for (int i = 0; i < SECTION_TIMES.size(); ++i) {
        QString timeText = QString("第%1节\n%2-%3")
                               .arg(i+1)
                               .arg(SECTION_TIMES[i].first)
                               .arg(SECTION_TIMES[i].second);
        ui->courseTable->setVerticalHeaderItem(i, new QTableWidgetItem(timeText));
    }

    // 表格样式设置
    ui->courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->courseTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->courseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->courseTable->setSelectionMode(QAbstractItemView::NoSelection);
}

// 初始化任务列表
void MainWindow::setupTaskList()
{
    ui->taskTable->setColumnCount(5);
    ui->taskTable->setHorizontalHeaderLabels(
        QStringList() << "状态" << "课程" << "任务" << "截止日期" << "剩余时间");
    ui->taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

// 初始化系统托盘图标
void MainWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(QIcon(":/icons/app_icon.png"), this);

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(ui->actionShowHide);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->actionExit);

    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            toggleWindowVisibility();
        }
    });
}

// 设置信号槽连接
void MainWindow::setupConnections()
{
    // 课程操作
    connect(ui->actionAddCourse, &QAction::triggered, this, &MainWindow::addCourse);
    connect(ui->actionEditCourse, &QAction::triggered, this, &MainWindow::editCourse);
    connect(ui->actionDeleteCourse, &QAction::triggered, this, &MainWindow::deleteCourse);

    // 任务操作
    connect(ui->actionAddTask, &QAction::triggered, this, &MainWindow::addTask);
    connect(ui->actionCompleteTask, &QAction::triggered, this, &MainWindow::completeTask);
    connect(ui->actionDeleteTask, &QAction::triggered, this, &MainWindow::deleteTask);

    // 窗口操作
    connect(ui->actionShowHide, &QAction::triggered, this, &MainWindow::toggleWindowVisibility);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);

    // 课程表变化时更新UI
    connect(m_scheduleManager, &ScheduleManager::coursesChanged, this, &MainWindow::updateCourseTable);
}

// 更新课程表显示
void MainWindow::updateCourseTable()
{
    // 清空表格
    for (int row = 0; row < ui->courseTable->rowCount(); ++row) {
        for (int col = 0; col < ui->courseTable->columnCount(); ++col) {
            if (ui->courseTable->item(row, col)) {
                delete ui->courseTable->item(row, col);
            }
        }
    }

    // 填充课程数据
    for (int day = 1; day <= 7; ++day) {
        auto courses = m_scheduleManager->getCoursesByDay(day);
        for (auto course : courses) {
            for (int section = course->startSection(); section <= course->endSection(); ++section) {
                int row = section - 1;
                int col = day - 1;

                QTableWidgetItem *item = new QTableWidgetItem(course->displayText());
                item->setBackground(course->color());
                item->setTextAlignment(Qt::AlignCenter);
                item->setData(Qt::UserRole, QVariant::fromValue(course));

                // 合并单元格（如果是多节连上的课程）
                if (section == course->startSection()) {
                    int span = course->duration();
                    ui->courseTable->setSpan(row, col, span, 1);
                }

                ui->courseTable->setItem(row, col, item);
            }
        }
    }
}

// 更新任务列表
void MainWindow::updateTaskList()
{
    ui->taskTable->setRowCount(0);

    // 这里应该从任务管理器获取任务列表
    // 示例代码，实际应该替换为从TaskManager获取任务
    QList<Task*> tasks; // 应该从任务管理器获取

    for (int i = 0; i < tasks.size(); ++i) {
        Task *task = tasks[i];

        ui->taskTable->insertRow(i);

        // 状态图标
        QTableWidgetItem *statusItem = new QTableWidgetItem();
        statusItem->setBackground(task->priorityColor());
        if (task->isCompleted()) {
            statusItem->setIcon(QIcon(":/icons/checked.png"));
        } else if (task->isExam()) {
            statusItem->setIcon(QIcon(":/icons/exam.png"));
        } else {
            statusItem->setIcon(QIcon(":/icons/homework.png"));
        }

        // 课程名称
        QTableWidgetItem *courseItem = new QTableWidgetItem(task->courseName());

        // 任务标题
        QTableWidgetItem *titleItem = new QTableWidgetItem(task->title());

        // 截止日期
        QTableWidgetItem *dateItem = new QTableWidgetItem(task->dueDate().toString("yyyy-MM-dd"));

        // 剩余时间
        QTableWidgetItem *daysItem = new QTableWidgetItem(task->statusText());

        // 设置数据关联
        statusItem->setData(Qt::UserRole, QVariant::fromValue(task));
        courseItem->setData(Qt::UserRole, QVariant::fromValue(task));
        titleItem->setData(Qt::UserRole, QVariant::fromValue(task));
        dateItem->setData(Qt::UserRole, QVariant::fromValue(task));
        daysItem->setData(Qt::UserRole, QVariant::fromValue(task));

        // 添加到表格
        ui->taskTable->setItem(i, 0, statusItem);
        ui->taskTable->setItem(i, 1, courseItem);
        ui->taskTable->setItem(i, 2, titleItem);
        ui->taskTable->setItem(i, 3, dateItem);
        ui->taskTable->setItem(i, 4, daysItem);
    }
}

// 更新当前课程和下一节课显示
void MainWindow::updateCurrentCourse()
{
    Course *current = m_scheduleManager->getCurrentCourse();
    Course *next = m_scheduleManager->getNextCourse();

    if (current) {
        ui->currentCourseLabel->setText(
            QString("当前: %1\n%2 @ %3")
                .arg(current->name())
                .arg(current->classroom())
                .arg(SECTION_TIMES[current->startSection()-1].first));
    } else {
        ui->currentCourseLabel->setText("当前无课程");
    }

    if (next) {
        QString dayStr;
        switch(next->dayOfWeek()) {
        case 1: dayStr = "周一"; break;
        case 2: dayStr = "周二"; break;
        case 3: dayStr = "周三"; break;
        case 4: dayStr = "周四"; break;
        case 5: dayStr = "周五"; break;
        case 6: dayStr = "周六"; break;
        case 7: dayStr = "周日"; break;
        }

        ui->nextCourseLabel->setText(
            QString("下一节: %1\n%2 %3 @ %4")
                .arg(next->name())
                .arg(dayStr)
                .arg(SECTION_TIMES[next->startSection()-1].first)
                .arg(next->classroom()));
    } else {
        ui->nextCourseLabel->setText("今天没有更多课程了");
    }

    // 检查是否需要发送上课提醒
    if (next) {
        m_notification->checkReminder(next);
    }
}

// 添加课程
void MainWindow::addCourse()
{
    CourseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Course course = dialog.getCourse();
        if (!m_scheduleManager->addCourse(course)) {
            QMessageBox::warning(this, "冲突", "该时间段已有其他课程");
        }
    }
}

// 编辑课程
void MainWindow::editCourse()
{
    // 获取选中的课程
    QList<QTableWidgetItem*> items = ui->courseTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要编辑的课程");
        return;
    }

    Course *selectedCourse = items.first()->data(Qt::UserRole).value<Course*>();
    if (!selectedCourse) return;

    CourseDialog dialog(this);
    dialog.setCourse(*selectedCourse);

    if (dialog.exec() == QDialog::Accepted) {
        Course newCourse = dialog.getCourse();
        if (!m_scheduleManager->editCourse(m_scheduleManager->getAllCourses().indexOf(selectedCourse), newCourse)) {
            QMessageBox::warning(this, "冲突", "该时间段已有其他课程");
        }
    }
}

// 删除课程
void MainWindow::deleteCourse()
{
    QList<QTableWidgetItem*> items = ui->courseTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要删除的课程");
        return;
    }

    Course *selectedCourse = items.first()->data(Qt::UserRole).value<Course*>();
    if (!selectedCourse) return;

    if (QMessageBox::question(this, "确认", "确定要删除这门课程吗？") == QMessageBox::Yes) {
        m_scheduleManager->removeCourse(m_scheduleManager->getAllCourses().indexOf(selectedCourse));
    }
}

// 添加任务
void MainWindow::addTask()
{
    TaskDialog dialog(this);
    dialog.setCourseList(m_scheduleManager->getAllCourses());

    if (dialog.exec() == QDialog::Accepted) {
        Task *task = dialog.getTask();
        // 这里应该将任务添加到任务管理器
        // m_taskManager->addTask(task);
        updateTaskList();
    }
}

// 标记任务完成
void MainWindow::completeTask()
{
    int row = ui->taskTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择要标记的任务");
        return;
    }

    QTableWidgetItem *item = ui->taskTable->item(row, 0);
    Task *task = item->data(Qt::UserRole).value<Task*>();
    if (task) {
        task->setCompleted(!task->isCompleted());
        updateTaskList();
    }
}

// 删除任务
void MainWindow::deleteTask()
{
    int row = ui->taskTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先选择要删除的任务");
        return;
    }

    if (QMessageBox::question(this, "确认", "确定要删除这个任务吗？") == QMessageBox::Yes) {
        QTableWidgetItem *item = ui->taskTable->item(row, 0);
        Task *task = item->data(Qt::UserRole).value<Task*>();
        if (task) {
            // 这里应该从任务管理器删除任务
            // m_taskManager->removeTask(task);
            updateTaskList();
        }
    }
}

// 切换窗口显示/隐藏
void MainWindow::toggleWindowVisibility()
{
    if (isVisible()) {
        hide();
    } else {
        show();
        activateWindow();
        raise();
    }
}

// 关闭事件处理
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}
