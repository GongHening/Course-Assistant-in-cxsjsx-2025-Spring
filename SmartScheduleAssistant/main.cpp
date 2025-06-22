#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

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
    MainWindow w;
    w.showNormal();

    return a.exec();
}
