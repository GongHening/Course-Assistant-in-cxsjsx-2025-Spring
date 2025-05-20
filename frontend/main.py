import sys
import datetime
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QTableWidgetItem,
    QSystemTrayIcon, QMenu, QAction, QMessageBox
)
from PyQt5.QtGui import QIcon, QColor
from PyQt5.QtCore import QTimer, QTime

from ui.main_window_ui import Ui_MainWindow
from ui.course_dialog_ui import CourseDialog

class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.setWindowTitle("智能课程表助手")
        self.setWindowIcon(QIcon("resources/icon.png"))
        self.courses = []

        self.setup_table()
        self.setup_tray()
        self.bind_buttons()
        self.setup_timer()

    def setup_table(self):
        self.table.setRowCount(12)
        self.table.setColumnCount(7)
        self.table.setHorizontalHeaderLabels(["周一", "周二", "周三", "周四", "周五", "周六", "周日"])
        self.table.setVerticalHeaderLabels([f"第{i+1}节" for i in range(12)])
        self.table.verticalHeader().setDefaultSectionSize(50)

    def setup_tray(self):
        self.tray_icon = QSystemTrayIcon(QIcon("resources/icon.png"), self)
        menu = QMenu()
        quit_action = QAction("退出", self)
        quit_action.triggered.connect(QApplication.quit)
        menu.addAction(quit_action)
        self.tray_icon.setContextMenu(menu)
        self.tray_icon.show()

    def bind_buttons(self):
        self.btn_add_course.clicked.connect(self.add_course)
        self.btn_del_course.clicked.connect(self.del_course)

    def setup_timer(self):
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.check_current_course)
        self.timer.start(60 * 1000)  # 每分钟检查一次

    def add_course(self):
        dialog = CourseDialog(self)
        if dialog.exec_():
            course = dialog.get_course()
            self.courses.append(course)
            self.render_course(course)

    def del_course(self):
        selected = self.table.selectedItems()
        if selected:
            item = selected[0]
            self.table.setItem(item.row(), item.column(), None)

    def render_course(self, course):
        row = course['section'] - 1
        col = course['weekday'] - 1
        text = f"{course['name']}\n{course['room']}"
        item = QTableWidgetItem(text)
        item.setBackground(QColor(course.get('color', '#aee1f9')))
        self.table.setItem(row, col, item)

    def check_current_course(self):
        now = datetime.datetime.now()
        weekday = now.weekday()  # 0 = Monday
        hour = now.hour
        section = self.time_to_section(hour)

        if section is not None:
            course = next((c for c in self.courses if c['weekday'] == weekday + 1 and c['section'] == section), None)
            if course:
                self.tray_icon.showMessage("上课提醒", f"{course['name']} 现在开始\n地点: {course['room']}", QSystemTrayIcon.Information, 5000)

    def time_to_section(self, hour):
        # 简单映射，需根据实际时间表调整
        if 8 <= hour < 10: return 1
        if 10 <= hour < 12: return 2
        if 13 <= hour < 15: return 3
        if 15 <= hour < 17: return 4
        if 18 <= hour < 20: return 5
        if 20 <= hour < 22: return 6
        return None

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
