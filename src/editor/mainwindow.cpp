//#include <QGLWidget>
#include <QtOpenGL>
#include <QFileDialog>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#include "version.h"
#include "model.h"
#include "const.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.h"
#include "log.h"
#include "parser.h"
#include "config.h"
#include "simulation.h"
#include "timers.h"
#include "statistics.h"



MainWindow *MainWindowPtr = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    simulation_running(false),
    run_repaint(false),
    x_pressed(false), y_pressed(false), z_pressed(false),
    selected_item(0),
    assistant_proc(0),
    selected_object(0)
{
    ui->setupUi(this);

    // model tube names...
    ui->pushButton_add_bundle->setText(MODEL_TUBEBUNDLE_SHORTNAME.left(1).toUpper() + MODEL_TUBEBUNDLE_SHORTNAME.mid(1));
    ui->pushButton_add_tube->setText(MODEL_TUBELINE_SHORTNAME.left(1).toUpper() + MODEL_TUBELINE_SHORTNAME.mid(1));
    ui->checkBox_show_tubes->setText(MODEL_TUBE_SHORTNAME_PL);

    // set default font size...
    QFont f(font());
    f.setPointSize(8);
    setFont(f);

    ui->pushButton_down->setVisible(false);
    ui->pushButton_up->setVisible(false);
    ui->progressBar->setVisible(false);

    DefineAllTimers();

    timer_proc_msg = new QTimer(this);
    connect(timer_proc_msg, SIGNAL(timeout()), this, SLOT(slot_process_msg()));
    timer_proc_msg->start(5);
}


void MainWindow::slot_process_msg()
{
    // dummy timer, but forcess to process 3Dconnexion messages (why?!)
}


