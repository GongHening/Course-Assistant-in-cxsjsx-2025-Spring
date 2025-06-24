#include "ReminderDialog.h"
#include "ui_ReminderDialog.h"
#include <QMessageBox>
#include <QSettings>
#include <QPushButton>

ReminderDialog::ReminderDialog(int currentMinutes, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::ReminderDialog)
{
    ui->setupUi(this);
    QSettings settings;
    int savedMinutes = settings.value("Notification/ReminderMinutes", currentMinutes).toInt();
    ui->spinBoxMinutes->setValue(savedMinutes);

    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);

    ui->buttonBox->removeButton(okButton);

    ui->buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);

    connect(okButton, &QPushButton::clicked,
            this, &ReminderDialog::validateInput);

    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}

ReminderDialog::~ReminderDialog()
{
    delete ui;
}

int ReminderDialog::minutes() const
{
    return ui->spinBoxMinutes->value();
}

void ReminderDialog::validateInput()
{
    int m = ui->spinBoxMinutes->value();
    if (m < 1 || m > 720) {
        QMessageBox::warning(this,
                             tr("无效值"),
                             tr("提醒时间必须在 1–720 分钟之间"));
        return;  // 不关闭
    }
    accept();    // 合法时，真正关闭
}
