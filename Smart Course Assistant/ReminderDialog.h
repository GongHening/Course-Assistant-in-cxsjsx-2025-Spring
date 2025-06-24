#ifndef REMINDERDIALOG_H
#define REMINDERDIALOG_H

#include <QDialog>

namespace Ui {
class ReminderDialog;
}

class ReminderDialog : public QDialog
{
    Q_OBJECT

public:
    ReminderDialog(int currentMinutes, QWidget *parent = nullptr);
    ~ReminderDialog();

    /// 返回用户选择的分钟数
    int minutes() const;
private:
    Ui::ReminderDialog *ui;
    void validateInput();

};

#endif // REMINDERDIALOG_H