void MainWindow::show_help_page(QString const &page)
{
    if (!assistant_proc)
        assistant_proc = new QProcess;

    if (assistant_proc->state() != QProcess::Running)
    {
        QString app = "assistant -enableRemoteControl";
        QString args = " -collectionFile \"";

        // developer environment?...
        switch (GlobalSettings.run_env)
        {
        case sat::reProduction:
            app = QString("\"") + GlobalSettings.app_dir + "\"" + app;
            args += QString(GlobalSettings.app_dir);
            break;
        case sat::reDebug:
        case sat::reRelease:
        case sat::reUnknown:
            args += QString(GlobalSettings.app_dir) + "../../manual/";
            break;
        }
        args += "motpuca.qhc\"";

        assistant_proc->start(QDir::toNativeSeparators(app + args));
        LOG(llDebug, QDir::toNativeSeparators(app + args).toLatin1());
    }

    QByteArray ba("SetSource qthelp://pl.agh.icsr.padme.motpuca.manual/manual/" + page.toLocal8Bit() + "; ");
    ba.append("expandToc 3;");
    assistant_proc->write(ba + '\0');
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void MainWindow::on_pushButton_FullScreen_clicked()
{
    if (windowState() == Qt::WindowFullScreen)
    {
        showMaximized();
        ui->pushButton_FullScreen->setText(tr("Full screen"));
    }
    else
    {
        showFullScreen();
        ui->pushButton_FullScreen->setText(tr("Restore window"));
    }
}

void MainWindow::on_actionRestore_windows_triggered()
{
    ui->dockWidget_File->setVisible(!ui->dockWidget_File->isEnabled());
    ui->dockWidget_File->setEnabled(true);

    ui->dockWidget_Add->setVisible(!ui->dockWidget_Add->isEnabled());
    ui->dockWidget_Add->setEnabled(true);

    ui->dockWidget_Simulation->setVisible(!ui->dockWidget_Simulation->isEnabled());
    ui->dockWidget_Simulation->setEnabled(true);

    ui->dockWidget_View->setVisible(!ui->dockWidget_View->isEnabled());
    ui->dockWidget_View->setEnabled(true);

    ui->dockWidget_show->setVisible(!ui->dockWidget_show->isEnabled());
    ui->dockWidget_show->setEnabled(true);

    ui->dockWidget_mode->setVisible(!ui->dockWidget_mode->isEnabled());
    ui->dockWidget_mode->setEnabled(true);

    ui->dockWidget_Objects->setVisible(!ui->dockWidget_Objects->isEnabled());
    ui->dockWidget_Objects->setEnabled(true);

    ui->dockWidget_Messages->setVisible(!ui->dockWidget_Messages->isEnabled());
    ui->dockWidget_Messages->setEnabled(true);

    ui->dockWidget_Timers->setVisible(!ui->dockWidget_Timers->isEnabled());
    ui->dockWidget_Timers->setEnabled(true);
}



void MainWindow::on_actionMaximize_main_window_triggered()
{
    ui->dockWidget_File->setEnabled(!ui->dockWidget_File->isVisible());
    ui->dockWidget_File->hide();

    ui->dockWidget_Add->setEnabled(!ui->dockWidget_Add->isVisible());
    ui->dockWidget_Add->hide();

    ui->dockWidget_Simulation->setEnabled(!ui->dockWidget_Simulation->isVisible());
    ui->dockWidget_Simulation->hide();

    ui->dockWidget_View->setEnabled(!ui->dockWidget_View->isVisible());
    ui->dockWidget_View->hide();

    ui->dockWidget_show->setEnabled(!ui->dockWidget_show->isVisible());
    ui->dockWidget_show->hide();

    ui->dockWidget_mode->setEnabled(!ui->dockWidget_mode->isVisible());
    ui->dockWidget_mode->hide();

    ui->dockWidget_Objects->setEnabled(!ui->dockWidget_Objects->isVisible());
    ui->dockWidget_Objects->hide();

    ui->dockWidget_Messages->setEnabled(!ui->dockWidget_Messages->isVisible());
    ui->dockWidget_Messages->hide();

    ui->dockWidget_Timers->setEnabled(!ui->dockWidget_Timers->isVisible());
    ui->dockWidget_Timers->hide();
}


void MainWindow::compact_docks()
{
    tabifyDockWidget(ui->dockWidget_File, ui->dockWidget_Add);
    tabifyDockWidget(ui->dockWidget_Add, ui->dockWidget_Simulation);

    tabifyDockWidget(ui->dockWidget_View, ui->dockWidget_mode);
    tabifyDockWidget(ui->dockWidget_mode, ui->dockWidget_show);
}


void MainWindow::msg(int line, QString const file, QString const msg)
{
    QString fmsg;
    if (GlobalSettings.debug)
        fmsg = file + ":" + QString::number(line) + ":  " + msg;
    else
        fmsg = msg;

    if (GlobalSettings.app_thread_id != QThread::currentThreadId())
        QMetaObject::invokeMethod(this, "slot_msg", Qt::QueuedConnection, Q_ARG(QString, fmsg));
    else
        ui->textBrowser_messages->append(fmsg);
}


void MainWindow::repaint_graph()
{
    if (GlobalSettings.app_thread_id != QThread::currentThreadId())
        QMetaObject::invokeMethod(this, "slot_repaint_graph", Qt::QueuedConnection);
    else
        ui->widget_graph->repaint();
}


void MainWindow::set_window_name(QString fname)
{
    QString s = QString(APP_NAME) +
                   ", " +
                   QString::number(MOTPUCA_VERSION) +
                   "." + QString::number(MOTPUCA_SUBVERSION) +
                   "." + QString::number(MOTPUCA_RELEASE);
    if (fname != "")
        s +=       " -- " + QFileInfo(fname).fileName();

    setWindowTitle(s);
}


void MainWindow::load_scene(const char *fname)
{
    try
    {
        DeallocSimulation();
        DeallocateCellBlocks();
        DeallocateTubeLines();
        DeallocateTubeBundles();
        DeallocateTissueSettings();
        DeallocateBarriers();
        DeallocateDefinitions();

        SimulationSettings.reset();
        TubularSystemSettings.reset();
        VisualSettings.reset();

        char basefile[P_MAX_PATH];
        snprintf(basefile, P_MAX_PATH, "%sinclude/base.ag", GlobalSettings.app_dir);
        ParseFile(basefile, false);
        ParseFile(fname, true);
        ui->checkBox_show_blocks->setChecked(!GlobalSettings.simulation_allocated);
        ui->tabWidget->setTabText(2, tr("Input text: ") + fname);
        set_window_name(QFileInfo(fname).fileName());
        ui->glwidget_main_view->repaint();
        display_tree_objects();
        display_statistics();
        slot_set_buttons();
    }
    catch (Error *err)
    {
        LogError(err);
    }
}


void MainWindow::on_pushButton_load_clicked()
/**
  Loads input file.
*/
{
    QFileDialog dialog;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_INPUT_FILES));
    urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_OUTPUT));
    dialog.setSidebarUrls(urls);
    dialog.setNameFilter(tr("Input files (*.ag);;All files (*)"));
    dialog.setDirectory(QString(GlobalSettings.user_dir) + QString(FOLDER_INPUT_FILES));
    if (!dialog.exec())
        return;

    load_scene(dialog.selectedFiles().at(0).toLatin1());
}


void MainWindow::on_pushButton_step_clicked()
{
    ResetTimer(TimerSimulationId);
    for (int i = 0; i < ui->spinBox_steps->value(); i++)
    {
        TimeStep();
        if (SimulationSettings.step % SimulationSettings.graph_sampling == 0)
            AddAllStatistics();
    }
    MainWindowPtr->repaint_graph();
    display_properties();
    slot_gl_repaint();
}


void MainWindow::slot_gl_repaint()
{
    static int last_step = 0;

    if (!ui->checkBox_freeze->isChecked())
        ui->glwidget_main_view->repaint();
    display_properties();

    QString prc;
    if (SimulationSettings.stop_time < 1000000000)
    {
        if (!ui->progressBar->isVisible()) ui->progressBar->setVisible(true);
        ui->progressBar->setMaximum(SimulationSettings.stop_time/SimulationSettings.time_step);
        ui->progressBar->setValue(SimulationSettings.step);
        prc = " (" + QString::number(int(SimulationSettings.time*100/SimulationSettings.stop_time)) + "%) ";
    }
    else
    {
        if (ui->progressBar->isVisible()) ui->progressBar->setVisible(false);
        prc = "";
    }

    set_simulation_info(QObject::tr("step: ") + QString::number(SimulationSettings.step) + prc + " | " +
                        QObject::tr("time: ") + SecToString(SimulationSettings.time) + " | " +
                        QObject::tr("steps per frame: ") + QString::number(SimulationSettings.step - last_step));
    last_step = SimulationSettings.step;
}


