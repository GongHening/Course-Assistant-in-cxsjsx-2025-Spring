#ifndef COURSEDIALOG_H
#define COURSEDIALOG_H

#include <QDialog>
#include "Course.h"

namespace Ui {
class CourseDialog;
}

class CourseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CourseDialog(QWidget *parent = nullptr);
    ~CourseDialog();

    void setCourse(const Course &course);
    Course getCourse() const;

private slots:
    void selectColor();
    void validateInput();

private:
    void setupUi();
    void updateColorButton();

    Ui::CourseDialog *ui;
    QColor m_color;
};

#endif // COURSEDIALOG_H
