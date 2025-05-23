#ifndef ADDHOMEWORKDIALOG_H
#define ADDHOMEWORKDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QLineEdit>
#include <QComboBox>
#include <QDateTimeEdit>

namespace Ui {
class AddHomeworkDialog;
}

class AddHomeworkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddHomeworkDialog(const QStringList &courses, QWidget *parent = nullptr);
    ~AddHomeworkDialog();

    QString getHomeworkTitle() const;
    QString getRelatedCourse() const;
    QDateTime getDeadline() const;

private:
    Ui::AddHomeworkDialog *ui;
};

#endif
