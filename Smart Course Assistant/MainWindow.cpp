#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CourseDialog.h"
#include "TaskDialog.h"
#include "ReminderDialog.h"
#include "ScheduleManager.h"
#include <QSettings>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTimer>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QBrush>
#include <QInputDialog>

// 节次时间表
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(nullptr)
    , m_scheduleManager(nullptr)
    , m_taskManager(nullptr)
    , m_notification(nullptr)
    , m_trayIcon(nullptr)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/app_icon"));

    try {
        // 按正确顺序初始化
        m_settings = new Settings(this);
        m_scheduleManager = new ScheduleManager(this);
        m_taskManager = new TaskManager(this);

        // 确保在创建 Notification 前 ScheduleManager 已初始化
        if (m_scheduleManager) {
            m_notification = new Notification(m_scheduleManager, this);
        } else {
            qCritical() << "Failed to create ScheduleManager";
            return;
        }

        // 加载数据
        m_scheduleManager->loadCourses();
        m_taskManager->loadTasks();

        // 初始化通知系统
        if (m_notification) {
            m_notification->resetNotifications();
            int savedMinutes = m_settings->reminderMinutes();
            m_notification->setReminderMinutes(savedMinutes);
        }

        // 初始化UI
        setupCourseTable();
        setupTaskList();
        setupTrayIcon();
        setupConnections();

        // 启动定时器
        QTimer *updateTimer = new QTimer(this);
        connect(updateTimer, &QTimer::timeout, this, [this]() {
            updateCurrentCourse();
            if (m_notification) {
                m_notification->checkReminders();
            }
        });
        updateTimer->start(60 * 1000);

        // 首次更新
        updateCourseTable();
        updateTaskList();
        updateCurrentCourse();

    } catch (const std::exception& e) {
        qCritical() << "初始化失败:" << e.what();
        QMessageBox::critical(this, "致命错误", "程序初始化失败，请重启应用。");
    }
}

MainWindow::~MainWindow()
{
    // 保存设置
    if (m_settings) {
        m_settings->setWindowGeometry(saveGeometry());
        m_settings->setWindowState(saveState());
    }
    if (m_taskManager) {
        m_taskManager->saveTasks();
    }

    delete ui;
}

// 初始化课程表
void MainWindow::setupCourseTable()
{
    // 使用 ScheduleManager 获取节次数
    int sectionCount = ScheduleManager::getSectionTimes().size();
    ui->courseTable->setRowCount(sectionCount);
    ui->courseTable->setColumnCount(7); // 周一至周日

    // 设置表头
    QStringList headers;
    headers << "周一" << "周二" << "周三" << "周四" << "周五" << "周六" << "周日";
    ui->courseTable->setHorizontalHeaderLabels(headers);

    // 设置行头（节次和时间）
    for (int i = 0; i < sectionCount; ++i) {
        int section = i + 1;
        QTime startTime = ScheduleManager::getSectionStartTime(section);
        QTime endTime = ScheduleManager::getSectionEndTime(section);

        QString timeText = QString("第%1节\n%2-%3")
                               .arg(section)
                               .arg(startTime.toString("hh:mm"))
                               .arg(endTime.toString("hh:mm"));

        ui->courseTable->setVerticalHeaderItem(i, new QTableWidgetItem(timeText));
    }

    // 表格样式设置
    ui->courseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->courseTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->courseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->courseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->courseTable->setSelectionBehavior(QAbstractItemView::SelectItems);

    // 连接单元格点击事件
    connect(ui->courseTable, &QTableWidget::cellClicked,
            this, &MainWindow::selectEntireCourseSpan);
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
        if (m_scheduleManager) m_scheduleManager->saveCourses();
        if (m_taskManager) m_taskManager->saveTasks();

        // 隐藏托盘图标
        if (m_trayIcon) m_trayIcon->hide();

        // 退出程序
        qApp->quit();
    });

    // 课程表变化时更新UI
    if (m_scheduleManager) {
        connect(m_scheduleManager, &ScheduleManager::coursesChanged,
                this, &MainWindow::updateCourseTable);
    }

    // 安全连接设置提醒动作
    if (ui->actionSetReminder) {
        connect(ui->actionSetReminder, &QAction::triggered,
                this, &MainWindow::slotSetReminder);
    }
    if (m_taskManager) {
        connect(m_taskManager, &TaskManager::tasksChanged,
                this, &MainWindow::updateTaskList);
    }
}

