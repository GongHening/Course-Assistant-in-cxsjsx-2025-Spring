#include "AddHomeworkDialog.h"
#include "ui_AddHomeworkDialog.h" 

// 构造函数：初始化界面和数据
AddHomeworkDialog::AddHomeworkDialog(const QStringList &courses, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddHomeworkDialog)
{
    ui->setupUi(this); // 关键！加载UI布局

    // 初始化课程下拉框
    ui->comboCourses->addItems(courses);

    // 设置默认时间（当前时间+7天）
    ui->editDeadline->setDateTime(QDateTime::currentDateTime().addDays(7));

    // 启用日历弹窗
    ui->editDeadline->setCalendarPopup(true);

    // 设置对话框标题（中文显示）
    setWindowTitle("添加新作业");
}

// 析构函数
AddHomeworkDialog::~AddHomeworkDialog()
{
    delete ui;
}

QString AddHomeworkDialog::getHomeworkTitle() const {
    return ui->editTitle->text().trimmed();
}

QString AddHomeworkDialog::getRelatedCourse() const {
    return ui->comboCourses->currentText();
}

QDateTime AddHomeworkDialog::getDeadline() const {
    return ui->editDeadline->dateTime();
}
