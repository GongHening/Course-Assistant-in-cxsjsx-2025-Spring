cmake_minimum_required(VERSION 3.21)
project(CourseAssistant)

# 设置Qt6路径
set(Qt6_DIR "C:/qt/6.9.0/mingw_64/lib/cmake/Qt6") 

# 查找Qt6库
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 添加可执行文件
add_executable(CourseAssistant
    # main
    main.cpp
    
    # 前端
    frontend/MainWindow.h
    frontend/MainWindow.cpp
    frontend/AddCourseDialog.cpp
    frontend/AddCourseDialog.h
    frontend/AddHomeworkDialog.cpp
    frontend/AddHomeworkDialog.h
    
    # UI 文件
    frontend/addhomeworkdialog.ui
    frontend/course_dialog.ui
    frontend/main_window.ui
    frontend/mainwindow.ui

    #后端
    backend/Course.h
    backend/ScheduleManager.h
    backend/ScheduleManager.cpp
    backend/Course.cpp
)

# 链接Qt6库
target_link_libraries(CourseAssistant 
    Qt6::Core 
    Qt6::Widgets
    Qt6::Gui
)

# 启用自动MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