// 更新课程表显示
void MainWindow::updateCourseTable()
{
    if (!ui || !ui->courseTable || !m_scheduleManager) {
        qWarning() << "UI组件或计划管理器未初始化";
        return;
    }

    // 清空表格
    ui->courseTable->clearSpans();
    ui->courseTable->clearContents();

    // 填充课程数据
    for (int day = 1; day <= 7; ++day) {
        auto courses = m_scheduleManager->getCoursesByDay(day);
        for (auto course : courses) {
            if (!course) continue; // 跳过空指针

            int startSection = course->startSection();
            int endSection = course->endSection();

            // 验证节次范围
            if (startSection < 1 || endSection < startSection) {
                qWarning() << "无效的节次范围:" << startSection << "-" << endSection;
                continue;
            }

            int row = startSection - 1;
            int col = day - 1;

            // 验证表格索引
            if (row < 0 || row >= ui->courseTable->rowCount() ||
                col < 0 || col >= ui->courseTable->columnCount()) {
                qWarning() << "无效的表格位置:" << row << "," << col;
                continue;
            }

            int duration = endSection - startSection + 1;

            // 设置单元格
            QTableWidgetItem *item = new QTableWidgetItem(course->displayText());
            item->setBackground(course->color());
            item->setTextAlignment(Qt::AlignCenter);
            item->setData(Qt::UserRole, QVariant::fromValue(course));
            ui->courseTable->setItem(row, col, item);

            // 设置跨度
            if (duration > 1) {
                ui->courseTable->setSpan(row, col, duration, 1);
            }
        }
    }
}

// 更新任务列表
void MainWindow::updateTaskList()
{
    if (!ui || !ui->taskTable) {
        qWarning() << "UI组件未初始化";
        return;
    }

    // 清空表格
    ui->taskTable->setRowCount(0);

    if (!m_taskManager) {
        qWarning() << "任务管理器未初始化";
        return;
    }

    // 获取所有任务
    QList<Task*> tasks = m_taskManager->getAllTasks();

    for (int i = 0; i < tasks.size(); ++i) {
        Task *task = tasks[i];
        if (!task) {
            qWarning() << "跳过空任务";
            continue;
        }

        ui->taskTable->insertRow(i);

        // 状态图标
        QTableWidgetItem *statusItem = new QTableWidgetItem();
        statusItem->setBackground(task->priorityColor());

        // 设置图标
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
        QVariant taskVariant = QVariant::fromValue(task);
        statusItem->setData(Qt::UserRole, taskVariant);
        courseItem->setData(Qt::UserRole, taskVariant);
        titleItem->setData(Qt::UserRole, taskVariant);
        dateItem->setData(Qt::UserRole, taskVariant);
        daysItem->setData(Qt::UserRole, taskVariant);

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
        // 使用 ScheduleManager 获取节次时间
        QTime startTime = ScheduleManager::getSectionStartTime(current->startSection());
        ui->currentCourseLabel->setText(
            QString("当前: %1\n%2 @ %3")
                .arg(current->name())
                .arg(current->classroom())
                .arg(startTime.toString("hh:mm")));
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

        // 使用 ScheduleManager 获取节次时间
        QTime startTime = ScheduleManager::getSectionStartTime(next->startSection());
        ui->nextCourseLabel->setText(
            QString("下一节: %1\n%2 %3 @ %4")
                .arg(next->name())
                .arg(dayStr)
                .arg(startTime.toString("hh:mm"))
                .arg(next->classroom()));
    } else {
        ui->nextCourseLabel->setText("今天没有更多课程了");
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
void MainWindow::selectEntireCourseSpan(int row, int col)
{
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
    // 获取当前选中行
    int currentRow = ui->taskTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, "提示", "请先选择要删除的任务");
        return;
    }

    // 获取任务指针
    QTableWidgetItem *item = ui->taskTable->item(currentRow, 0);
    if (!item) {
        qWarning() << "无法获取任务项";
        return;
    }

    Task *task = item->data(Qt::UserRole).value<Task*>();
    if (!task) {
        qWarning() << "无效的任务指针";
        return;
    }

    // 确认删除
    if (QMessageBox::question(this, "确认", "确定要删除这个任务吗？") == QMessageBox::Yes)
    {
        // 先移除表格行
        ui->taskTable->removeRow(currentRow);

        // 然后从任务管理器中删除任务
        if (m_taskManager) {
            m_taskManager->removeTask(task);
        }

        // 清除当前选中项
        ui->taskTable->clearSelection();
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

int MainWindow::loadReminderTime() const
{
    QSettings settings("YourCompany", "SmartScheduleAssistant");
    // 默认15分钟
    return settings.value("reminderTime", 15).toInt();
}

void MainWindow::saveReminderTime(int minutes) const
{
    QSettings settings("YourCompany", "SmartScheduleAssistant");
    settings.setValue("reminderTime", minutes);
}

void MainWindow::slotSetReminder()
{
    if (!m_notification) {
        qWarning() << "Notification is not initialized";
        return;
    }

    // 直接从设置获取当前值
    int current = m_settings->reminderMinutes();
    ReminderDialog dlg(current, this);

    if (dlg.exec() == QDialog::Accepted) {
        int m = dlg.minutes();
        m_settings->setReminderMinutes(m);  // 通过设置类保存

        QMessageBox::information(
            this,
            "设置已保存",
            QString("已将提前提醒时间设置为 %1 分钟。").arg(m)
            );
    }
}
