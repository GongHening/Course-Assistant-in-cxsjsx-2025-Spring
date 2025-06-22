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
    {"08:00", "08:50"},   // 第1节
    {"09:00", "09:50"},   // 第2节
    {"10:10", "11:00"},   // 第3节
    {"11:10", "12:00"},   // 第4节
    {"13:00", "13:50"},   // 第5节
    {"14:00", "14:50"},   // 第6节
    {"15:10", "16:00"},   // 第7节
    {"16:10", "17:00"},   // 第8节
    {"17:10", "18:00"},   // 第9节
    {"18:40", "19:30"},   // 第10节
    {"19:40", "20:30"},   // 第11节
    {"20:40", "21:30"}    // 第12节
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(new Settings(this)),  // 确保 Settings 最先初始化
    m_notification(new Notification(this)),
    m_scheduleManager(new ScheduleManager(this)),
    m_taskManager(new TaskManager(this)),
    m_trayIcon(nullptr)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/app_icon"));

    // 1. 加载数据
    m_scheduleManager->loadCourses();
    m_taskManager->loadTasks();

    // 2. 初始化UI组件
    setupCourseTable();
    setupTaskList();

    // 3. 初始化系统托盘 - 必须在所有依赖项之后
    setupTrayIcon();  // 创建托盘图标
    setWindowFlags(windowFlags() & ~Qt::Tool);

    // 4. 设置信号连接
    setupConnections();

        // 额外连接：使用专门的槽处理托盘激活（双击唤回窗口）
    connect(m_trayIcon, &QSystemTrayIcon::activated,
                            this, &MainWindow::onTrayIconActivated);
    // 5. 启动定时器
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateCurrentCourse);
    connect(m_taskManager, &TaskManager::tasksChanged, this, &MainWindow::updateTaskList);
    timer->start(60000); // 每分钟更新一次

    // 6. 自动保存定时器
    QTimer *autoSaveTimer = new QTimer(this);
    connect(autoSaveTimer, &QTimer::timeout, this, [this]() {
        m_scheduleManager->saveCourses();
        m_taskManager->saveTasks();
        qDebug() << "数据已自动保存";
    });
    autoSaveTimer->start(5 * 60 * 1000); // 每5分钟自动保存

    // 7. 初始更新
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
    ui->courseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->courseTable->setSelectionBehavior(QAbstractItemView::SelectItems);

    // 连接单元格点击事件
    connect(ui->courseTable, &QTableWidget::cellClicked, this, &MainWindow::selectEntireCourseSpan);

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
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(this, tr("错误"), tr("系统不支持系统托盘"));
        return;
    }

    if (m_trayIcon)
        return;

    m_trayIcon = new QSystemTrayIcon(this);

    // 强化图标加载判断
    QIcon icon(":/icons/app_icon");
    if (icon.isNull()) {
        qDebug() << "图标资源未加载成功，尝试使用默认图标";
        icon = QApplication::windowIcon();  // fallback
    }
    m_trayIcon->setIcon(icon);
    m_trayIcon->setVisible(true);

    m_trayIcon->setToolTip(tr("智能课程表助手"));

    // 托盘菜单
    m_trayMenu = new QMenu(this);

    m_actionToggleVisibility = new QAction(tr("显示/隐藏窗口"), this);
    connect(m_actionToggleVisibility, &QAction::triggered, this, &MainWindow::toggleWindowVisibility);
    m_trayMenu->addAction(m_actionToggleVisibility);

    m_trayMenu->addSeparator();

    m_actionExit = new QAction(tr("退出"), this);
    connect(m_actionExit, &QAction::triggered, qApp, &QApplication::quit);
    m_trayMenu->addAction(m_actionExit);

    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayIconActivated);

    m_trayIcon->show();

    qDebug() << "托盘图标是否可见: " << m_trayIcon->isVisible();
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
    connect(ui->actionExit, &QAction::triggered, this, [this] {
        // 保存数据
        m_scheduleManager->saveCourses();
        m_taskManager->saveTasks();

        // 隐藏托盘图标
        m_trayIcon->hide();

        // 真正退出程序
        qApp->quit();
    });

    // 课程表变化时更新UI
    connect(m_scheduleManager, &ScheduleManager::coursesChanged, this, &MainWindow::updateCourseTable);
}