void MainWindow::run_simulation()
{
    QTime t;
    t.start();

    srand(QDateTime::currentMSecsSinceEpoch());

    for (int i = 0; i < 10000; i++)
        BloodFlow();


    ResetTimer(TimerSimulationId);
    while (simulation_running)
    {
        TimeStep();

        if (SimulationSettings.step % SimulationSettings.graph_sampling == 0)
        {
            AddAllStatistics();
            MainWindowPtr->repaint_graph();
        }

        if (SimulationSettings.save_statistics && SimulationSettings.step % SimulationSettings.save_statistics == 0)
        {
            char fname[P_MAX_PATH];
            snprintf(fname, P_MAX_PATH, "%sstatistics_%08d.csv", GlobalSettings.output_dir, SimulationSettings.step);
            SaveStatistics(fname);
        }

        if (SimulationSettings.save_povray && SimulationSettings.step % SimulationSettings.save_povray == 0)
            SavePovRay(0, true);

        if (SimulationSettings.save_ag && SimulationSettings.step % SimulationSettings.save_ag == 0)
        {
            char fname[P_MAX_PATH];
            snprintf(fname, P_MAX_PATH, "%sstep_%08d.ag", GlobalSettings.output_dir, SimulationSettings.step);
            SaveAG(fname, true);
        }

        if (!get_run_repaint() && t.elapsed() > 1 + 990*!(ui->checkBox_slow->isChecked()))
        {
            QMetaObject::invokeMethod(MainWindowPtr, "slot_gl_repaint", Qt::QueuedConnection);
            t.restart();
        }

        if (SimulationSettings.time >= SimulationSettings.stop_time)
        {
            simulation_running = false;

            QMetaObject::invokeMethod(MainWindowPtr, "slot_set_buttons", Qt::QueuedConnection);
            QMetaObject::invokeMethod(MainWindowPtr, "slot_gl_repaint", Qt::QueuedConnection);
        }
    }
}


int MainWindow::get_coloring_mode()
{
    int mode = 0;

    if (ui->checkBox_color_tissue->isChecked())
        mode |= COLOR_MODE_TISSUE_COLOR;
    if (ui->checkBox_color_pressure->isChecked())
        mode |= COLOR_MODE_PRESSURE;
    if (ui->checkBox_color_state->isChecked())
        mode |= COLOR_MODE_STATE;
    if (ui->checkBox_color_o2->isChecked())
        mode |= COLOR_MODE_O2;
    if (ui->checkBox_color_TAF->isChecked())
        mode |= COLOR_MODE_TAF;
    if (ui->checkBox_color_Pericytes->isChecked())
        mode |= COLOR_MODE_PERICYTES;

    return mode;
}


bool MainWindow::get_show_velocities()
{
    return ui->checkBox_velocity->isChecked();
}


unsigned long MainWindow::get_show_elements(unsigned long elem)
{
    unsigned long ret =  SHOW_BARRIERS*ui->checkBox_show_barriers->isChecked()
                          + SHOW_COMPBOX*ui->checkBox_show_compbox->isChecked()
                          + SHOW_AXIS*ui->checkBox_show_axes->isChecked()
                          + SHOW_TUMOR*ui->checkBox_show_tumor->isChecked()
                          + SHOW_NORMAL*ui->checkBox_show_normal->isChecked()
                          + SHOW_BLOCKS*ui->checkBox_show_blocks->isChecked()
                          + SHOW_LEGEND*ui->checkBox_show_legend->isChecked()
                          + SHOW_BOXES*ui->checkBox_show_boxes->isChecked()
                          + SHOW_CLIPPING*ui->checkBox_clip->isChecked()
                          + SHOW_CLIPPING_PLANE*ui->checkBox_show_clip->isChecked()
                          + SHOW_TUBES*ui->checkBox_show_tubes->isChecked();

    if (!elem)
        return ret;
    else
        return ret & elem;
}



bool MainWindow::is_mouse_pressed()
{
    return ui->glwidget_main_view->is_mouse_pressed();
}


void MainWindow::on_pushButton_run_clicked()
{
    simulation_running = true;
    slot_set_buttons();
    run_thread = QtConcurrent::run(MainWindowPtr, &MainWindow::run_simulation);
}


void MainWindow::on_pushButton_quit_app_clicked()
{
    simulation_running = false;
    run_thread.waitForFinished();
    slot_set_buttons();

    close();
}

void MainWindow::on_pushButton_stop_clicked()
{
    simulation_running = false;
    run_thread.waitForFinished();

    slot_set_buttons();

    ui->glwidget_main_view->repaint();
}


void MainWindow::slot_redraw_main_view_on_checked(bool checked)
{
    if (checked)
        ui->glwidget_main_view->repaint();
}


void MainWindow::slot_redraw_main_view(bool /*checked*/)
{
    ui->glwidget_main_view->repaint();
}


