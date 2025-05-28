#include "TaskDialog.h"
#include "ui_TaskDialog.h"
#include <QMessageBox>
#include <QDate>

TaskDialog::TaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskDialog)
{
    // 调用 Qt 自动生成的 setupUi 函数来初始化 UI
    ui->setupUi(this);

    // 调用自定义的 setupUi 函数来进行额外的 UI 初始化
    this->setupUi();

    // 连接信号槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TaskDialog::validateInput);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, [this](const QDate &date) {
        if (date < QDate::currentDate()) {
            ui->dateEdit->setDate(QDate::currentDate());
        }
    });
}

TaskDialog::~TaskDialog()
{
    delete ui;
}

// 初始化UI
void TaskDialog::setupUi()
{
    // 设置日期范围（今天到一年后）
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setMinimumDate(QDate::currentDate());
    ui->dateEdit->setMaximumDate(QDate::currentDate().addYears(1));

    // 设置时间默认为 23:59
    ui->timeEdit->setTime(QTime(23, 59));

    // 初始化课程下拉框
    updateCourseComboBox();
}

// 设置任务信息到对话框
void TaskDialog::setTask(Task *task)
{
    if (!task) return;

    ui->editTitle->setText(task->title());

    // 设置关联课程
    int index = ui->comboCourse->findText(task->courseName());
    if (index >= 0) {
        ui->comboCourse->setCurrentIndex(index);
    }

    // 设置日期时间
    ui->dateEdit->setDate(task->dueDate());
    ui->timeEdit->setTime(QTime(23, 59)); // 默认时间

    // 设置任务类型
    ui->checkExam->setChecked(task->isExam());

    // 设置描述
    ui->editDescription->setPlainText(task->description());

    // 存储原始任务指针（用于编辑模式）
    m_task = task;
}

// 从对话框获取任务信息
Task TaskDialog::getTask() const
{
    Task task;

    task.setTitle(ui->editTitle->text().trimmed());
    task.setCourseName(ui->comboCourse->currentText());
    task.setDueDate(ui->dateEdit->date());
    task.setExam(ui->checkExam->isChecked());
    task.setDescription(ui->editDescription->toPlainText().trimmed());

    return task;
}

// 设置课程列表
void TaskDialog::setCourseList(const QList<Course*> &courses)
{
    m_courses = courses;
    updateCourseComboBox();
}

// 更新课程下拉框
void TaskDialog::updateCourseComboBox()
{
    ui->comboCourse->clear();
    ui->comboCourse->addItem("(无关联课程)", "");

    for (const auto &course : m_courses) {
        ui->comboCourse->addItem(course->name(), course->name());
    }
}

// 验证输入
void TaskDialog::validateInput()
{
    // 检查必填字段
    if (ui->editTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入任务标题");
        return;
    }

    // 检查截止日期
    QDateTime dueDateTime(ui->dateEdit->date(), ui->timeEdit->time());
    if (dueDateTime < QDateTime::currentDateTime()) {
        QMessageBox::warning(this, "警告", "截止时间不能早于当前时间");
        return;
    }

     // 发射信号（传递新任务数据）
    emit taskConfirmed(getTask());
    
    // 所有验证通过，接受对话框
    accept();
}
