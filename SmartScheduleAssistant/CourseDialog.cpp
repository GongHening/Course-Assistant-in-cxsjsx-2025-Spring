#include "CourseDialog.h"
#include "ui_CourseDialog.h"
#include <QColorDialog>
#include <QMessageBox>

CourseDialog::CourseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CourseDialog)
{
    ui->setupUi(this);

    // 初始化UI
    setupUi();

    // 连接信号槽
    connect(ui->buttonColor, &QPushButton::clicked, this, &CourseDialog::selectColor);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CourseDialog::validateInput);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

CourseDialog::~CourseDialog()
{
    delete ui;
}

// 初始化UI
void CourseDialog::setupUi()
{
    // 设置星期下拉框
    ui->comboDay->addItem("周一", 1);
    ui->comboDay->addItem("周二", 2);
    ui->comboDay->addItem("周三", 3);
    ui->comboDay->addItem("周四", 4);
    ui->comboDay->addItem("周五", 5);
    ui->comboDay->addItem("周六", 6);
    ui->comboDay->addItem("周日", 7);

    // 设置节次下拉框
    for (int i = 1; i <= Course::MAX_SECTION; ++i) {
        ui->comboStart->addItem(QString::number(i), i);
        ui->comboEnd->addItem(QString::number(i), i);
    }

    // 默认颜色
    m_color = QColor(100, 150, 255);
    updateColorButton();

    // 默认节次范围
    ui->comboStart->setCurrentIndex(0);
    ui->comboEnd->setCurrentIndex(0);
}

// 设置课程信息到对话框
void CourseDialog::setCourse(const Course &course)
{
    ui->editName->setText(course.name());
    ui->comboDay->setCurrentIndex(course.dayOfWeek() - 1);
    ui->comboStart->setCurrentIndex(course.startSection() - 1);
    ui->comboEnd->setCurrentIndex(course.endSection() - 1);
    ui->editClassroom->setText(course.classroom());
    ui->editTeacher->setText(course.teacher());
    ui->editNote->setPlainText(course.note());
    m_color = course.color();
    updateColorButton();
}

// 从对话框获取课程信息
Course CourseDialog::getCourse() const
{
    Course course;
    course.setName(ui->editName->text().trimmed());
    course.setDayOfWeek(ui->comboDay->currentData().toInt());
    course.setStartSection(ui->comboStart->currentData().toInt());
    course.setEndSection(ui->comboEnd->currentData().toInt());
    course.setClassroom(ui->editClassroom->text().trimmed());
    course.setTeacher(ui->editTeacher->text().trimmed());
    course.setNote(ui->editNote->toPlainText().trimmed());
    course.setColor(m_color);
    return course;
}

// 选择颜色
void CourseDialog::selectColor()
{
    QColor newColor = QColorDialog::getColor(m_color, this, "选择课程颜色");
    if (newColor.isValid()) {
        m_color = newColor;
        updateColorButton();
    }
}

// 更新颜色按钮显示
void CourseDialog::updateColorButton()
{
    QPixmap pixmap(16, 16);
    pixmap.fill(m_color);
    ui->buttonColor->setIcon(QIcon(pixmap));
}

// 验证输入
void CourseDialog::validateInput()
{
    // 检查必填字段
    if (ui->editName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入课程名称");
        return;
    }

    if (ui->editClassroom->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入教室");
        return;
    }

    // 检查节次范围
    int start = ui->comboStart->currentData().toInt();
    int end = ui->comboEnd->currentData().toInt();

    if (start > end) {
        QMessageBox::warning(this, "警告", "结束节次不能早于开始节次");
        return;
    }

    // 所有验证通过，接受对话框
    accept();
}