void MainWindow::on_pushButton_gen_blocks_clicked()
{
    try
    {
        if (!GlobalSettings.simulation_allocated)
            AllocSimulation();

        GenerateTubesInAllTubeBundles();
        GenerateTubesInAllTubeLines();
        GenerateCellsInAllBlocks();
    }
    catch (Error *err)
    {
        LogError(err);
    }

    ui->checkBox_show_blocks->setChecked(false);

    slot_set_buttons();

    display_properties();
    ui->glwidget_main_view->repaint();
}



void MainWindow::display_properties()
{
    if (ui->tree_Objects->selectedItems().isEmpty())
        display_properties(tree_statistics);
    else
        display_properties(ui->tree_Objects->selectedItems().first());
}



void MainWindow::display_properties(QTreeWidgetItem* item)
{
    ui->pushButton_down->setVisible(false);
    ui->pushButton_up->setVisible(false);
    ui->pushButton_down->setEnabled(false);
    ui->pushButton_up->setEnabled(false);

    anyEditable *itemptr;
    display_timers();
    if (item == tree_statistics)
        display_statistics();
    else if (item == tree_scene)
    {
        ui->textBrowser_stats->clear();
        ui->textBrowser_stats->append(tr("SCENE"));
        ui->textBrowser_stats->append("");
        ui->textBrowser_stats->append(tr("width: ") + "<b>" + QString::number(SimulationSettings.comp_box_to.x - SimulationSettings.comp_box_from.x) + "</b>");
        ui->textBrowser_stats->append(tr("height: ") + "<b>" + QString::number(SimulationSettings.comp_box_to.y - SimulationSettings.comp_box_from.z) + "</b>");
        ui->textBrowser_stats->append(tr("depth: ") + "<b>" + QString::number(SimulationSettings.comp_box_to.z - SimulationSettings.comp_box_from.z) + "</b>");
    }
    else if (item)
    {
        itemptr = (anyEditable *)item->data(0, Qt::UserRole).toInt();
        if (itemptr)
        {
            itemptr->display_properties(ui->textBrowser_stats);
            ui->pushButton_down->setEnabled(itemptr->can_move_down());
            ui->pushButton_up->setEnabled(itemptr->can_move_up());
            ui->pushButton_down->setVisible(ui->pushButton_up->isEnabled() || ui->pushButton_down->isEnabled());
            ui->pushButton_up->setVisible(ui->pushButton_up->isEnabled() || ui->pushButton_down->isEnabled());
        }
        else
            ui->textBrowser_stats->clear();
    }
}

void MainWindow::display_statistics()
{
    ui->textBrowser_stats->clear();
    ui->textBrowser_stats->append(tr("STATISTICS"));
    ui->textBrowser_stats->append("");

    anyTissueSettings *ts = FirstTissueSettings;

    // global data...
    ui->textBrowser_stats->append(tr("Time: ") + SecToString(SimulationSettings.time));
    ui->textBrowser_stats->append(tr("Step: ") + QString::number(SimulationSettings.step));

    // loop over all tissues...
    int no_cells = 0;
    real pressure_sum = 0;
    while (ts)
    {
        QString t_color;

        t_color = QString(ts->color.to_HTML());

        ui->textBrowser_stats->append("");
        ui->textBrowser_stats->append(QString("<b>") + ts->name + QString("</b> (") + sat::TissueType{ts->type} + ") " +
                                      "<span style='color:" + t_color + "'>&bull;</span>");
        ui->textBrowser_stats->append(tr("  cells: ") + QString::number(ts->no_cells[0]));
        ui->textBrowser_stats->append(tr("  pressure: ") + QString::number(ts->pressure, 'g', 3) + " (" + QString::number(int(ts->pressure*100/ts->max_pressure)) + "%)");

        no_cells += ts->no_cells[0];
        pressure_sum += ts->pressure_sum;

        ts = ts->next;
    }

    // tubes...
    ui->textBrowser_stats->append("");
    ui->textBrowser_stats->append(QString("<b>") + MODEL_TUBECHAIN_SHORTNAME_PL + QString("</b>") + ": " + QString::number(NoTubeChains));
    ui->textBrowser_stats->append(QString("<b>") + MODEL_TUBE_SHORTNAME_PL + QString("</b>") + ": " + QString::number(NoTubes));

    // global...
    ui->textBrowser_stats->append("");
    ui->textBrowser_stats->append(QString("<b>") + tr("totals") + QString("</b>"));
    ui->textBrowser_stats->append(tr("  cells: ") + QString::number(no_cells) + " + " + QString::number(NoTubes));
    if (no_cells > 0)
        ui->textBrowser_stats->append(tr("  pressure: ") + QString::number(pressure_sum/no_cells, 'g', 3));
    ui->textBrowser_stats->append(tr("  max cells in box: ") + QString::number(SimulationSettings.max_max_cells_per_box) +
                                  " (" + QString::number(SimulationSettings.max_max_max_cells_per_box) + ")");
}


void MainWindow::display_timers()
{
    // timers...
    ui->textBrowser_timers->clear();
    ui->textBrowser_timers->append(ReportTimer(TimerSimulationId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerTubeUpdateId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerResetForcesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerCellCellForcesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerCellBarrierForcesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerTubeTubeForcesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerTubeCellForcesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerCellGrowId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerTubeGrowId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerRearangeId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerRemoveTubesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerConnectTubeChainsId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerMergeTubesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerUpdatePressuresId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerCopyConcentrationsId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerTissuePropertiesId, true));
    ui->textBrowser_timers->append(ReportTimer(TimerBloodFlowId, true));
}


