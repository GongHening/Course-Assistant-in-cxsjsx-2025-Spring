# 项目名称
TARGET = SmartScheduleAssistant

# 项目模板，app 表示创建一个应用程序
TEMPLATE = app

# 指定使用的 Qt 模块，这里使用了 core、gui 和 widgets 模块
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 源文件列表，列出项目中所有的源文件（.cpp 文件）
SOURCES += \
    TaskManager.cpp \
    main.cpp \
    MainWindow.cpp \
    ScheduleManager.cpp \
    Notification.cpp \
    Task.cpp \
    Course.cpp \
    CourseDialog.cpp \
    TaskDialog.cpp \
    Settings.cpp

# 头文件列表，列出项目中所有的头文件（.h 文件）
HEADERS += \
    MainWindow.h \
    ScheduleManager.h \
    Notification.h \
    Task.h \
    Course.h \
    CourseDialog.h \
    TaskDialog.h \
    Settings.h \
    TaskManager.h
FORMS += \
    MainWindow.ui\
    CourseDialog.ui\
    TaskDialog.ui
# 资源文件列表，指定项目使用的资源文件（.qrc 文件）
RESOURCES += resources.qrc

# 如果你需要链接其他库，可以在这里添加相应的配置
# LIBS += -L/path/to/library -llibraryname