// 更新课程表显示
void MainWindow::updateCourseTable()
{
    // 清空表格
    ui->courseTable->clearSpans();
    ui->courseTable->clearContents();

    // 填充课程数据
    for (int day = 1; day <= 7; ++day) {
        auto courses = m_scheduleManager->getCoursesByDay(day);
        for (auto course : courses) {
            int startSection = course->startSection();
            int endSection = course->endSection();
            int row = startSection - 1;
            int col = day - 1;
            int duration = endSection - startSection + 1;

            // 只对多节课程设置跨度
            if (duration > 1) {
                ui->courseTable->setSpan(row, col, duration, 1);
            }

            // 只在开始节次创建单元格
            QTableWidgetItem *item = new QTableWidgetItem(course->displayText());
            item->setBackground(course->color());
            item->setTextAlignment(Qt::AlignCenter);
            item->setData(Qt::UserRole, QVariant::fromValue(course));
            ui->courseTable->setItem(row, col, item);
        }
    }
}

// 更新任务列表
// --- MainWindow.cpp ---
void MainWindow::updateTaskList()
{
    ui->taskTable->setRowCount(0);

    // 从任务管理器获取所有任务
    QList<Task*> tasks = m_taskManager->getAllTasks();

    for (int i = 0; i < tasks.size(); ++i) {
        Task *task = tasks[i];

        ui->taskTable->insertRow(i);

        // 状态图标
        QTableWidgetItem *statusItem = new QTableWidgetItem();
        statusItem->setBackground(task->priorityColor());
        if (task->isCompleted()) {
            statusItem->setIcon(QIcon(":/icons/checked"));
        } else if (task->isExam()) {
            statusItem->setIcon(QIcon(":/icons/exam"));
        } else {
            statusItem->setIcon(QIcon(":/icons/homework"));
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
    //获取所有选中单元格
    QList<QTableWidgetItem*> items = ui->courseTable->selectedItems();
    if (items.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要删除的课程");
        return;
    }

    // 关键：取第一个选中项（跨行课程的左上角单元格）
    QTableWidgetItem *item = items.first();
    Course *selectedCourse = item->data(Qt::UserRole).value<Course*>();
    if (!selectedCourse) return;

    if (QMessageBox::question(this, "确认", "确定要删除这门课程吗？") == QMessageBox::Yes) {
        m_scheduleManager->removeCourse(m_scheduleManager->getAllCourses().indexOf(selectedCourse));
    }
}

//槽函数，在点击时自动选中整个合并区域
void MainWindow::selectEntireCourseSpan(int row, int col) {
    // 获取点击位置的跨度信息（如果跨行）
    int rowSpan = ui->courseTable->rowSpan(row, col);
    int colSpan = ui->courseTable->columnSpan(row, col);

    if (rowSpan > 1 || colSpan > 1) {
        ui->courseTable->clearSelection();
        for (int r = row; r < row + rowSpan; ++r) {
            for (int c = col; c < col + colSpan; ++c) {
                if (ui->courseTable->item(r, c)) {
                    ui->courseTable->item(r, c)->setSelected(true);
                }
            }
        }
    }
}

// 添加任务
void MainWindow::addTask()
{
    TaskDialog dialog(this);
    dialog.setCourseList(m_scheduleManager->getAllCourses());

    if (dialog.exec() == QDialog::Accepted) {
        Task *task = dialog.getTask();
        m_taskManager->addTask(task); // 将任务添加到任务管理器
    }
}

// --- MainWindow.cpp ---
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
        m_taskManager->setTaskCompleted(task, !task->isCompleted());
    }
}

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
            m_taskManager->removeTask(task);
        }
    }
}
// 切换窗口显示/隐藏
void MainWindow::toggleWindowVisibility()
{
    if (isVisible()) {
        hide();
    } else {
        showNormal();
        activateWindow();
        raise();
    }
}

// 关闭事件处理
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
        showTrayMessage("提示", "程序已最小化到托盘，可通过托盘图标重新打开。");
    } else {
        event->accept();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent *stateEvent = static_cast<QWindowStateChangeEvent*>(event);

        // 正确处理最小化事件
        if (isMinimized()) {
            showMinimized(); // 完全隐藏窗口
            qDebug() << "窗口最小化到托盘";
            event->accept(); // 接受事件处理
            return;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        restoreFromTray();
    }
}
void MainWindow::showTrayMessage(const QString &title, const QString &message)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }
}


void MainWindow::restoreFromTray()
{
    if (!isVisible()) {
        // 1. 清除最小化标志
        Qt::WindowStates st = windowState();
        st &= ~Qt::WindowMinimized;
        setWindowState(st);

        // 2. 恢复并激活窗口
        show();
        raise();
        activateWindow();
    }
}