bool MainWindow::on_pushButton_SaveAs_clicked()
{
    try
    {
        QFileDialog dialog;
        QList<QUrl> urls;
        urls << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_INPUT_FILES));
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setSidebarUrls(urls);
        dialog.setNameFilter(tr("Input files (*.ag);;All files (*)"));
        dialog.setDefaultSuffix("ag");
        dialog.setDirectory(QString(GlobalSettings.user_dir) + QString(FOLDER_INPUT_FILES));
        if (!dialog.exec())
            return false;

        SaveAG(dialog.selectedFiles().at(0).toLatin1(), true);
    }
    catch (Error *err)
    {
        LogError(err);
        return false;
    }

    SaveNeeded(false);
    return true;
}


void MainWindow::on_pushButton_PovRay_Save_clicked()
{
    try
    {
        QFileDialog dialog;
        QList<QUrl> urls;
        urls << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_INPUT_FILES));
        urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_OUTPUT));
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setSidebarUrls(urls);
        dialog.setNameFilter(tr("Povray files (*.pov);;All files (*)"));
        dialog.setDefaultSuffix("pov");
        if (!dialog.exec())
            return;

        SavePovRay(dialog.selectedFiles().at(0).toLatin1(), false);
    }
    catch (Error *err)
    {
        LogError(err);
        return;
    }
}

void MainWindow::on_pushButton_VTK_Save_clicked()
{
    SaveVTK();
}


void MainWindow::display_tree_objects(anyEditable *o)
{
    ui->pushButton_down->setVisible(false);
    ui->pushButton_up->setVisible(false);

//    QFont font_bold("Arial", 10, 1, false);
    QFont font_bold(ui->dockWidget_Objects->font());
    font_bold.setBold(true);
    ui->tree_Objects->clear();
    ui->tree_Objects->setUniformRowHeights(true);
    tree_statistics = new QTreeWidgetItem(ui->tree_Objects);
    tree_statistics->setText(0, tr("Statistics"));
    tree_statistics->setFont(0, font_bold);
    tree_scene = new QTreeWidgetItem(ui->tree_Objects);
    tree_scene->setText(0, tr("Scene"));
    tree_scene->setFont(0, font_bold);
    tree_barriers = new QTreeWidgetItem(ui->tree_Objects);
    tree_barriers->setText(0, tr("Barriers"));
    tree_tissues = new QTreeWidgetItem(ui->tree_Objects);
    tree_tissues->setText(0, tr("Tissues"));
    tree_blocks = new QTreeWidgetItem(ui->tree_Objects);
    tree_blocks->setText(0, tr("Cell Blocks"));
    tree_bundles = new QTreeWidgetItem(ui->tree_Objects);
    tree_bundles->setText(0, MODEL_TUBEBUNDLE_NAME_PL.left(1).toUpper() + MODEL_TUBEBUNDLE_NAME_PL.mid(1));
    tree_lines = new QTreeWidgetItem(ui->tree_Objects);
    tree_lines->setText(0, MODEL_TUBELINE_NAME_PL.left(1).toUpper() + MODEL_TUBELINE_NAME_PL.mid(1));

    tree_statistics->setSizeHint(0, QSize(40, QFontMetrics(font_bold).height()));


    // barriers...
    anyBarrier *b = FirstBarrier;
    int cnt = 0;
    int cnt_all = 0;
    while (b)
    {
        cnt++;
        cnt_all++;
        QTreeWidgetItem *i = new QTreeWidgetItem(tree_barriers);
        i->setData(0, Qt::UserRole, (int)(size_t)(b));
        i->setText(0, b->get_name());
        i->setFont(0, font_bold);

        b = (anyBarrier *)b->next;
    }
    tree_barriers->setHidden(!cnt);

    // tissues...
    anyTissueSettings *ts = FirstTissueSettings;
    cnt = 0;
    while (ts)
    {
        cnt++;
        cnt_all++;
        QTreeWidgetItem *i = new QTreeWidgetItem(tree_tissues);
        i->setData(0, Qt::UserRole, (int)(size_t)(ts));
        i->setText(0, ts->get_name());
        i->setFont(0, font_bold);

        ts = ts->next;
    }
    tree_tissues->setHidden(!cnt);

    // cell blocks...
    anyCellBlock *cb = FirstCellBlock;
    cnt = 0;
    while (cb)
    {
        cnt++;
        cnt_all++;
        QTreeWidgetItem *i = new QTreeWidgetItem(tree_blocks);
        i->setData(0, Qt::UserRole, (int)(size_t)(cb));
        i->setText(0, cb->get_name());
        i->setFont(0, font_bold);

        if (o == cb)
            ui->tree_Objects->setCurrentItem(i);

        cb = (anyCellBlock *)cb->next;
    }
    tree_blocks->setHidden(!cnt);

    // tube bundles...
    anyTubeBundle *vb = FirstTubeBundle;
    cnt = 0;
    while (vb)
    {
        cnt++;
        cnt_all++;
        QTreeWidgetItem *i = new QTreeWidgetItem(tree_bundles);
        i->setData(0, Qt::UserRole, (int)(size_t)(vb));
        i->setText(0, vb->get_name());
        i->setFont(0, font_bold);

        vb = (anyTubeBundle *)vb->next;
    }
    tree_bundles->setHidden(!cnt);

    // tube lines...
    anyTubeLine *vl = FirstTubeLine;
    cnt = 0;
    while (vl)
    {
        cnt++;
        cnt_all++;
        QTreeWidgetItem *i = new QTreeWidgetItem(tree_lines);
        i->setData(0, Qt::UserRole, (int)(size_t)(vl));
        i->setText(0, vl->get_name());
        i->setFont(0, font_bold);

        vl = (anyTubeLine *)vl->next;
    }
    tree_lines->setHidden(!cnt);

    cnt_all += 2   // statistics & scene
            + int(!tree_barriers->isHidden())
            + int(!tree_tissues->isHidden())
            + int(!tree_blocks->isHidden())
            + int(!tree_bundles->isHidden())
            + int(!tree_lines->isHidden());

    ui->tree_Objects->expandAll();
    ui->tree_Objects->setMaximumHeight(cnt_all * (tree_statistics->sizeHint(0).height() + 1));
    ui->tree_Objects->setMinimumHeight(cnt_all * (tree_statistics->sizeHint(0).height() + 1));
}

