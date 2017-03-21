#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QThread>

#include "version.h"
#include "const.h"
#include "mainwindow.h"
#include "config.h"
#include "parser.h"
#include "log.h"
#include "myqapp.h"


void SetupDirectories()
{
    // store application path...
    QFileInfo app_file(QApplication::applicationFilePath());
    strncpy(GlobalSettings.app_dir, app_file.absolutePath().toLatin1(), P_MAX_PATH - 1);
    Slashify(GlobalSettings.app_dir, true);

    // try to guess the environment...
    GlobalSettings.run_env = reProduction;
    char dev_dir[50];
    strncpy(dev_dir, "/src/editor/release/", 50);
    Slashify(dev_dir, true);
    if (QString(GlobalSettings.app_dir).indexOf(dev_dir) >= 0)
    {
        GlobalSettings.run_env = reRelease;
        GlobalSettings.app_dir[strlen(GlobalSettings.app_dir) - strlen(dev_dir) + 1] = 0;
    }
    strncpy(dev_dir, "/src/editor/debug/", 50);
    Slashify(dev_dir, true);
    if (GlobalSettings.run_env != reRelease && QString(GlobalSettings.app_dir).indexOf(dev_dir) >= 0)
    {
        GlobalSettings.run_env = reDebug;
        GlobalSettings.app_dir[strlen(GlobalSettings.app_dir) - strlen(dev_dir) + 1] = 0;
    }

    GlobalSettings.debug = (GlobalSettings.run_env != reProduction);

    // user data path...
    if (GlobalSettings.run_env != reProduction)
    {
        strcpy(GlobalSettings.user_dir, GlobalSettings.app_dir);
        strncat(GlobalSettings.user_dir, "home/", P_MAX_PATH);
        Slashify(GlobalSettings.user_dir, true);
    }
    else
    {
        strncpy(GlobalSettings.user_dir, QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).toLatin1(), P_MAX_PATH - 1);
        Slashify(GlobalSettings.user_dir, true);
        strncat(GlobalSettings.user_dir, FOLDER_USER, P_MAX_PATH);
        Slashify(GlobalSettings.user_dir, true);
    }

    // temp dir...
    strcpy(GlobalSettings.temp_dir, QDir::tempPath().toLatin1());
    Slashify(GlobalSettings.temp_dir, true);

    LOG2(llDebug, "Home dir: ", GlobalSettings.user_dir);
    LOG2(llDebug, "Prog dir: ", GlobalSettings.app_dir);
    LOG2(llDebug, "Temp dir: ", GlobalSettings.temp_dir);
}



int main(int argc, char *argv[])
{
    MyQApplication a(argc, argv);

    MyQApplication::setStyle("Fusion");

    // store thread id...
    GlobalSettings.app_thread_id = a.thread()->currentThreadId();

    QTranslator myappTranslator;
    myappTranslator.load("editor_pl");

    a.installTranslator(&myappTranslator);

    MainWindow main_window;
    MainWindowPtr = &main_window;

//    a.register_3Dconnexion((HWND)main_window.winId());

    SetupDirectories();

    // read default settings...
    try
    {
        SimulationSettings.reset();
        TubularSystemSettings.reset();
        VisualSettings.reset();
    }
    catch (Error *err)
    {
        LogError(err);
    }

    // show/hide debug widgets...
    main_window.set_debug(GlobalSettings.debug);

    // default window size...
    QRect rect = main_window.geometry();
    rect.setLeft(40);
    rect.setTop(40);
    rect.setWidth(VisualSettings.window_width);
    rect.setHeight(VisualSettings.window_height);
    main_window.setGeometry(rect);

    main_window.slot_set_buttons();

    // screen resolution...
    rect = QApplication::desktop()->screenGeometry();
    if (rect.width() <= VisualSettings.window_width || rect.height() <= VisualSettings.window_height)
        main_window.showMaximized();
    else
        main_window.showNormal();

    SaveNeeded(false);

    LOG(llInfo, "Application started");
    MainWindowPtr->set_window_name("");

//@@@
//    if (QApplication::arguments().size() > 1)
//        MainWindowPtr->load_scene(QApplication::arguments().at(1).toLatin1());
//    else
//        MainWindowPtr->on_pushButton_load_clicked();

    return a.exec();
}
