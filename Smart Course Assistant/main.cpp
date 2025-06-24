#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("YourCompany");
    a.setApplicationName("SmartScheduleAssistant");
    // 关键设置：确保程序在窗口关闭后不退出
    a.setQuitOnLastWindowClosed(false);

    QIcon appIcon(":/icons/app_icon");
    if (!appIcon.isNull()) {
        a.setWindowIcon(appIcon);
    }

    // 保留原有的翻译设置
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SmartScheduleAssistant_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // 设置应用程序图标
    a.setWindowIcon(QIcon(":/icons/app_icon"));

    // 创建并显示主窗口
    try {
        MainWindow w;
        w.show();
        return a.exec();
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "致命错误",
                              QString("发生未处理的异常:\n%1").arg(e.what()));
        return 1;
    } catch (...) {
        QMessageBox::critical(nullptr, "致命错误", "发生未知异常");
        return 1;
    }
}