void MainWindow::on_tree_Objects_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current)
    {
        display_properties(current);
        selected_object = (anyEditable *)(current->data(0, Qt::UserRole).toInt());
        selected_item = current;
        ui->glwidget_main_view->repaint();
    }
    else
    {
        selected_object = 0;
        selected_item = 0;
    }
}


bool MainWindow::is_object_selected(void *obj)
{
    return obj == selected_object;
}


bool MainWindow::X_checked()
{
    return x_pressed || (!x_pressed && !y_pressed && !z_pressed);
}


bool MainWindow::Y_checked()
{
    return y_pressed || (!x_pressed && !y_pressed && !z_pressed);
}


bool MainWindow::Z_checked()
{
    return z_pressed || (!x_pressed && !y_pressed && !z_pressed);
}

void MainWindow::on_tree_Objects_itemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
    if (selected_object)
        selected_object->display_dialog();
    else if (item->text(0) == QObject::tr("Scene"))
        displayGlobalsDialog();
}


void MainWindow::update_selected_object_info()
{
    if (selected_object && selected_item)
    {
        selected_item->setText(0, selected_object->get_name());
        display_properties(selected_item);
    }
}

void MainWindow::on_pushButton_add_barrier_clicked()
{
    anyBarrier *b = new anyBarrier;
    b->read_defaults();

    b->display_dialog(true);
}

void MainWindow::on_pushButton_add_bundle_clicked()
{
    anyTubeBundle *vb = new anyTubeBundle;
    vb->read_defaults();

    vb->display_dialog(true);
}

void MainWindow::on_pushButton_add_tube_clicked()
{
    anyTubeLine *vl = new anyTubeLine;
    vl->read_defaults();

    vl->display_dialog(true);
}

void MainWindow::on_pushButton_add_block_clicked()
{
    anyCellBlock *cb = new anyCellBlock;
    cb->read_defaults();

    cb->display_dialog(true);
}

void MainWindow::on_pushButton_add_tissue_clicked()
{
    anyTissueSettings *ts = new anyTissueSettings;
    ts->read_defaults();

    ts->display_dialog(true);
}


int MainWindow::get_stereo_mode()
{
    if (ui->radioButton_mono->isChecked())
        return 0;
    else if (ui->radioButton_stereo1->isChecked())
        return 1;
    else
        return 2;
}


void MainWindow::on_actionFusion_triggered()
{
    QApplication::setStyle("Fusion");
}


void MainWindow::on_actionNative_triggered()
{
    QApplication::setStyle("Windows");
}


void MainWindow::on_pushButton_globalsettings_clicked()
{
    displayGlobalsDialog();
}


void MainWindow::set_save_needed(bool needed)
{
    ui->label_saveneeded->setText(needed ? "#": "");
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    bool accept = false;

    if (!GlobalSettings.save_needed)
        accept = true;
    else
        switch (QMessageBox::question(0, QObject::tr("Confirmation"),
                                      QObject::tr("Save configuration before closing?"),
                                      QObject::tr("Save"), QObject::tr("Don't save"), QObject::tr("Cancel")))
        {
        case 0: if (on_pushButton_SaveAs_clicked())
                    accept = true;
                break;
        case 1: accept = true; break;
        case 2: accept = false; break;
        }

    if (accept)
    {
        on_pushButton_quit_app_clicked();
        e->accept();
    }
    else
        e->ignore();
}


void MainWindow::set_simulation_info(const QString msg)
{
    ui->label_info->setText(msg);
}


void MainWindow::set_enable_stereo_gl(bool enable)
{
    ui->radioButton_stereo2->setEnabled(enable);
}


