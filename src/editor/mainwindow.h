#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QTreeWidgetItem>
#include <QCloseEvent>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>


#include "const.h"
#include "anyeditable.h"

#define SHOW_BARRIERS 1
#define SHOW_COMPBOX  2
#define SHOW_AXIS     4
#define SHOW_TUMOR    8
#define SHOW_NORMAL  16
#define SHOW_BLOCKS  32
#define SHOW_LEGEND  64
#define SHOW_BOXES  128
#define SHOW_CLIPPING 256
#define SHOW_CLIPPING_PLANE 512
#define SHOW_TUBES 1024

#define COLOR_MODE_TISSUE_COLOR 1
#define COLOR_MODE_STATE        2
#define COLOR_MODE_PRESSURE     4
#define COLOR_MODE_O2           8
#define COLOR_MODE_TAF         16
#define COLOR_MODE_MEDICINE   32
#define COLOR_MODE_PERICYTES   64


namespace Ui {
    class MainWindow;
}

extern class MainWindow *MainWindowPtr;

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Ui::MainWindow *ui;
    QFuture<void> run_thread;
    bool simulation_running;
    bool run_repaint;

    bool x_pressed, y_pressed, z_pressed;

    QTreeWidgetItem *tree_statistics;
    QTreeWidgetItem *tree_scene;
    QTreeWidgetItem *tree_barriers;
    QTreeWidgetItem *tree_tissues;
    QTreeWidgetItem *tree_blocks;
    QTreeWidgetItem *tree_bundles;
    QTreeWidgetItem *tree_lines;
    QTreeWidgetItem *selected_item;

    QProcess *assistant_proc;

    QTimer *timer_proc_msg;


public:
    bool event(QEvent *event);

    anyEditable *selected_object;
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void msg(int line, QString const file, QString const msg);
    void repaint_graph();
    bool get_run_simulation() { return simulation_running; }
    int  get_coloring_mode();
    int  get_stereo_mode();
    bool get_show_velocities();
    unsigned long get_show_elements(unsigned long elem = 0);
    bool is_mouse_pressed();
    void display_statistics();
    void display_timers();
    void display_properties(QTreeWidgetItem* item);
    void display_properties();
    void update_selected_object_info();
    void run_simulation();
    bool get_run_repaint() { return run_repaint; }
    void set_run_repaint(bool rp) { run_repaint = rp; }
    void display_tree_objects(anyEditable *o = 0);
    bool is_object_selected(void *obj);
    bool X_checked();
    bool Y_checked();
    bool Z_checked();
    void set_save_needed(bool needed);
    void set_simulation_info(const QString msg);
    void set_enable_stereo_gl(bool enable);
    void set_debug(bool debug);
    void load_scene(char const *fname);
    void show_help_page(QString const &page);
    void compact_docks();
    void set_window_name(QString fname);
    bool get_freeze();
    void set_freeze(bool freeze, bool enabled);
    int  get_max_simul_time();

    virtual void closeEvent(QCloseEvent *e);
//    void mouse_3d_event(MSG *msg);

protected:
    void changeEvent(QEvent *e);

    QFileInfo loadedFile;

private slots:
    void on_pushButton_stats_Save_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_dockWidget_Timers_visibilityChanged(bool visible);
    void on_action_Dock_Timers_triggered();
    void on_dockWidget_Objects_visibilityChanged(bool visible);
    void on_dockWidget_File_visibilityChanged(bool visible);
    void on_dockWidget_Add_visibilityChanged(bool visible);
    void on_dockWidget_Simulation_visibilityChanged(bool visible);
    void on_dockWidget_View_visibilityChanged(bool visible);
    void on_dockWidget_mode_visibilityChanged(bool visible);
    void on_dockWidget_show_visibilityChanged(bool visible);
    void on_dockWidget_Messages_visibilityChanged(bool visible);
    void on_action_Dock_Objects_triggered();
    void on_action_Dock_Messages_triggered();
    void on_action_Dock_Show_triggered();
    void on_action_Dock_mode_triggered();
    void on_action_Dock_View_triggered();
    void on_action_Dock_Simulation_triggered();
    void on_action_Dock_Add_triggered();
    void on_action_Dock_File_triggered();
    void on_actionCompact_Docks_triggered();
    void on_actionColor_buttons_toggled(bool );
    void on_actionColor_buttons_triggered();
    void on_actionSave_as_triggered();
    void on_actionOpen_triggered();
    void on_pushButton_view_z_clicked();
    void on_pushButton_view_y_clicked();
    void on_pushButton_view_x_clicked();
    void on_pushButton_down_clicked();
    void on_pushButton_up_clicked();
    void on_actionHelp_Index_triggered();
    void on_actionExit_triggered();
    void on_menuDebug_mode_triggered();
    void on_pushButton_globalsettings_clicked();
    void on_actionNative_triggered();
    void on_actionFusion_triggered();
    void on_pushButton_add_tissue_clicked();
    void on_pushButton_add_block_clicked();
    void on_pushButton_add_tube_clicked();
    void on_pushButton_add_bundle_clicked();
    void on_pushButton_add_barrier_clicked();
    void on_tree_Objects_itemDoubleClicked(QTreeWidgetItem* item, int column);
    void on_tree_Objects_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void on_pushButton_VTK_Save_clicked();
    void on_pushButton_PovRay_Save_clicked();
    bool on_pushButton_SaveAs_clicked();
    void on_pushButton_gen_blocks_clicked();
    void on_pushButton_stop_clicked();
    void on_pushButton_quit_app_clicked();
    void on_pushButton_run_clicked();
    void on_pushButton_run_gpu_clicked();
    void on_pushButton_step_clicked();
    void on_actionMaximize_main_window_triggered();
    void on_actionRestore_windows_triggered();
    void on_pushButton_FullScreen_clicked();
    void slot_redraw_main_view(bool checked);
    void slot_redraw_main_view_on_checked(bool checked);
    void slot_process_msg();

public slots:
    void on_pushButton_load_clicked();
    void slot_gl_repaint();
    void slot_set_buttons();
    void slot_msg(QString const msg);
    void slot_repaint_graph();
};


#endif // MAINWINDOW_H