void MainWindow::set_debug(bool debug)
{
    ui->menuDebug_mode->setChecked(debug);
    ui->dockWidget_Messages->setVisible(debug && ui->action_Dock_Messages->isChecked());
    ui->dockWidget_Timers->setVisible(debug && ui->action_Dock_Timers->isChecked());
    ui->action_Dock_Messages->setVisible(debug);
    ui->action_Dock_Timers->setVisible(debug);
    ui->pushButton_VTK_Save->setVisible(debug);
    ui->tabWidget->setTabEnabled(2, debug);
    ui->checkBox_show_boxes->setVisible(debug);
}


void MainWindow::on_menuDebug_mode_triggered()
{
    GlobalSettings.debug = ui->menuDebug_mode->isChecked();
    set_debug(ui->menuDebug_mode->isChecked());
}


void MainWindow::on_actionExit_triggered()
{
  on_pushButton_quit_app_clicked();
}


void MainWindow::slot_set_buttons()
{
    // running simulation...
    int no_cells = 0;
    anyTissueSettings *ts = FirstTissueSettings;
    while (ts)
    {
        no_cells += ts->no_cells[0];
        ts = ts->next;
    }

    ui->pushButton_run->setEnabled(no_cells + NoTubes > 0 && !simulation_running);
    ui->pushButton_run_gpu->setEnabled(no_cells + NoTubes > 0 && !simulation_running);
    ui->pushButton_step->setEnabled(no_cells + NoTubes > 0 && !simulation_running);
    ui->pushButton_stop->setEnabled(simulation_running);

    ui->pushButton_gen_blocks->setEnabled(!simulation_running);
    ui->pushButton_load->setEnabled(!simulation_running);
    ui->actionOpen->setEnabled(!simulation_running);
}


bool MainWindow::event(QEvent *event)
{
    if (MainWindowPtr)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_X)
                x_pressed = true;
            else if (ke->key() == Qt::Key_Y)
                y_pressed = true;
            else if (ke->key() == Qt::Key_Z)
                z_pressed = true;

            if (X_checked() && Y_checked() && Z_checked())
                ui->label_keys->setText("");
            else
                ui->label_keys->setText(QString(X_checked() ? "X" : "") + QString(Y_checked() ? "Y" : "") + QString(Z_checked() ? "Z" : ""));
        }
        else if (event->type() == QEvent::KeyRelease)
        {
            QKeyEvent *ke = static_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_X)
                x_pressed = false;
            else if (ke->key() == Qt::Key_Y)
                y_pressed = false;
            else if (ke->key() == Qt::Key_Z)
                z_pressed = false;

            if (X_checked() && Y_checked() && Z_checked())
                ui->label_keys->setText("");
            else
                ui->label_keys->setText(QString(X_checked() ? "X" : "") + QString(Y_checked() ? "Y" : "") + QString(Z_checked() ? "Z" : ""));
        }
    }
    return QMainWindow::event(event);
}



void MainWindow::on_actionHelp_Index_triggered()
{
    show_help_page("index.html");
}


void MainWindow::slot_msg(QString const msg)
{
    ui->textBrowser_messages->append(msg);
}


void MainWindow::slot_repaint_graph()
{
    ui->widget_graph->repaint();
}


void MainWindow::on_pushButton_up_clicked()
{
    if (selected_object && selected_object->can_move_up())
    {
        selected_object->move_up();
        MainWindowPtr->display_tree_objects(selected_object);
        SaveNeeded(true);
    }
}



void MainWindow::on_pushButton_down_clicked()
{
    if (selected_object && selected_object->can_move_down())
    {
        selected_object->move_down();
        MainWindowPtr->display_tree_objects(selected_object);
        SaveNeeded(true);
    }
}


void MainWindow::on_pushButton_view_x_clicked()
{
    VisualSettings.v_matrix.setToRotationY(-90);
    VisualSettings.r_matrix.setToRotationY(-90);
    ui->glwidget_main_view->repaint();
}

void MainWindow::on_pushButton_view_y_clicked()
{
    VisualSettings.v_matrix.setToRotationX(90);
    VisualSettings.r_matrix.setToRotationX(90);
    ui->glwidget_main_view->repaint();
}

void MainWindow::on_pushButton_view_z_clicked()
{
    VisualSettings.v_matrix.setToIdentity();
    VisualSettings.r_matrix.setToIdentity();
    ui->glwidget_main_view->repaint();
}

void MainWindow::on_actionOpen_triggered()
{
    on_pushButton_load_clicked();
}

void MainWindow::on_actionSave_as_triggered()
{
    on_pushButton_SaveAs_clicked();
}

void MainWindow::on_actionColor_buttons_triggered()
{
}

void MainWindow::on_actionColor_buttons_toggled(bool c)
{
    if (!c)
    {
        ui->pushButton_add_tissue->setStyleSheet("");
        ui->pushButton_add_block->setStyleSheet("");
        ui->pushButton_add_tube->setStyleSheet("");
        ui->pushButton_add_bundle->setStyleSheet("");
        ui->pushButton_add_barrier->setStyleSheet("");
        ui->pushButton_globalsettings->setStyleSheet("");
    }
    else
    {
        ui->pushButton_add_tissue->setStyleSheet("background: #96b7cf");
        ui->pushButton_add_block->setStyleSheet("background: #96b7cf");
        ui->pushButton_add_tube->setStyleSheet("background: #cf96a8");
        ui->pushButton_add_bundle->setStyleSheet("background: #cf96a8");
        ui->pushButton_add_barrier->setStyleSheet("background: #cfc896");
        ui->pushButton_globalsettings->setStyleSheet("background: #505050; color: #ffffff");
    }
}


void MainWindow::on_actionCompact_Docks_triggered()
{
    compact_docks();
}


void MainWindow::on_action_Dock_File_triggered()
{
    ui->dockWidget_File->setVisible(ui->action_Dock_File->isChecked());
}


void MainWindow::on_action_Dock_Add_triggered()
{
    ui->dockWidget_Add->setVisible(ui->action_Dock_Add->isChecked());
}


void MainWindow::on_action_Dock_Simulation_triggered()
{
    ui->dockWidget_Simulation->setVisible(ui->action_Dock_Simulation->isChecked());
}


void MainWindow::on_action_Dock_View_triggered()
{
    ui->dockWidget_View->setVisible(ui->action_Dock_View->isChecked());
}


void MainWindow::on_action_Dock_mode_triggered()
{
    ui->dockWidget_mode->setVisible(ui->action_Dock_mode->isChecked());
}


void MainWindow::on_action_Dock_Show_triggered()
{
    ui->dockWidget_show->setVisible(ui->action_Dock_Show->isChecked());
}


void MainWindow::on_action_Dock_Messages_triggered()
{
    ui->dockWidget_Messages->setVisible(ui->action_Dock_Messages->isChecked());
}


void MainWindow::on_action_Dock_Timers_triggered()
{
    ui->dockWidget_Timers->setVisible(ui->action_Dock_Timers->isChecked());
}


void MainWindow::on_action_Dock_Objects_triggered()
{
    ui->dockWidget_Objects->setVisible(ui->action_Dock_Objects->isChecked());
}

void MainWindow::on_dockWidget_Messages_visibilityChanged(bool visible)
{
    ui->action_Dock_Messages->setChecked(visible);
}

void MainWindow::on_dockWidget_Timers_visibilityChanged(bool visible)
{
    ui->action_Dock_Timers->setChecked(visible);
}

void MainWindow::on_dockWidget_show_visibilityChanged(bool visible)
{
    ui->action_Dock_Show->setChecked(visible);
}

void MainWindow::on_dockWidget_mode_visibilityChanged(bool visible)
{
    ui->action_Dock_mode->setChecked(visible);
}

void MainWindow::on_dockWidget_View_visibilityChanged(bool visible)
{
    ui->action_Dock_View->setChecked(visible);
}

void MainWindow::on_dockWidget_Simulation_visibilityChanged(bool visible)
{
    ui->action_Dock_Simulation->setChecked(visible);
}

void MainWindow::on_dockWidget_Add_visibilityChanged(bool visible)
{
    ui->action_Dock_Add->setChecked(visible);
}

void MainWindow::on_dockWidget_File_visibilityChanged(bool visible)
{
    ui->action_Dock_File->setChecked(visible);
}

void MainWindow::on_dockWidget_Objects_visibilityChanged(bool visible)
{
    ui->action_Dock_Objects->setChecked(visible);
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 2) // Input text tab
    {
        char fname[P_MAX_PATH];
        snprintf(fname, P_MAX_PATH, "%sscene.tmp", GlobalSettings.temp_dir);

        SaveAG(fname, true);

        ui->textEdit_src->clear();
//        QFile f2(fname);
//        f2.open(QIODevice::ReadOnly | QIODevice::Text);
//        ui->textEdit_src->setText(f2.readAll());
//        f2.close();
//        unlink(fname);
    }
}

void MainWindow::on_pushButton_stats_Save_clicked()
{
    if (!Statistics)
    {
        QMessageBox::information(this, tr("Information"), tr("No statistical data collected yet."));
        return;
    }

    try
    {
        QFileDialog dialog;
        QList<QUrl> urls;
        urls << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_INPUT_FILES));
        urls << QUrl::fromLocalFile(QString(GlobalSettings.user_dir) + QString(FOLDER_OUTPUT));
        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setSidebarUrls(urls);
        dialog.setNameFilter(tr("CSV files (*.csv);;All files (*)"));
        dialog.setDefaultSuffix("csv");
        if (!dialog.exec())
            return;

        SaveStatistics(dialog.selectedFiles().at(0).toLatin1());
    }
    catch (Error *err)
    {
        LogError(err);
        return;
    }
}


bool MainWindow::get_freeze()
{
    return ui->checkBox_freeze->isChecked();
}


void MainWindow::set_freeze(bool freeze, bool enabled)
{
    ui->checkBox_freeze->setChecked(freeze);
    ui->checkBox_freeze->setEnabled(enabled);
}


void MainWindow::on_pushButton_run_gpu_clicked()
{
    bool freeze_prev = get_freeze();
    set_freeze(true, false);
    QMessageBox::information(this, "VIM", "Not Implemented Yet...");
    set_freeze(freeze_prev, true);
}


int MainWindow::get_max_simul_time()
{
    return ui->spinBox_max_sim_time->value();
}


//void MainWindow::mouse_3d_event(MSG */*msg*/)
//{
//    //@@@
//}
